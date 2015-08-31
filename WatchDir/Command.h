//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Exec
{
	tstring mCommand;
	bool mAsync;

	//////////////////////////////////////////////////////////////////////

	Exec(xml_node<> *exec)
		: mAsync(false)
	{
		mCommand = TString(exec->val());
		mCommand = Replace(mCommand, $("\r\n"), $(" "));
		mCommand = Replace(mCommand, $("\r"), $(" "));
		mCommand = Replace(mCommand, $("\n"), $(" "));
		mCommand = Replace(mCommand, $("\t"), $(" "));
		xmlGetBoolAttr(exec, mAsync, Optional, $("async"));
	}

	//////////////////////////////////////////////////////////////////////

	tstring ReplaceAllTokens(tstring const &command, FileEvent *event)
	{
		tstring cmd(command);
		cmd = Replace(cmd, $("%{drive}"), event->mDrive);
		cmd = Replace(cmd, $("%{dir}"), event->mDir);
		cmd = Replace(cmd, $("%{folder}"), event->mDir);
		cmd = Replace(cmd, $("%{name}"), event->mName);
		cmd = Replace(cmd, $("%{ext}"), event->mExt);
		cmd = Replace(cmd, $("%{olddrive}"), event->mOldDrive);
		cmd = Replace(cmd, $("%{olddir}"), event->mOldDir);
		cmd = Replace(cmd, $("%{oldfolder}"), event->mOldDir);
		cmd = Replace(cmd, $("%{oldname}"), event->mOldName);
		cmd = Replace(cmd, $("%{oldext}"), event->mOldExt);
		cmd = Replace(cmd, $("%{filepath}"), event->mFilePath);
		cmd = Replace(cmd, $("%{fullpath}"), event->mFilePath);
		cmd = Replace(cmd, $("%{oldfilepath}"), event->mFilePath);
		cmd = Replace(cmd, $("%{oldfullpath}"), event->mFilePath);
		cmd = Replace(cmd, $("%{filename}"), event->mName + event->mExt);
		cmd = Replace(cmd, $("%{oldfilename}"), event->mOldName + event->mOldExt);
		return cmd;
	}

	//////////////////////////////////////////////////////////////////////

	void Execute(tstring const &folder, FileEvent *fileEvent, vector<HANDLE> &handles)
	{
		using ch = tstring::value_type;
		tstring c = Format($("cmd /c %s"), ReplaceAllTokens(mCommand, fileEvent).c_str());
		ptr<ch> cmd(new ch[c.size() + 1]);
		memcpy(cmd.get(), c.c_str(), c.size() + 1);
		STARTUPINFO si = { 0 };
		PROCESS_INFORMATION pi = { 0 };
		si.cb = sizeof(si);
		tprintf($("\t\t%s..."), c.c_str());
		if(!CreateProcess(NULL, cmd.get(), NULL, NULL, TRUE, 0, NULL, folder.c_str(), &si, &pi))
 		{
 			error($("error: %s\n"), GetLastErrorText().c_str());
			return;
 		}
		else
		{
			handles.push_back(pi.hProcess);
			if(!mAsync)
			{
				WaitForSingleObject(pi.hProcess, INFINITE);
				tprintf($("complete\n"), mCommand.c_str());
			}
			else
			{
				tprintf($("spawned\n"));
			}
		}
	}
};

__declspec(selectany) Map<tstring, DWORD> filters =
{
	{ "add",	FileEvent::FA_ADDED },
	{ "create",	FileEvent::FA_ADDED },
	{ "remove",	FileEvent::FA_REMOVED },
	{ "delete",	FileEvent::FA_REMOVED },
	{ "modify",	FileEvent::FA_MODIFIED },
	{ "rename",	FileEvent::FA_RENAMED },
	{ "all",	FileEvent::FA_ALL }
};

//////////////////////////////////////////////////////////////////////

struct Command
{
	bool mAsync;
	DWORD mFilter;
	vector<Exec> mExecs;

	//////////////////////////////////////////////////////////////////////

	Command(xml_node<> *commandNode)
		: mAsync(false)
		, mFilter(0)
	{
		xml_attribute<> *onAttr = commandNode->first_attribute("triggers");
		if (onAttr != null)
		{
			vector<tstring> tokens;
			tokenize(onAttr->val().c_str(), tokens, $(" ,;"), false);
			for (auto const &token : tokens)
			{
				DWORD f;
				if (!filters.try_get(token, f))
				{
					error($("Unknown filter: %s\n"), token.c_str());
				}
				else
				{
					mFilter |= f;
				}
			}
		}
		xmlGetBoolAttr(commandNode, mAsync, Optional, $("async"));
		vector<string> execs;
		xml_node<> *execNode = commandNode->first_node("exec");
		if(execNode == null)
		{
			throw err_bad_input;
		}
		while(execNode != null)
		{
			mExecs.push_back(Exec(execNode));
			execNode = execNode->next_sibling();
		}
	}

	//////////////////////////////////////////////////////////////////////

	struct ThreadParams
	{
		Command *mCommand;
		tstring const &mFolder;
		FileEvent *mFileEvent;

		ThreadParams(Command *command, tstring const &folder, FileEvent *fileEvent)
			: mCommand(command)
			, mFolder(folder)
			, mFileEvent(fileEvent)
		{
		}

		DWORD Run()
		{
			vector<HANDLE> handles;
			for(auto &e : mCommand->mExecs)
			{
				// check filter
				e.Execute(mFolder, mFileEvent, handles);
			}
			if(!mCommand->mAsync)
			{
				WaitForMultipleObjects((DWORD)handles.size(), handles.data(), TRUE, INFINITE);
			}
			return 0;
		}
	};

	//////////////////////////////////////////////////////////////////////

	static DWORD WINAPI ExecuteThread(LPVOID param)
	{
		return ((ThreadParams *)param)->Run();
	}

	//////////////////////////////////////////////////////////////////////

	void Execute(tstring const &folder, FileEvent *fileEvent)
	{
		ThreadParams *t = new ThreadParams(this, folder, fileEvent);
		DWORD threadID;
		HANDLE thread = CreateThread(NULL, 0, &ExecuteThread, t, 0, &threadID);
		if(thread == INVALID_HANDLE_VALUE)
		{
			error($("Error creating thread: %s\n"), GetLastErrorText().c_str());
		}
		else if(!mAsync)
		{
			WaitForSingleObject(thread, INFINITE);
		}
	}
};
