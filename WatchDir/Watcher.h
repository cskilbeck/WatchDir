//////////////////////////////////////////////////////////////////////

#pragma once

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

	using lock = std::lock_guard<std::mutex>;

	//////////////////////////////////////////////////////////////////////

	struct Signal
	{
		Watcher *mWatcher;
		list<FileEvent *> *mQueue;

		//////////////////////////////////////////////////////////////////////

		Signal(Watcher *watcher)
			: mWatcher(watcher)
			, mQueue(new list<FileEvent *>())
		{
		}

		//////////////////////////////////////////////////////////////////////

		~Signal()
		{
			delete mQueue;
		}

		//////////////////////////////////////////////////////////////////////

		void Execute()
		{
			// do %{filename} replacements
			// mask based on 'on' attribute

			tprintf($("======================================================================\n"));
			FileEvent *old = null;
			for (auto p : *mQueue)
			{
				if (p->mAction == FILE_ACTION_RENAMED_OLD_NAME)
				{
					old = p;
				}
				else
				{
					if (p->mAction == FILE_ACTION_RENAMED_NEW_NAME && old != null)
					{
						p->SetOldFilePath(mWatcher->mFolder, old->mFilePath);
						old = null;
					}
					tprintf($("\t%s\n"), p->Details().c_str());
					for (auto &cmd : mWatcher->mCommands)
					{
						cmd.Execute(mWatcher->mFolder, p);
					}
				}
			}
		}
	};

	//////////////////////////////////////////////////////////////////////

	void OnTimer()
	{
		Signal *signal = new Signal(this);
		while (!mQueue.empty())
		{
			// not the end of the world if something is added to the queue in here...
			signal->mQueue->push_back(mQueue.remove());
		}
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
		, mTimer(this)
		, mSettleDelay(0.25)
	{
		xml_node<> *pathNode = watch->first_node("path");
		xml_node<> *recursiveNode = watch->first_node("recursive");
		xml_node<> *triggersNode = watch->first_node("triggers");
		xml_node<> *commandNode = watch->first_node("command");
		xml_node<> *settleDelayNode = watch->first_node("settleDelay");
		if(pathNode == null || triggersNode == null || commandNode == null)
		{
			throw err_bad_input;
		}
		mFolder = ExpandEnvironment(TString(pathNode->val()));
		mRecurse = icmp(recursiveNode->val(), "true") == 0;
		mFlags = GetFlags(triggersNode->val());
		if (settleDelayNode != null)
		{
			mSettleDelay = atof(settleDelayNode->val().c_str());
			if (mSettleDelay == 0.0)
			{
				error($("Invalid settle delay: %s\n"), settleDelayNode->val().c_str());
				mSettleDelay = 0.25;
			}
		}
		
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
