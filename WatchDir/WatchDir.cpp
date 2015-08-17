//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////

using string = std::string;
template<typename T> using vector = std::vector<T>;

//////////////////////////////////////////////////////////////////////

template <class container_t, class string_t, class char_t>
void tokenize(string_t const &str, container_t &tokens, char_t const *delimiters, bool includeEmpty = true)
{
	string_t::size_type end = 0, start = 0, len = str.size();
	while(end < len)
	{
		end = str.find_first_of(delimiters, start);
		if(end == string_t::npos)
		{
			end = len;
		}
		if(end != start || includeEmpty)
		{
			tokens.push_back(container_t::value_type(str.data() + start, end - start));
		}
		start = end + 1;
	}
}

//////////////////////////////////////////////////////////////////////

template <class container_t, class char_t>
void tokenize(char_t const *str, container_t &tokens, char_t const *delimiters, bool includeEmpty = true)
{
	assert(str != null);
	assert(delimiters != null);

	size_t delimCount = Length(delimiters);
	size_t len = Length(str);	// LAME
	char_t const *start = str, *end = str, *stop = start + len;
	while(start < stop)
	{
		end = FindFirstOf(start, delimiters, delimCount);
		if(end == null)
		{
			end = stop;
		}
		if(end != start || includeEmpty)
		{
			tokens.push_back(container_t::value_type(start, end - start));
		}
		start = end + 1;
	}
}

//////////////////////////////////////////////////////////////////////

string ltrim(string const &s)
{
	auto b = std::find_if_not(s.begin(), s.end(), std::isspace);
	return s.substr(b - s.begin());
}

//////////////////////////////////////////////////////////////////////

static inline string rtrim(string const &s)
{
	auto e = std::find_if_not(s.rbegin(), s.rend(), std::isspace);
	return s.substr(0, e.base() - s.begin());
}

//////////////////////////////////////////////////////////////////////

static inline string trim(string const &s)
{
	auto b = std::find_if_not(s.begin(), s.end(), std::isspace);
	auto e = std::find_if_not(s.rbegin(), s.rend(), std::isspace);
	return s.substr(b - s.begin(), e.base() - b);
}

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

vector<Condition> condition_defs = {
	{ "FILE_NAME", FILE_NOTIFY_CHANGE_FILE_NAME },
	{ "DIR_NAME", FILE_NOTIFY_CHANGE_DIR_NAME },
	{ "ATTRIBUTES", FILE_NOTIFY_CHANGE_ATTRIBUTES },
	{ "SIZE", FILE_NOTIFY_CHANGE_SIZE },
	{ "LAST_WRITE", FILE_NOTIFY_CHANGE_LAST_WRITE },
	{ "SECURITY", FILE_NOTIFY_CHANGE_SECURITY }
};

//////////////////////////////////////////////////////////////////////

struct Watcher
{
	HANDLE handle;
	string command;
	string folder;

	Watcher(HANDLE h, string cmd, string fldr)
		: handle(h)
		, command(cmd)
		, folder(fldr)
	{
	}
};

int main()
{
	vector<HANDLE> handles;
	vector<Watcher> watchers;
	string s;
	int line = 0;
	while(std::getline(std::cin, s))
	{
		vector<string> tokens;
		tokenize(s, tokens, ",", true);
		if(tokens.size() == 4)
		{
			DWORD flags = 0;
			vector<string> conditions;
			tokenize(tokens[1], conditions, "+", true);
			for(auto const &condition : conditions)
			{
				auto c = std::find(condition_defs.begin(), condition_defs.end(), condition);
				if(c < condition_defs.end())
				{
					flags += c->flag;
				}
				else
				{
					std::cerr << "Unknown flag " << condition << " at line " << line << std::endl;
				}
			}
			if(flags == 0)
			{
				std::cerr << "No known flags on line " << line << std::endl;
			}
			else
			{
				string rc = trim(tokens[2]);
				BOOL recurse = _stricmp(rc.c_str(), "true") == 0 || _stricmp(rc.c_str(), "recursive") == 0;
				HANDLE h = FindFirstChangeNotification(tokens[0].c_str(), recurse, flags);
				if(h == INVALID_HANDLE_VALUE)
				{
					std::cerr << "Error watching folder " << tokens[0] << " Error: " << GetLastError() << std::endl;
				}
				else
				{
					std::cout << "Folder: " << tokens[0]
						<< " Conditions: " << flags
						<< " Recurse: " << recurse
						<< " Action: " << tokens[3]
						<< std::endl;
					handles.push_back(h);
					watchers.push_back(Watcher(h, trim(tokens[3]), tokens[0]));
				}
			}
		}
		else
		{
			std::cerr << "Bad input at line " << line << std::endl;
		}
		++line;
	}
	if(handles.empty())
	{
		std::cerr << "No folders to watch, exiting..." << std::endl;
	}
	else
	{
		std::cout << "Waiting for activity on " << handles.size() << " folders" << std::endl;
		while(true)
		{
			DWORD hit = WaitForMultipleObjects((DWORD)handles.size(), handles.data(), FALSE, INFINITE);
			if(hit == WAIT_TIMEOUT || hit < WAIT_OBJECT_0 || hit >= WAIT_OBJECT_0 + handles.size())
			{
				std::cerr << "Error waiting for folder changes " << GetLastError() << " continuing to wait..." << std::endl;
			}
			// execute the appropriate command
			Watcher const &w = watchers[hit - WAIT_OBJECT_0];
			vector<char> cmd(w.command.size() + 1);
			memcpy(cmd.data(), w.command.c_str(), w.command.size());
			cmd[cmd.size() - 1] = 0;
			STARTUPINFO si = { 0 };
			PROCESS_INFORMATION pi = { 0 };
			si.cb = sizeof(si);
			BOOL b = CreateProcess(NULL, cmd.data(), NULL, NULL, TRUE, 0, NULL, w.folder.c_str(), &si, &pi);
			if(!b)
			{
				std::cerr << "Error creating process " << w.command << " Error: " << GetLastError() << std::endl;
			}
			else
			{
				std::cout << "Spawned " << w.command << std::endl;
			}
			WaitForSingleObject(pi.hProcess, INFINITE);
			if(!FindNextChangeNotification(w.handle))
			{
				std::cerr << "Error re-watching " << w.folder << " Error: " << GetLastError() << std::endl;
			}
		}
	}
	getchar();
	return 0;
}
