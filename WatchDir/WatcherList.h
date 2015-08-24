//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct WatcherList
{
	//////////////////////////////////////////////////////////////////////

	struct Event
	{
		Watcher *mWatcher;
		DWORD mAction;		// FILE_NOTIFY_XXX etc
		// how to handle rename?
	};

	//////////////////////////////////////////////////////////////////////

	vector<HANDLE> mHandles;
	vector<Watcher *> mWatchers;
	list<Watcher *> mEvents;

	//////////////////////////////////////////////////////////////////////

	WatcherList()
	{
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
			xml_node<> *root = doc.first_node("watcher");
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
				mWatchers.push_back(new Watcher(watch));
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
		for (auto &w : mWatchers)
		{
			if (w->Watch())
			{
				++n;
			}
		}
		return n > 0;
	}

	//////////////////////////////////////////////////////////////////////

	~WatcherList()
	{
		for(auto w : mWatchers)
		{
			delete w;
		}
		mWatchers.clear();
		mHandles.clear();
	}

	//////////////////////////////////////////////////////////////////////

	int Count()
	{
		return (int)mWatchers.size();
	}
};

