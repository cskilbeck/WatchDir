//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Condition
{
	tchar const *name;
	DWORD flag;

	bool operator == (tstring const &n)
	{
		return _tcsicmp(trim(n).c_str(), name) == 0;
	}
};

//////////////////////////////////////////////////////////////////////

__declspec(selectany) vector<Condition> condition_defs =
{
	{ $("FILE_NAME"), FILE_NOTIFY_CHANGE_FILE_NAME },
	{ $("DIR_NAME"), FILE_NOTIFY_CHANGE_DIR_NAME },
	{ $("ATTRIBUTES"), FILE_NOTIFY_CHANGE_ATTRIBUTES },
	{ $("SIZE"), FILE_NOTIFY_CHANGE_SIZE },
	{ $("LAST_WRITE"), FILE_NOTIFY_CHANGE_LAST_WRITE },
	{ $("SECURITY"), FILE_NOTIFY_CHANGE_SECURITY },
	{ $("CREATION"), FILE_NOTIFY_CHANGE_CREATION }
};
