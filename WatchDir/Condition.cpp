#include "stdafx.h"

//////////////////////////////////////////////////////////////////////

DWORD const Condition::writeFlags = FILE_NOTIFY_CHANGE_FILE_NAME |
									FILE_NOTIFY_CHANGE_DIR_NAME |
									FILE_NOTIFY_CHANGE_CREATION |
									FILE_NOTIFY_CHANGE_SIZE |
									FILE_NOTIFY_CHANGE_LAST_WRITE |
									FILE_NOTIFY_CHANGE_SECURITY |
									FILE_NOTIFY_CHANGE_ATTRIBUTES;

DWORD const Condition::readFlags = FILE_NOTIFY_CHANGE_LAST_ACCESS;

//////////////////////////////////////////////////////////////////////

vector<Condition> Condition::condition_defs =
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

//////////////////////////////////////////////////////////////////////

DWORD Condition::GetFlags(string const &triggers)
{
	vector<string> tokens;
	tokenize(triggers.c_str(), tokens, " ;,", false);
	DWORD flags = 0;
	for (auto const &t : tokens)
	{
		auto c = std::find(condition_defs.begin(), condition_defs.end(), t);
		if (c != condition_defs.end())
		{
			flags |= c->flag;
		}
		else
		{
			error("Unknown condition %s\n", t.c_str());
			throw err_bad_input;
		}
	}
	return flags;
}
