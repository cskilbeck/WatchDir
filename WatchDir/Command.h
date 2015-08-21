//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Command
{
	struct Exec
	{
		tstring command;
		bool async;

		void Execute()
		{
		}
	};

	bool async;
	vector<Exec> commands;

	Command()
		: async(false)
	{
	}

	void Wait()
	{
	}


};
