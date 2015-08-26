//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Exec
{
	tstring mCommand;
	bool mAsync;

	//////////////////////////////////////////////////////////////////////

	Exec(xml_node<> *exec)
	{
		mCommand = TString(exec->val());
		xml_attribute<> *asyncAttr = exec->first_attribute("async");
		mAsync = asyncAttr != null && icmp(asyncAttr->val(), "true") == 0;
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

	void Execute(tstring const &folder, FileEvent *fileEvent)
	{
		using ch = tstring::value_type;
		size_t size = mCommand.size();
		ptr<ch> cmd(new ch[size + 1]);
		memcpy(cmd.get(), mCommand.data(), size * sizeof(ch));
		cmd.get()[size] = 0;
		STARTUPINFO si = { 0 };
		PROCESS_INFORMATION pi = { 0 };
		si.cb = sizeof(si);
		tprintf($("\t\t%s\n"), ReplaceAllTokens(mCommand, fileEvent).c_str());
// 		BOOL b = CreateProcess(NULL, cmd.get(), NULL, NULL, TRUE, 0, NULL, folder.c_str(), &si, &pi);
// 		if(b == NULL)
// 		{
// 			error($("Error creating process %s, Error: %s\n"), mCommand.c_str(), GetLastErrorText().c_str());
// 		}
// 		else if(!mAsync)
// 		{
// 			WaitForSingleObject(pi.hProcess, INFINITE);
// 			tprintf($("%s complete\n"), mCommand.c_str());
// 		}
	}
};

__declspec(selectany) Map<tstring, DWORD> filters =
{
	{ "add",	FileEvent::FA_ADDED },
	{ "remove",	FileEvent::FA_REMOVED },
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
		, mFilter(0xff)
	{
		xml_attribute<> *asyncAttr = commandNode->first_attribute("async");
		xml_attribute<> *onAttr = commandNode->first_attribute("on");
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
		mAsync = asyncAttr != null && icmp(asyncAttr->val(), "true") == 0;
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

	int Execute(tstring const &folder, FileEvent *fileEvent)
	{
		for(auto &e : mExecs)
		{
			// check filter
			e.Execute(folder, fileEvent);
		}
		return 0;
	}
};
