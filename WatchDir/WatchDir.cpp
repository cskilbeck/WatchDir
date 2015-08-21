//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////

int _tmain(int argc, tchar *argv[])
{
	WatcherList watchers;

	int e = CheckArgs(argc, argv);
	if(e != success)
	{
		return e;
	}

	// parse args

	if (watchers.ReadInput(input_filename.c_str()))
	{
		if (watchers.StartWatching())
		{
			while (MsgWaitForMultipleObjectsEx(0, NULL, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE) == WAIT_IO_COMPLETION)
			{
			}
		}
	}
	return 0;
}
