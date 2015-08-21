//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Exec
{
	tstring command;
	bool async;

	Exec(tstring const &cmd, bool async_)
		: command(cmd)
		, async(async_)
	{
	}

	void Execute()
	{
		// CreateProcess
		// if !async, wait for it to complete
	}
};

struct Command
{
	bool async;
	vector<Exec> execs;

	Command()
		: async(false)
	{
	}

	int Execute()
	{
		// create thread to:
		// for each Exec in execs
		// exec.Execute();
		//
		// if async detach() else join()
	}
};
