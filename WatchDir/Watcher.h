//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Watcher
{
	HANDLE							mDirHandle;
	HANDLE							mHandle;
	vector<Command>					mCommands;
	tstring							mFolder;
	bool							mRecurse;
	DWORD							mFlags;
	ptr<byte>						mBuffer;
	size_t							mBufferSize;
	OVERLAPPED						mOverlapped;
	thread_safe_queue<FileEvent *>	mQueue;
	std::thread						mThread;
	HANDLE							mThreadHandle;

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
	{
		xml_node<> *pathNode = watch->first_node("path");
		xml_node<> *recursiveNode = watch->first_node("recursive");
		xml_node<> *triggersNode = watch->first_node("triggers");
		xml_node<> *commandNode = watch->first_node("command");
		if(pathNode == null || triggersNode == null || commandNode == null)
		{
			throw err_bad_input;
		}
		mFolder = ExpandEnvironment(TString(pathNode->val()));
		mRecurse = icmp(recursiveNode->val(), "true") == 0;
		mFlags = GetFlags(triggersNode->val());

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
			FileEvent *v = mQueue.remove();
			Execute(v);
		}
	}

	//////////////////////////////////////////////////////////////////////

	void Execute(FileEvent *v)
	{
		for(auto &cmd : mCommands)
		{
			cmd.Execute(mFolder, v);
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
				mQueue.add(new FileEvent(f->Action, filename, tstring()));
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
									mBufferSize,
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

// C:\Temp, FILE_NAME + DIR_NAME + ATTRIBUTES + SIZE + LAST_WRITE + SECURITY + CREATION, recursive, cmd /c dir