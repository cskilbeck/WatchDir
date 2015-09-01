//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Condition
{
	char const *name;
	DWORD flag;

	static vector<Condition> condition_defs;

	static DWORD GetFlags(string const &triggers);
	static DWORD const writeFlags;
	static DWORD const readFlags;

	bool operator == (string const &n)
	{
		return _stricmp(trim(n).c_str(), name) == 0;
	}
};

