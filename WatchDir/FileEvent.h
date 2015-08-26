//////////////////////////////////////////////////////////////////////

#pragma once

#include <PathCch.h>
#pragma comment(lib, "pathcch.lib")

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

	DWORD mAction;
	tstring mFilePath;
	tstring mOldFilePath;

	tstring mDrive;
	tstring mDir;
	tstring mName;
	tstring mExt;

	tstring mOldDrive;
	tstring mOldDir;
	tstring mOldName;
	tstring mOldExt;

	//////////////////////////////////////////////////////////////////////

	tchar const *ChangeName() const
	{
		return changeNames.get(mAction, $(""));
	}

	//////////////////////////////////////////////////////////////////////

	tstring OldName() const
	{
		return (mAction == FILE_ACTION_RENAMED_NEW_NAME) ? mOldFilePath : tstring();
	}

	//////////////////////////////////////////////////////////////////////

	tstring Details() const
	{
		return Format($("%s %s %s"), mFilePath.c_str(), ChangeName(), OldName().c_str());
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

	void SetOldFilePath(tstring const &folder, tstring const &oldFilePath)
	{
		mOldFilePath = GetJoinedPath(folder, oldFilePath);
		tchar drive[_MAX_DRIVE];
		tchar dir[_MAX_DIR];
		tchar name[_MAX_FNAME];
		tchar ext[_MAX_EXT];
		_tsplitpath_s(mOldFilePath.c_str(), drive, dir, name, ext);
		mOldDrive = drive;
		mOldDir = dir;
		mOldName = name;
		mOldExt = ext;
	}

	//////////////////////////////////////////////////////////////////////

	FileEvent(DWORD action, tstring const &folder, tstring const &filepath)
		: mAction(action)
		, mFilePath(filepath)
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
};
