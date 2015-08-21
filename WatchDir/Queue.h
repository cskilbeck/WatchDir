//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

template <class K, class A = std::allocator<K>> struct thread_safe_queue
{
private:
	using lock = std::unique_lock<std::mutex>;

	std::list<K, A> mList;
	std::mutex mMutex;
	std::condition_variable mSignal;

	// no copying due to the mutex
	thread_safe_queue(thread_safe_queue const &&) {}
	thread_safe_queue(thread_safe_queue const &) {}

public:

	thread_safe_queue()
	{
	}

	void add(K value)
	{
		lock lk(mMutex);
		mList.push_back(value);
		mSignal.notify_all();
	}

	K remove()
	{
		lock lk(mMutex);
		while(mList.empty())
		{
			mSignal.wait(lk);
		}
		K v = mList.front();
		mList.pop_front();
		return v;
	}

	bool empty()
	{
		lock lk(mMutex);
		return mList.empty();
	}
};