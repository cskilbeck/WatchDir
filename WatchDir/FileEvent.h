//////////////////////////////////////////////////////////////////////
// - batch up events behind a settle timer
// - remove duplicate events (& coalesce renames) and build full activity mask

#pragma once

//////////////////////////////////////////////////////////////////////
// EG:
// C:\foo\bar\baz.txt

// %{drive} : C:
// %{path} : \foo\bar\
// %{name} : baz
// %{ext} : .txt
// %{filename} : baz.txt
// %{fullpath} C:\foo\bar\baz.txt

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

	tstring drive();		// drive letter only
	tstring path();			// path only
	tstring name();			// name only
	tstring ext();			// where
	tstring filename();		// filename with extension

	// these are only valid if action = FILE_ACTION_RENAMED_NEW_NAME
	tstring oldfilename();
	tstring oldname();		// name only
	tstring oldext();		// filename with extension

	FileEvent(DWORD action, tstring const &filepath, tstring const &oldname)
		: mAction(action)
		, mFilePath(filepath)
		, mOldFilePath(oldname)
	{
	}

	void Handle()
	{
		tprintf($("%s %s\n"), mFilePath.c_str(), GetChangeName(mAction));
	}
};
