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
						flags |= c->flag;
					}
					else
					{
						error(L"Unknown flag %s at line %d\n", condition.c_str(), line);
					}
				}
				if (flags == 0)
				{
					error(L"No known flags on line %d\n", line);
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
				error(L"Bad input at line %d\n", line);
			}
			++line;
		}
		if (Count() == 0)
		{
			error(L"No folders to watch, exiting...\n");
			return false;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////

	bool StartWatching()
	{
		wprintf(L"Waiting for activity on %d folders\n", Count());
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
			error(L"Error waiting for folder changes: %s (continuing to wait...)\n", GetLastErrorText().c_str());
		}
		else
		{
			Watcher const &w = watchers[hit - WAIT_OBJECT_0];
			wprintf(L"Change detected in folder %s\n", w.folder.c_str());
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

