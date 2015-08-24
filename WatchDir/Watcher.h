//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

__declspec(selectany) std::map<DWORD, tchar const *> changeNames = 
{
	{ FILE_ACTION_ADDED, $("File Added") },
	{ FILE_ACTION_REMOVED, $("File Removed") },
	{ FILE_ACTION_MODIFIED, $("File Modified") },
	{ FILE_ACTION_RENAMED_OLD_NAME, $("File Renamed from") },
	{ FILE_ACTION_RENAMED_NEW_NAME, $("File Renamed to") }
};

//////////////////////////////////////////////////////////////////////

static inline tchar const *GetChangeName(DWORD type)
{
	auto f = changeNames.find(type);
	if (f != changeNames.end())
	{
		return f->second;
	}
	static tchar const *empty = $("");
	return empty;
}

//////////////////////////////////////////////////////////////////////

struct Watcher
{
	HANDLE						mDirHandle;
	HANDLE						mHandle;
	vector<Command>				mCommands;
	tstring						mFolder;
	bool						mRecurse;
	DWORD						mFlags;
	ptr<byte>					mBuffer;
	size_t						mBufferSize;
	OVERLAPPED					mOverlapped;
	thread_safe_queue<Event *>	mQueue;
	std::thread					mThread;
	HANDLE						mThreadHandle;

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
		tprintf($("Thread is waiting for events...\n"));
		while(true)
		{
			Event *v = mQueue.remove();
			tprintf($("File event: \n"));
			if(v->Handle())
			{
				break;
			}
		}
		tprintf($("Thread finished.\n"));
	}

	//////////////////////////////////////////////////////////////////////

	static void CALLBACK ChangeOccurred(DWORD error, DWORD numBytes, LPOVERLAPPED overlappedPtr)
	{
		((Watcher *)(overlappedPtr->hEvent))->OnChange(error, numBytes, overlappedPtr);
	}

	//////////////////////////////////////////////////////////////////////

	void OnChange(DWORD errorCode, DWORD numBytes, LPOVERLAPPED overlappedPtr)
	{
		// add a FileEvent to the queue
		if (errorCode != ERROR_SUCCESS)
		{
			error($("Error %d stopping watching %s\n"), errorCode, mFolder.c_str());
		}
		else
		{
			tprintf($("Activity detected:\n"));
			FILE_NOTIFY_INFORMATION *f = (FILE_NOTIFY_INFORMATION *)(mBuffer.get());
			DWORD offset;
			int n = 0;
			do
			{
				++n;
				offset = f->NextEntryOffset;
				size_t len = (size_t)(f->FileNameLength / sizeof(wstring::value_type));
				tstring filename = TString(wstring((wchar *)f->FileName, len));
				tstring change = GetChangeName(f->Action);

				tprintf($("%s occurred on %s\n"), change.c_str(), filename.c_str());

				// handle renames strangely

				// Lock the queue, add the event
				FileEvent *fe = new FileEvent(f->Action, filename, tstring());
				mQueue.add(fe);

				f = (FILE_NOTIFY_INFORMATION *)((byte *)f + offset);
			} while (offset != 0);
			tprintf($("%d events processed\n"), n);
			// cancel timer if it's running
			// set the timer to fire in Nms

			// kick off watching again
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
		else
		{
//			tprintf($("Folder: %s, Conditions: %04x, Recurse: %s, Action: %s\n"), folder.c_str(), flags, recurse ? L"Yes" : L"No", command.c_str());
			return TRUE;
		}
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
		mQueue.add(new QuitEvent());
		Close();
		tprintf($("Sending Quit to thread...\n"));
		if(WaitForSingleObject(mThreadHandle, 1000) != WAIT_OBJECT_0)
		{
			// ask user here if they'd like to kill the thread or wait? (MessageBox?)
			tprintf($("Killing thread.\n"));
			TerminateThread(mThreadHandle, 0);
		}
	}

	//////////////////////////////////////////////////////////////////////

	void Execute() const
	{
		tstring command($("cmd /c dir"));
		vector<tstring::value_type> cmd(command.size() + 1);
		memcpy(cmd.data(), command.c_str(), command.size() * sizeof(tstring::value_type));
		cmd[cmd.size() - 1] = 0;
		STARTUPINFO si = { 0 };
		PROCESS_INFORMATION pi = { 0 };
		si.cb = sizeof(si);
		tprintf($("Spawning %s\n"), command.c_str());
		BOOL b = CreateProcess(NULL, cmd.data(), NULL, NULL, TRUE, 0, NULL, mFolder.c_str(), &si, &pi);
		if (b == NULL)
		{
			error($("Error creating process %s, Error: %s\n"), command.c_str(), GetLastErrorText().c_str());
		}
		else
		{
			WaitForSingleObject(pi.hProcess, INFINITE);
			tprintf($("Completed %s\n"), command);
		}
	}
};

// C:\Temp, FILE_NAME + DIR_NAME + ATTRIBUTES + SIZE + LAST_WRITE + SECURITY + CREATION, recursive, cmd /c dir