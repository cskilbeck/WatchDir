//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

WatcherList watchers;

int main()
{
	if (watchers.ReadInput())
	{
		if (watchers.StartWatching())
		{
			Sleep(INFINITE);
		}
	}
	return 0;
}
