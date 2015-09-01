//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

__declspec(selectany) Map<DWORD, tchar const *> changeNames =
{
	{ FILE_ACTION_ADDED, $("Added") },
	{ FILE_ACTION_REMOVED, $("Removed") },
	{ FILE_ACTION_MODIFIED, $("Modified") },
	{ FILE_ACTION_RENAMED_OLD_NAME, $("[Renamed]") },
	{ FILE_ACTION_RENAMED_NEW_NAME, $("Renamed from ") }
};

//////////////////////////////////////////////////////////////////////

struct Watcher;

struct FileEvent
{
	struct PathComponents
	{
		tchar drive[_MAX_DRIVE];
		tchar dir[_MAX_DIR];
		tchar name[_MAX_FNAME];
		tchar ext[_MAX_EXT];
	};

	enum ActionFlag : uint32
	{
		FA_ADDED = 1 << (FILE_ACTION_ADDED - 1),
		FA_REMOVED = 1 << (FILE_ACTION_REMOVED - 1),
		FA_MODIFIED = 1 << (FILE_ACTION_MODIFIED - 1),
		FA_RENAMED = 1 << (FILE_ACTION_RENAMED_NEW_NAME - 1),
		FA_ALL = (FA_ADDED | FA_REMOVED | FA_MODIFIED | FA_RENAMED)
	};

	Watcher *mWatcher;

	DWORD mAction;
	tstring mRelativePath;
	tstring mFilePath;
	tstring mNewFilePath;

	tstring mDrive;
	tstring mDir;
	tstring mName;
	tstring mExt;

	tstring mNewDrive;
	tstring mNewDir;
	tstring mNewName;
	tstring mNewExt;

	//////////////////////////////////////////////////////////////////////

	tchar const *ChangeName() const
	{
		return changeNames.get(mAction, $(""));
	}

	//////////////////////////////////////////////////////////////////////

	tstring NewName() const
	{
		return (mAction == FILE_ACTION_RENAMED_NEW_NAME) ? mNewFilePath : tstring();
	}

	//////////////////////////////////////////////////////////////////////

	tstring Details() const
	{
		return Format($("%s %s %s"), mFilePath.c_str(), ChangeName(), NewName().c_str());
	}

	//////////////////////////////////////////////////////////////////////

	bool IsFile() const
	{
		if (mAction == FILE_ACTION_REMOVED)
		{

		}
		if (mNewFilePath.empty())
		{
			return IsFileOrFolder(mFilePath.c_str()) == FileOrFolder::File;
		}
		else
		{
			return IsFileOrFolder(mNewFilePath.c_str()) == FileOrFolder::File;
		}
	}

	//////////////////////////////////////////////////////////////////////

	tstring GetJoinedPath(tstring const &folder, tstring const &filepath)
	{
		wstring fldr = WString(folder);
		ptr<wchar> path(new wchar[32768]);
		ptr<wchar> canonicalPath(new wchar[32768]);
		memcpy(path.get(), fldr.c_str(), sizeof(wchar) * (fldr.size() + 1));
		if (FAILED(PathCchAppendEx(path.get(), 32768, WString(filepath).c_str(), PATHCCH_ALLOW_LONG_PATHS)))
		{
			throw err_bad_input;
		}
		if (FAILED(PathCchCanonicalizeEx(canonicalPath.get(), 32768, path.get(), PATHCCH_ALLOW_LONG_PATHS)))
		{
			throw err_bad_input;
		}
		return TString(canonicalPath.get());
	}

	//////////////////////////////////////////////////////////////////////

	void SetFilePath(tstring const &folder, tstring const &filepath)
	{
		mFilePath = GetJoinedPath(folder, filepath);
		tchar drive[_MAX_DRIVE];
		tchar dir[_MAX_DIR];
		tchar name[_MAX_FNAME];
		tchar ext[_MAX_EXT];
		_tsplitpath_s(mFilePath.c_str(), drive, dir, name, ext);
		mDrive = drive;
		mDir = dir;
		mName = name;
		mExt = ext;
	}

	//////////////////////////////////////////////////////////////////////

	void SetNewFilePath(tstring const &folder, tstring const &oldFilePath)
	{
		mNewFilePath = GetJoinedPath(folder, oldFilePath);
		tchar drive[_MAX_DRIVE];
		tchar dir[_MAX_DIR];
		tchar name[_MAX_FNAME];
		tchar ext[_MAX_EXT];
		_tsplitpath_s(mNewFilePath.c_str(), drive, dir, name, ext);
		mNewDrive = drive;
		mNewDir = dir;
		mNewName = name;
		mNewExt = ext;
	}

	//////////////////////////////////////////////////////////////////////

	FileEvent(DWORD action, tstring const &folder, tstring const &filepath, tstring const &watcherFolder, Watcher *watcher)
		: mAction(action)
		, mWatcher(watcher)
	{
		SetFilePath(folder, filepath);
		mRelativePath = GetRelativePath(mFilePath, watcherFolder);
	}
};
