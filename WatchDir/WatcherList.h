//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////
// Create a thread which waits for messages
// In the thread, copy off the q of events into a new one for processing
// then process them
// then loop

// in the callback, add event to q and (re)set timer to fire in N

// in the timer callback, post a message to the thread
// wait for q to be empty

struct WatcherList
{
	vector<HANDLE> handles;
	vector<Watcher *> watchers;

	struct Event
	{
		Watcher *watcher;
		DWORD action;		// FILE_NOTIFY_XXX etc
		// how to handle rename?
	};

	list<Watcher *> events;

	//////////////////////////////////////////////////////////////////////

	void Add(tstring const &folder, tstring const &command, BOOL recurse, DWORD flags)
	{
		watchers.push_back(new Watcher(folder, recurse, flags));
	}

	//////////////////////////////////////////////////////////////////////

	DWORD GetFlags(string const &triggers)
	{
		vector<string> tokens;
		tokenize(triggers.c_str(), tokens, " ;,", false);
		DWORD flags = 0;
		for(auto const &t : tokens)
		{
			auto c = std::find(condition_defs.begin(), condition_defs.end(), t);
			if(c != condition_defs.end())
			{
				flags |= c->flag;
			}
			else
			{
				error("Unknown condition %s\n", t.c_str());
			}
		}
		return flags;
	}

	//////////////////////////////////////////////////////////////////////

	int ReadInput(tchar const *filename)
	{
		int err = success;
		try
		{
			ptr<byte> file;
			xml_document<> doc;
			if(LoadFile(filename, file) != success)
			{
				throw err_input_not_found;
			}
			doc.parse<0>((char *)file.get());
			xml_node<> *root = doc.first_node("watchdir");
			if(root == null)
			{
				throw err_bad_input;
			}
			xml_node<> *watch = root->first_node("watch");
			if(watch == null)
			{
				throw err_bad_input;
			}
			while(watch != null)
			{
				xml_node<> *pathNode = watch->first_node("path");
				xml_node<> *recursiveNode = watch->first_node("recursive");
				xml_node<> *triggersNode = watch->first_node("triggers");
				xml_node<> *commandNode = watch->first_node("command");
				if(pathNode == null || triggersNode == null || commandNode == null)
				{
					throw err_bad_input;
				}
				tstring path = TStringFromString(string(pathNode->value(), pathNode->value_size()));
				bool recursive = icmp(string(recursiveNode->value(), recursiveNode->value_size()), "true") == 0;
				string triggers = string(triggersNode->value(), triggersNode->value_size());
				uint32 flags = GetFlags(triggers);
				ptr<Watcher> watcher(new Watcher(path, recursive, flags));
				while(commandNode != null)
				{
					watcher->commands.push_back(Command());
					vector<string> execs;
					xml_node<> *exec = commandNode->first_node("exec");
					if(exec == null)
					{
						throw err_bad_input;
					}
					while(exec != null)
					{
						xml_attribute<> *asyncAttr = exec->first_attribute("async");
						bool async = asyncAttr != null && icmp(string(asyncAttr->value(), asyncAttr->value_size()), "true") == 0;
						watcher->commands.back().execs.push_back(Exec(TStringFromString(string(exec->value(), exec->value_size())), async));
						exec = exec->next_sibling();
					}
					commandNode = commandNode->next_sibling();
				}
				watchers.push_back(watcher.release());
				watch = watch->next_sibling();
			}
		}
		catch(int e)
		{
			err = e;
		}
		catch(rapidxml::parse_error)
		{
			err = err_bad_input;
		}
		return err;
	}

	//////////////////////////////////////////////////////////////////////

	bool StartWatching()
	{
		tprintf($("Waiting for activity on %d folders\n"), Count());
		int n = 0;
		for (auto &w : watchers)
		{
			if (w->Watch())
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
			error($("Error waiting for folder changes: %s (continuing to wait...)\n"), GetLastErrorText().c_str());
		}
		else
		{
			Watcher const *w = watchers[hit - WAIT_OBJECT_0];
			tprintf($("Change detected in folder %s\n"), w->folder.c_str());
			w->Exec();
		}
	}

	//////////////////////////////////////////////////////////////////////

	~WatcherList()
	{
		for(auto w : watchers)
		{
			delete w;
		}
		watchers.clear();
		handles.clear();
	}

	//////////////////////////////////////////////////////////////////////

	int Count()
	{
		return (int)watchers.size();
	}
};

