//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Processor
{
	struct Filename
	{
		string name;
		string oldname;
	};

	struct Command
	{
		string command;
		vector<Filename> filenames;

		void Execute()
		{
		}
	};

	DWORD threadID;
	HANDLE thread;

	Processor()
		: threadID(0)
		, thread(INVALID_HANDLE_VALUE)
	{
	}

	void Wait()
	{
		HANDLE thread = CreateThread(nullptr, 0, &ThreadProc, (LPVOID)this, 0, &threadID);
	}

	void Run()
	{
		MSG msg;
		while (GetMessage(&msg, (HWND)-1, 0, 0) > 0)
		{
			// execute command
			// terminate (WM_QUIT)
			switch (msg.message)
			{
				case WM_USER:
					((Command *)msg.lParam)->Execute();
					break;
			}
		}
		threadID = 0;
		thread = INVALID_HANDLE_VALUE;
	}

	static inline DWORD WINAPI ThreadProc(LPVOID parameter)
	{
		((Processor *)parameter)->Run();
	}
};
