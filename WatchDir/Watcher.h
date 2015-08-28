//////////////////////////////////////////////////////////////////////

#pragma once
#include "Shlwapi.h"
#pragma comment(lib, "shlwapi.lib")

//////////////////////////////////////////////////////////////////////

struct Watcher
{
	//////////////////////////////////////////////////////////////////////
	// Add things to the Queue and push the timer back

	struct MyTimer : Timer
	{
		Watcher *mWatcher;

		MyTimer(Watcher *w)
			: Timer()
			, mWatcher(w)
		{
		}

		void OnTimer() override
		{
			mWatcher->OnTimer();
		}
	};

	//////////////////////////////////////////////////////////////////////

	HANDLE					mDirHandle;
	HANDLE					mHandle;
	vector<Command>			mCommands;
	tstring					mFolder;
	bool					mRecurse;
	DWORD					mFlags;
	ptr<byte>				mBuffer;
	size_t					mBufferSize;
	OVERLAPPED				mOverlapped;
	safe_queue<FileEvent *>	mQueue;
	std::thread				mThread;
	HANDLE					mThreadHandle;
	MyTimer					mTimer;
	std::mutex				mMutex;
	float64					mSettleDelay;
	vector<tstring>			mIncludes;
	vector<tstring>			mExcludes;

	using lock = std::lock_guard<std::mutex>;

	//////////////////////////////////////////////////////////////////////

	struct Signal
	{
		Watcher *mWatcher;
		list<FileEvent *> mQueue;

		//////////////////////////////////////////////////////////////////////

		Signal(Watcher *watcher)
			: mWatcher(watcher)
		{
		}

		//////////////////////////////////////////////////////////////////////

		~Signal()
		{
		}

		//////////////////////////////////////////////////////////////////////

		void Execute()
		{
			// for coalescing the activity on each object (eg it was (modified+renamed))
			std::map<tstring, DWORD> folderActivity;

			// join renames into single events and build activity mask for each event
			FileEvent *old = null;
			for (auto p : mQueue)
			{
				if (p->mAction == FILE_ACTION_RENAMED_OLD_NAME)
				{
					old = p;
				}
				else
				{
					if (p->mAction == FILE_ACTION_RENAMED_NEW_NAME && old != null)
					{
						p->SetFilePath(mWatcher->mFolder, old->mFilePath);
						p->SetNewFilePath(mWatcher->mFolder, p->mFilePath);
						old = null;
					}
				}
			}

			mQueue.remove_if([](FileEvent *f) {
				return f->mAction == FILE_ACTION_RENAMED_OLD_NAME;
			});

			// remove what should not be included
			mQueue.remove_if([this](FileEvent *f) {
				if (mWatcher->mIncludes.empty())
				{
					return false;
				}
				for (auto const &wildCard : mWatcher->mIncludes)
				{
					if (PathMatchSpec(f->mFilePath.c_str(), wildCard.c_str()))
					{
						return false;
					}
				}
				return true;
			});

			// remove what should be excluded
			mQueue.remove_if([this](FileEvent *f) {
				if (mWatcher->mExcludes.empty())
				{
					return false;
				}
				for (auto const &wildCard : mWatcher->mExcludes)
				{
					if (PathMatchSpec(f->mFilePath.c_str(), wildCard.c_str()))
					{
						return true;
					}
				}
				return false;
			});

			// sort the filepaths based on their depth (deepest first)
			mQueue.sort([](FileEvent *a, FileEvent *b) {
				return GetPathDepth(b->mFilePath) < GetPathDepth(a->mFilePath);
			});

			// build activity masks
			for (auto p : mQueue)
			{
				folderActivity[p->mFilePath] |= 1 << (p->mAction - 1);
			}

			// call commands once for each unique filepath
			tprintf($("======================================================================\n"));
			for (auto p : mQueue)
			{
				DWORD activity = folderActivity[p->mFilePath];
				if (activity != 0)
				{
					tprintf($("\t%s (%04x)\n"), p->Details().c_str(), activity);
					for (auto &cmd : mWatcher->mCommands)
					{
						if ((cmd.mFilter & activity) != 0)
						{
							cmd.Execute(mWatcher->mFolder, p);
						}
					}
					folderActivity[p->mFilePath] = 0;
				}
			}
		}
	};

	//////////////////////////////////////////////////////////////////////

	void OnTimer()
	{
		Signal *signal = new Signal(this);
		mQueue.move_to(signal->mQueue);
		QueueUserAPC(ExecuteList, mThreadHandle, (ULONG_PTR)signal);
	}

	//////////////////////////////////////////////////////////////////////

	static void CALLBACK ExecuteList(ULONG_PTR param)
	{
		Signal *signal = (Signal *)param;
		signal->Execute();
		delete signal;
	}

	//////////////////////////////////////////////////////////////////////

	static DWORD GetFlags(string const &triggers)
	{
		vector<string> tokens;
		tokenize(triggers.c_str(), tokens, " ;,", false);
		DWORD flags = 0;
		for(auto const &t : tokens)
		{
			auto c = std::find(condition_defs.begin(), condition_defs.end(), t);
			if(c != condition_defs.end())
			{
				flags |= c->flag;
			}
			else
			{
				error("Unknown condition %s\n", t.c_str());
				throw err_bad_input;
			}
		}
		return flags;
	}

