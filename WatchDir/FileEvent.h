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
	DWORD action;		// FILE_ACTION_[ADDED|REMOVED|MODIFIED|RENAMED_NEW_NAME] (RENAMED_OLD_NAME is coalesced into RENAMED_NEW_NAME and oldfilename is set)

	string command;		// eg. cmd /c copy %{fullpath} Z:\backup\%{path}\%{filename}	// To reconstruct full filename & path: %{drive}%{path}%{name}%{ext}

	string drive;		// drive letter only
	string path;		// path only
	string name;		// name only
	string ext;			// where
	string filename;	// filename with extension

	// these only set if action = FILE_ACTION_RENAMED_NEW_NAME
	string oldname;		// name only
	string oldfilename;	// filename with extension
};
/*



*/