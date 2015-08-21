//////////////////////////////////////////////////////////////////////

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

struct FileEvent
{
	DWORD action;			// FILE_ACTION_[ADDED|REMOVED|MODIFIED|RENAMED_NEW_NAME] (RENAMED_OLD_NAME is coalesced into RENAMED_NEW_NAME and oldfilename is set)
	tstring filePath;
	tstring oldfilePath;

	tstring drive();		// drive letter only
	tstring path();			// path only
	tstring name();			// name only
	tstring ext();			// where
	tstring filename();		// filename with extension

	// these are only valid if action = FILE_ACTION_RENAMED_NEW_NAME
	tstring oldfilename();
	tstring oldname();		// name only
	tstring oldext();		// filename with extension

	FileEvent(DWORD action_, tchar const *filepath_, tchar const *oldname_ = null)
		: action(action_)
		, filePath(filepath_)
		, oldfilePath(oldname_)
	{
	}
};
