//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////

int wmain(int argc, wchar *argv[])
{
	WatcherList watchers;

	if (watchers.ReadInput())
	{
		if (watchers.StartWatching())
		{
			while (MsgWaitForMultipleObjectsEx(0, NULL, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE) == WAIT_IO_COMPLETION)
			{
				wprintf(L"APC was called\n");
			}
		}
	}
	return 0;
}
