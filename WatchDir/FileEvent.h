//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

__declspec(selectany) std::map<DWORD, tchar const *> changeNames =
{
	{ FILE_ACTION_ADDED, $("Added") },
	{ FILE_ACTION_REMOVED, $("Removed") },
	{ FILE_ACTION_MODIFIED, $("Modified") },
	{ FILE_ACTION_RENAMED_OLD_NAME, $("Renamed") },
	{ FILE_ACTION_RENAMED_NEW_NAME, $("is the new name") }
};

//////////////////////////////////////////////////////////////////////

static inline tchar const *GetChangeName(DWORD type)
{
	auto f = changeNames.find(type);
	if(f != changeNames.end())
	{
		return f->second;
	}
	static tchar const *empty = $("");
	return empty;
}

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

	enum ActionFlag: uint32
	{
		Added = 1,
		Removed = 2,
		Modified = 4,
		Renamed = 8
	};

	DWORD mAction;			// FILE_ACTION_[ADDED|REMOVED|MODIFIED|RENAMED_NEW_NAME] (RENAMED_OLD_NAME is coalesced into RENAMED_NEW_NAME and oldfilename is set)
	tstring mFilePath;
	tstring mOldFilePath;

	tstring mDrive;
	tstring mDir;
	tstring mName;
	tstring mExt;

	//////////////////////////////////////////////////////////////////////

	FileEvent(DWORD action, tstring const &filepath, tstring const &oldname)
		: mAction(action)
		, mFilePath(filepath)
		, mOldFilePath(oldname)
	{
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
