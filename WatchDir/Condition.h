//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Condition
{
	wchar const *name;
	DWORD flag;

	bool operator == (string const &n)
	{
		return _wcsicmp(trim(n).c_str(), name) == 0;
	}
};

//////////////////////////////////////////////////////////////////////

__declspec(selectany) vector<Condition> condition_defs =
{
	{ L"FILE_NAME", FILE_NOTIFY_CHANGE_FILE_NAME },
	{ L"DIR_NAME", FILE_NOTIFY_CHANGE_DIR_NAME },
	{ L"ATTRIBUTES", FILE_NOTIFY_CHANGE_ATTRIBUTES },
	{ L"SIZE", FILE_NOTIFY_CHANGE_SIZE },
	{ L"LAST_WRITE", FILE_NOTIFY_CHANGE_LAST_WRITE },
	{ L"SECURITY", FILE_NOTIFY_CHANGE_SECURITY },
	{ L"CREATION", FILE_NOTIFY_CHANGE_CREATION }
};
