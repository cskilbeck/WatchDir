//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Condition
{
	char const *name;
	DWORD flag;

	bool operator == (string const &n)
	{
		return _stricmp(trim(n).c_str(), name) == 0;
	}
};

//////////////////////////////////////////////////////////////////////

static const DWORD writeFlags = FILE_NOTIFY_CHANGE_FILE_NAME |
								FILE_NOTIFY_CHANGE_DIR_NAME |
								FILE_NOTIFY_CHANGE_CREATION |
								FILE_NOTIFY_CHANGE_LAST_WRITE |
								FILE_NOTIFY_CHANGE_ATTRIBUTES;

static const DWORD readFlags = FILE_NOTIFY_CHANGE_LAST_ACCESS;

__declspec(selectany) vector<Condition> condition_defs =
{
	// explicit
	{ "file_name", FILE_NOTIFY_CHANGE_FILE_NAME },
	{ "dir_name", FILE_NOTIFY_CHANGE_DIR_NAME },
	{ "attributes", FILE_NOTIFY_CHANGE_ATTRIBUTES },
	{ "size", FILE_NOTIFY_CHANGE_SIZE },
	{ "last_write", FILE_NOTIFY_CHANGE_LAST_WRITE },
	{ "access", FILE_NOTIFY_CHANGE_LAST_ACCESS },
	{ "creation", FILE_NOTIFY_CHANGE_CREATION },
	{ "security", FILE_NOTIFY_CHANGE_SECURITY },

	// simple
	{ "write", writeFlags },
	{ "read", readFlags }
};
