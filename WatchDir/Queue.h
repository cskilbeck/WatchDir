//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

template <class K, class A = std::allocator<K>> struct blocking_queue
{
private:
	using lock = std::unique_lock<std::mutex>;

	std::list<K, A> mList;
	std::mutex mMutex;
	std::condition_variable mSignal;

	blocking_queue(blocking_queue const &&) {}
	blocking_queue(blocking_queue const &) {}

public:

	blocking_queue()
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

//////////////////////////////////////////////////////////////////////

template <class K, class A = std::allocator<K>> struct safe_queue
{
private:
	using lock = std::lock_guard<std::mutex>;

	std::list<K, A> mList;
	std::mutex mMutex;

	safe_queue(safe_queue const &&) {}
	safe_queue(safe_queue const &) {}

public:

	safe_queue()
	{
	}

	void add(K value)
	{
		lock lk(mMutex);
		mList.push_back(value);
	}

	K remove()
	{
		lock lk(mMutex);
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
