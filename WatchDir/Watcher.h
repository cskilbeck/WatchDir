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

	void OnChange(DWORD error, DWORD numBytes, LPOVERLAPPED overlappedPtr)
	{
		if (error != ERROR_SUCCESS)
		{
			std::wcerr << "Error " << error << ", stopping watching " << folder << std::endl;
		}
		else
		{
			FILE_NOTIFY_INFORMATION *f = (FILE_NOTIFY_INFORMATION *)(buffer);
			while (f->NextEntryOffset != 0)
			{
				string filename(f->FileName, (size_t)f->FileNameLength);
				string change = GetChangeName(f->Action);
				std::wcout << change << " occurred on " << filename << std::endl;
				f = (FILE_NOTIFY_INFORMATION *)(((byte *)f) + f->NextEntryOffset);
			}
			Read();
		}
	}

	//////////////////////////////////////////////////////////////////////

	BOOL Read()
	{
		overlapped = { 0 };
		overlapped.hEvent = (HANDLE)this;
		if (!ReadDirectoryChangesW(dirHandle, (LPVOID)buffer, sizeof(buffer), recurse, flags, NULL, &overlapped, ChangeOccurred))
		{
			Close();
			return FALSE;
		}
		return TRUE;
	}

	//////////////////////////////////////////////////////////////////////

	BOOL Watch()
	{
		dirHandle = CreateFile(folder.c_str(), GENERIC_READ, FILE_LIST_DIRECTORY | FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
		if (dirHandle == INVALID_HANDLE_VALUE)
		{
			return FALSE;
		}
		if (!Read())
		{
			Close();
			std::wcerr << "Error watching folder " << folder << " Error: " << GetLastError() << std::endl;
			return FALSE;
		}
		else
		{
			std::wcout << "Folder: " << folder
				<< " Conditions: " << flags
				<< " Recurse: " << recurse
				<< " Action: " << command
				<< std::endl;
			return TRUE;
		}
	}

	//////////////////////////////////////////////////////////////////////

	void Close()
	{
		if (dirHandle != INVALID_HANDLE_VALUE) {
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
		vector<wchar> cmd(command.size() + 1);
		memcpy(cmd.data(), command.c_str(), command.size() * sizeof(wchar));
		cmd[cmd.size() - 1] = 0;
		STARTUPINFO si = { 0 };
		PROCESS_INFORMATION pi = { 0 };
		si.cb = sizeof(si);
		BOOL b = CreateProcessW(NULL, cmd.data(), NULL, NULL, TRUE, 0, NULL, folder.c_str(), &si, &pi);
		if (b == NULL)
		{
			std::wcerr << "Error creating process " << command << " Error: " << GetLastError() << std::endl;
		}
		else
		{
			std::wcout << "Spawned " << command << std::endl;
			WaitForSingleObject(pi.hProcess, INFINITE);
			std::wcout << "Completed " << command << std::endl;
		}
	}
};

// C:\Temp, FILE_NAME + DIR_NAME + LAST_WRITE, recursive, cmd /c dir