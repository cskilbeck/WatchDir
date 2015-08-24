//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Exec
{
	tstring mCommand;
	bool mAsync;

	Exec(xml_node<> *exec)
	{
		mCommand = TString(exec->val());
		xml_attribute<> *asyncAttr = exec->first_attribute("async");
		mAsync = asyncAttr != null && icmp(asyncAttr->val(), "true") == 0;
	}

	void Execute()
	{
		tprintf($("EXEC: %s\n"), mCommand.c_str());
	}
};

//////////////////////////////////////////////////////////////////////

struct Command
{
	bool mAsync;
	vector<Exec> mExecs;

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

	int Execute()
	{
		// create thread to:
		// for each Exec in execs
		// exec.Execute();
		//
		// if async detach() else join()
		for(auto &e : mExecs)
		{
			e.Execute();
		}
	}
};
