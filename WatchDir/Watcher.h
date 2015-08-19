//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

__declspec(selectany) nameMap changeNames = 
{
	{ FILE_ACTION_ADDED, L"File Added" },
	{ FILE_ACTION_REMOVED, L"File Removed" },
	{ FILE_ACTION_MODIFIED, L"File Modified" },
	{ FILE_ACTION_RENAMED_OLD_NAME, L"File Renamed from" },
	{ FILE_ACTION_RENAMED_NEW_NAME, L"File Renamed to" }
};

//////////////////////////////////////////////////////////////////////

static inline string const &GetChangeName(DWORD type)
{
	auto f = changeNames.find(type);
	if (f != changeNames.end())
	{
		return f->second;
	}
	static const string empty;
	return empty;
}

//////////////////////////////////////////////////////////////////////

struct Watcher
{
	HANDLE dirHandle;
	HANDLE handle;
	string command;
	string folder;
	BOOL recurse;
	DWORD flags;
	byte buffer[65536];
	OVERLAPPED overlapped;

	//////////////////////////////////////////////////////////////////////

	Watcher(string cmd, string fldr, BOOL recurse, DWORD flags)
		: handle(INVALID_HANDLE_VALUE)
		, dirHandle(INVALID_HANDLE_VALUE)
		, command(trim(cmd))
		, folder(trim(fldr))
		, recurse(recurse)
		, flags(flags)
	{
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
			error(L"Error %d stopping watching %s\n", errorCode, folder.c_str());
		}
		else
		{
			wprintf(L"Activity detected:\n");
			FILE_NOTIFY_INFORMATION *f = (FILE_NOTIFY_INFORMATION *)(buffer);
			DWORD offset;
			int n = 0;
			do
			{
				++n;
				offset = f->NextEntryOffset;
				string filename(f->FileName, (size_t)(f->FileNameLength / sizeof(string::value_type)));
				string change = GetChangeName(f->Action);

				// Lock the queue, add the event

				wprintf(L"%s occurred on %s\n", change.c_str(), filename.c_str());
				f = (FILE_NOTIFY_INFORMATION *)((byte *)f + offset);
			} while (offset != 0);
			wprintf(L"%d events processed\n", n);
			// cancel timer if it's running
			// set the timer to fire in Nms

			// kick off watching again
			Read();
		}
	}

	//////////////////////////////////////////////////////////////////////

	void ResetTimer()
	{
	}

	//////////////////////////////////////////////////////////////////////

	static void CALLBACK TIMERPROC(HWND, UINT, UINT_PTR, DWORD)
	{
	}

	//////////////////////////////////////////////////////////////////////

	BOOL Read()
	{
		DWORD bytesGot = 0;
		overlapped = { 0 };
		overlapped.hEvent = (HANDLE)this;
		if (!ReadDirectoryChangesW(dirHandle, (LPVOID)buffer, sizeof(buffer), recurse, flags, &bytesGot, &overlapped, &ChangeOccurred))
		{
			return FALSE;
		}
		return TRUE;
	}

	//////////////////////////////////////////////////////////////////////

	BOOL Watch()
	{
		dirHandle = CreateFile(folder.c_str(), FILE_LIST_DIRECTORY, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
		if (dirHandle == INVALID_HANDLE_VALUE)
		{
			error(L"Error opening handle to %s : %s\n", folder.c_str(), GetLastErrorText().c_str());
			return FALSE;
		}
		if (!Read())
		{
			Close();
			error(L"Error watching folder %s : %s\n", folder.c_str(), GetLastErrorText().c_str());
			return FALSE;
		}
		else
		{
			wprintf(L"Folder: %s, Conditions: %04x, Recurse: %s, Action: %s\n", folder.c_str(), flags, recurse ? L"Yes" : L"No", command.c_str());
			return TRUE;
		}
	}

	//////////////////////////////////////////////////////////////////////

	void Close()
	{
		if (dirHandle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(dirHandle);
			dirHandle = INVALID_HANDLE_VALUE;
		}

		if (handle != INVALID_HANDLE_VALUE)
		{
			FindCloseChangeNotification(handle);
			handle = INVALID_HANDLE_VALUE;
		}
	}

	//////////////////////////////////////////////////////////////////////

	~Watcher()
	{
		Close();
	}

	//////////////////////////////////////////////////////////////////////

	void Exec() const
	{
		vector<string::value_type> cmd(command.size() + 1);
		memcpy(cmd.data(), command.c_str(), command.size() * sizeof(string::value_type));
		cmd[cmd.size() - 1] = 0;
		STARTUPINFO si = { 0 };
		PROCESS_INFORMATION pi = { 0 };
		si.cb = sizeof(si);
		wprintf(L"Spawning %s\n", command.c_str());
		BOOL b = CreateProcessW(NULL, cmd.data(), NULL, NULL, TRUE, 0, NULL, folder.c_str(), &si, &pi);
		if (b == NULL)
		{
			error(L"Error creating process %s, Error: %s\n", command.c_str(), GetLastErrorText().c_str());
		}
		else
		{
			WaitForSingleObject(pi.hProcess, INFINITE);
			wprintf(L"Completed %s\n", command);
		}
	}
};

// C:\Temp, FILE_NAME + DIR_NAME + ATTRIBUTES + SIZE + LAST_WRITE + SECURITY + CREATION, recursive, cmd /c dir