	//////////////////////////////////////////////////////////////////////

	Watcher(xml_node<> *watch, int buffer_size = 16384)
		: mHandle(INVALID_HANDLE_VALUE)
		, mDirHandle(INVALID_HANDLE_VALUE)
		, mBufferSize(buffer_size)
		, mBuffer(new byte[buffer_size])
		, mThread(&Watcher::WaitForEvents, this)
		, mRecurse(true)
		, mTimer(this)
		, mSettleDelay(0.25)
	{
		xml_node<> *pathNode = watch->first_node("path");
		xml_node<> *triggersNode = watch->first_node("triggers");
		xml_node<> *commandNode = watch->first_node("command");
		xml_node<> *includeNode = watch->first_node("include");
		xml_node<> *excludeNode = watch->first_node("exclude");

		if (pathNode == null || triggersNode == null || commandNode == null)
		{
			throw err_bad_input;
		}

		if (includeNode != null)
		{
			tokenize(TString(includeNode->val()).c_str(), mIncludes, $(";"), false);
		}

		if (excludeNode != null)
		{
			tokenize(TString(excludeNode->val()).c_str(), mExcludes, $(";"), false);
		}

		mFolder = ExpandEnvironment(TString(pathNode->val()));
		mFlags = GetFlags(triggersNode->val());

		xmlGetBoolNode(watch, mRecurse, Optional, $("recursive"));
		xmlGetDoubleNode(watch, mSettleDelay, Optional, $("settleDelay"));

		while(commandNode != null)
		{
			mCommands.push_back(Command(commandNode));
			commandNode = commandNode->next_sibling();
		}
		mThreadHandle = mThread.native_handle();
	}

	//////////////////////////////////////////////////////////////////////

	void WaitForEvents()
	{
		while(true)
		{
			SleepEx(INFINITE, TRUE);
		}
	}

	//////////////////////////////////////////////////////////////////////

	static void CALLBACK ChangeOccurred(DWORD error, DWORD numBytes, LPOVERLAPPED overlappedPtr)
	{
		((Watcher *)(overlappedPtr->hEvent))->OnChange(error, numBytes, overlappedPtr);
	}

	//////////////////////////////////////////////////////////////////////

	void OnChange(DWORD errorCode, DWORD numBytes, LPOVERLAPPED overlappedPtr)
	{
		if (errorCode != ERROR_SUCCESS)
		{
			error($("Error %d stopping watching %s\n"), errorCode, mFolder.c_str());
		}
		else
		{
			FILE_NOTIFY_INFORMATION *f = (FILE_NOTIFY_INFORMATION *)(mBuffer.get());
			DWORD offset;
			int n = 0;
			do
			{
				++n;
				offset = f->NextEntryOffset;
				size_t len = (size_t)(f->FileNameLength / sizeof(wstring::value_type));
				tstring filename = TString(wstring((wchar *)f->FileName, len));
				mQueue.add(new FileEvent(f->Action, mFolder, filename));
				mTimer.SetDelay(1);
				f = (FILE_NOTIFY_INFORMATION *)((byte *)f + offset);
			} while (offset != 0);
			Read();
		}
	}

	//////////////////////////////////////////////////////////////////////

	BOOL Read()
	{
		DWORD bytesGot = 0;
		mOverlapped = { 0 };
		mOverlapped.hEvent = (HANDLE)this;
		if (!ReadDirectoryChangesW(mDirHandle,
									(LPVOID)mBuffer.get(),
									(DWORD)mBufferSize,
									(BOOL)mRecurse,
									mFlags,
									&bytesGot,
									&mOverlapped,
									&ChangeOccurred))
		{
			return FALSE;
		}
		return TRUE;
	}

	//////////////////////////////////////////////////////////////////////

	BOOL Watch()
	{
		mDirHandle = CreateFile(mFolder.c_str(),
							   FILE_LIST_DIRECTORY,
							   FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
							   NULL,
							   OPEN_EXISTING,
							   FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
							   NULL);

		if (mDirHandle == INVALID_HANDLE_VALUE)
		{
			error($("Error opening handle to %s : %s\n"), mFolder.c_str(), GetLastErrorText().c_str());
			return FALSE;
		}
		if (!Read())
		{
			Close();
			error($("Error watching folder %s : %s\n"), mFolder.c_str(), GetLastErrorText().c_str());
			return FALSE;
		}
		return TRUE;
	}

	//////////////////////////////////////////////////////////////////////

	void Close()
	{
		if (mDirHandle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(mDirHandle);
			mDirHandle = INVALID_HANDLE_VALUE;
		}

		if (mHandle != INVALID_HANDLE_VALUE)
		{
			FindCloseChangeNotification(mHandle);
			mHandle = INVALID_HANDLE_VALUE;
		}
	}

	//////////////////////////////////////////////////////////////////////

	~Watcher()
	{
		Close();
	}
};
