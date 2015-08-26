//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Timer
{
	//////////////////////////////////////////////////////////////////////

	HANDLE mTimerHandle;
	HANDLE mThreadHandle;
	HANDLE mThreadBegunEvent;
	DWORD mThreadID;

	//////////////////////////////////////////////////////////////////////

	Timer()
	{
		mThreadBegunEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		mThreadHandle = CreateThread(NULL, 0, ThreadProc, (LPVOID)this, 0, &mThreadID);
		WaitForSingleObject(mThreadBegunEvent, INFINITE);
		CloseHandle(mThreadBegunEvent);
	}

	//////////////////////////////////////////////////////////////////////

	static DWORD WINAPI ThreadProc(LPVOID param)
	{
		return ((Timer *)param)->Wait();
	}

	//////////////////////////////////////////////////////////////////////
	// Just sit and wait for APCs to fire

	DWORD Wait()
	{
		mTimerHandle = CreateWaitableTimer(NULL, TRUE, NULL);
		SetEvent(mThreadBegunEvent);
		while (true)
		{
			SleepEx(INFINITE, TRUE);
		}
		return 0;
	}

	//////////////////////////////////////////////////////////////////////

	static void CALLBACK TimerFireProc(LPVOID param, DWORD, DWORD)
	{
		((Timer *)param)->OnTimer();
	}

	//////////////////////////////////////////////////////////////////////

	virtual void OnTimer()
	{
	}

	//////////////////////////////////////////////////////////////////////

	struct Signal
	{
		Timer *timer;
		LARGE_INTEGER timeout;
	};

	//////////////////////////////////////////////////////////////////////

	static void WINAPI SignalFunc(ULONG_PTR param)
	{
		Signal *signal = (Signal *)param;
		SetWaitableTimer(signal->timer->mTimerHandle, &signal->timeout, 0, Timer::TimerFireProc, (LPVOID)signal->timer, FALSE);
		delete signal;
	}

	//////////////////////////////////////////////////////////////////////

	void SetDelay(float64 seconds)
	{
		Signal *signal = new Signal();
		signal->timer = this;
		signal->timeout.QuadPart = (int64)(seconds * -10000000.0);
		QueueUserAPC(SignalFunc, mThreadHandle, (ULONG_PTR)signal);
	}
};
