//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////

int _tmain(int argc, tchar *argv[])
{
	WatcherList watchers;

	ConsoleWrite($("\x1b[;31m RED \x1b[32m GREEN \x1b[33m YELLOW \x1b[34m BLUE \x1b[35m MAGENTA \x1b[36m CYAN \x1b[37m WHITE \n"));
	ConsoleWrite($("\x1b[6;41m RED \x1b[42m GREEN \x1b[43m YELLOW \x1b[44m BLUE \x1b[45m MAGENTA \x1b[46m CYAN \x1b[47m WHITE \n"));

	ConsoleWrite(ANSI_START ANSI_FG_RED ANSI_BG_DIM ANSI_BG_BLUE ANSI_END $("Hello\n\n"));
	ConsoleWrite(ANSI_RESET);

	int e = CheckArgs(argc, argv);
	if(e != success)
	{
		return e;
	}

	if (watchers.ReadInput(input_filename.c_str()) == success)
	{
		if (watchers.StartWatching())
		{
			while (MsgWaitForMultipleObjectsEx(0, NULL, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE) == WAIT_IO_COMPLETION)
			{
			}
		}
	}

#ifdef _DEBUG
	if(IsDebuggerPresent())
	{
		getchar();
	}
#endif

	return 0;
}
