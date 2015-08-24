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
		tprintf($("Spawning %s\n"), mCommand.c_str());
		BOOL b = CreateProcess(NULL, cmd.get(), NULL, NULL, TRUE, 0, NULL, folder.c_str(), &si, &pi);
		if(b == NULL)
		{
			error($("Error creating process %s, Error: %s\n"), mCommand.c_str(), GetLastErrorText().c_str());
		}
		else if(!mAsync)
		{
			WaitForSingleObject(pi.hProcess, INFINITE);
			tprintf($("%s complete\n"), mCommand.c_str());
		}
	}
};

//////////////////////////////////////////////////////////////////////

struct Command
{
	bool mAsync;
	vector<Exec> mExecs;

	//////////////////////////////////////////////////////////////////////

	Command(xml_node<> *commandNode)
	{
		xml_attribute<> *asyncAttr = commandNode->first_attribute("async");
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
			e.Execute(folder, fileEvent);
		}
		return 0;
	}
};
