//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

WatcherList watchers;

int main()
{
	if (watchers.ReadInput())
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
