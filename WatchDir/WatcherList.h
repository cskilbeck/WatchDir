//////////////////////////////////////////////////////////////////////

#pragma once

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

	bool ReadInput()
	{
		string s;
		int line = 0;
		while (std::getline(std::wcin, s))
		{
			vector<string> tokens;
			tokenize(s, tokens, L",", true);
			if (tokens.size() == 4)
			{
				DWORD flags = 0;
				vector<string> conditions;
				tokenize(tokens[1], conditions, L"+", true);
				for (auto const &condition : conditions)
				{
					auto c = std::find(condition_defs.begin(), condition_defs.end(), condition);
					if (c < condition_defs.end())
					{
						flags += c->flag;
					}
					else
					{
						std::wcerr << "Unknown flag " << condition << " at line " << line << std::endl;
					}
				}
				if (flags == 0)
				{
					std::wcerr << "No known flags on line " << line << std::endl;
				}
				else
				{
					string rc = trim(tokens[2]);
					BOOL recurse = _wcsicmp(rc.c_str(), L"true") == 0 || _wcsicmp(rc.c_str(), L"recursive") == 0;
					Add(tokens[0], tokens[3], recurse, flags);
				}
			}
			else
			{
				std::cerr << "Bad input at line " << line << std::endl;
			}
			++line;
		}
		if (Count() == 0)
		{
			std::cerr << "No folders to watch, exiting..." << std::endl;
			return false;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////

	bool StartWatching()
	{
		std::cout << "Waiting for activity on " << Count() << " folders" << std::endl;
		int n = 0;
		for (auto &w : watchers)
		{
			if (w.Watch())
			{
				++n;
			}
		}
		return n > 0;
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
			std::wcout << "Change detected in folder " << w.folder << std::endl;
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

