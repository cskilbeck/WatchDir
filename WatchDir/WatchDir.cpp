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
	BOOL recurse;
	DWORD flags;

	//////////////////////////////////////////////////////////////////////

	Watcher(string cmd, string fldr, BOOL recurse, DWORD flags)
		: handle(INVALID_HANDLE_VALUE)
		, command(trim(cmd))
		, folder(trim(fldr))
		, recurse(recurse)
		, flags(flags)
	{
	}

	//////////////////////////////////////////////////////////////////////

	bool IsValid() const
	{
		return handle != INVALID_HANDLE_VALUE;
	}

	//////////////////////////////////////////////////////////////////////

	BOOL Watch()
	{
		handle = FindFirstChangeNotification(folder.c_str(), recurse, flags);
		if (handle == INVALID_HANDLE_VALUE)
		{
			std::cerr << "Error watching folder " << folder << " Error: " << GetLastError() << std::endl;
			return FALSE;
		}
		else
		{
			std::cout << "Folder: " << folder
				<< " Conditions: " << flags
				<< " Recurse: " << recurse
				<< " Action: " << command
				<< std::endl;
			return TRUE;
		}
	}

	//////////////////////////////////////////////////////////////////////

	~Watcher()
	{
		if (handle != INVALID_HANDLE_VALUE)
		{
			FindCloseChangeNotification(handle);
			handle = INVALID_HANDLE_VALUE;
		}
	}

	//////////////////////////////////////////////////////////////////////

	void Exec() const
	{
		vector<char> cmd(command.size() + 1);
		memcpy(cmd.data(), command.c_str(), command.size());
		cmd[cmd.size() - 1] = 0;
		STARTUPINFO si = { 0 };
		PROCESS_INFORMATION pi = { 0 };
		si.cb = sizeof(si);
		BOOL b = CreateProcess(NULL, cmd.data(), NULL, NULL, TRUE, 0, NULL, folder.c_str(), &si, &pi);
		if (b == NULL)
		{
			std::cerr << "Error creating process " << command << " Error: " << GetLastError() << std::endl;
		}
		else
		{
			std::cout << "Spawned " << command << std::endl;
			WaitForSingleObject(pi.hProcess, INFINITE);
			std::cout << "Completed " << command << std::endl;
		}
		if (!FindNextChangeNotification(handle))
		{
			std::cerr << "Error re-watching " << folder << " Error: " << GetLastError() << std::endl;
		}
	}
};

//////////////////////////////////////////////////////////////////////

struct WatcherList
{
	vector<HANDLE> handles;
	vector<Watcher> watchers;

	//////////////////////////////////////////////////////////////////////

	void Add(string const &folder, string const &command, BOOL recurse, DWORD flags)
	{
		watchers.push_back(Watcher(command, folder, recurse, flags));
	}

	//////////////////////////////////////////////////////////////////////

	void ReadInput()
	{
		string s;
		int line = 0;
		while (std::getline(std::cin, s))
		{
			vector<string> tokens;
			tokenize(s, tokens, ",", true);
			if (tokens.size() == 4)
			{
				DWORD flags = 0;
				vector<string> conditions;
				tokenize(tokens[1], conditions, "+", true);
				for (auto const &condition : conditions)
				{
					auto c = std::find(condition_defs.begin(), condition_defs.end(), condition);
					if (c < condition_defs.end())
					{
						flags += c->flag;
					}
					else
					{
						std::cerr << "Unknown flag " << condition << " at line " << line << std::endl;
					}
				}
				if (flags == 0)
				{
					std::cerr << "No known flags on line " << line << std::endl;
				}
				else
				{
					string rc = trim(tokens[2]);
					BOOL recurse = _stricmp(rc.c_str(), "true") == 0 || _stricmp(rc.c_str(), "recursive") == 0;
					Add(tokens[0], tokens[3], recurse, flags);
				}
			}
			else
			{
				std::cerr << "Bad input at line " << line << std::endl;
			}
			++line;
		}
	}

	//////////////////////////////////////////////////////////////////////

	void StartWatching()
	{
		for (auto &w : watchers)
		{
			if (w.Watch())
			{
				handles.push_back(w.handle);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////

	void WaitAndExec()
	{
		DWORD hit = WaitForMultipleObjects((DWORD)Count(), handles.data(), FALSE, INFINITE);
		if (hit < WAIT_OBJECT_0 || hit >= WAIT_OBJECT_0 + Count())
		{
			std::cerr << "Error waiting for folder changes " << GetLastError() << " continuing to wait..." << std::endl;
		}
		else
		{
			Watcher const &w = watchers[hit - WAIT_OBJECT_0];
			std::cout << "Change detected in folder " << w.folder << std::endl;
			w.Exec();
		}
	}

	//////////////////////////////////////////////////////////////////////

	~WatcherList()
	{
		watchers.clear();
		handles.clear();
	}

	//////////////////////////////////////////////////////////////////////

	int Count()
	{
		return (int)watchers.size();
	}
};

//////////////////////////////////////////////////////////////////////

WatcherList watchers;

//////////////////////////////////////////////////////////////////////

int main()
{
	watchers.ReadInput();

	if(watchers.Count() == 0)
	{
		std::cerr << "No folders to watch, exiting..." << std::endl;
	}
	else
	{
		std::cout << "Waiting for activity on " << watchers.Count() << " folders" << std::endl;
		watchers.StartWatching();
		while(true)
		{
			watchers.WaitAndExec();
		}
	}
	return 0;
}
