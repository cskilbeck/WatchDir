//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

template <class container_t, class string_t, class char_t>
void tokenize(string_t const &str, container_t &tokens, char_t const *delimiters, bool includeEmpty = true)
{
	string_t::size_type end = 0, start = 0, len = str.size();
	while (end < len)
	{
		end = str.find_first_of(delimiters, start);
		if (end == string_t::npos)
		{
			end = len;
		}
		if (end != start || includeEmpty)
		{
			tokens.push_back(container_t::value_type(str.data() + start, end - start));
		}
		start = end + 1;
	}
}

//////////////////////////////////////////////////////////////////////

template <class container_t, class char_t>
void tokenize(char_t const *str, container_t &tokens, char_t const *delimiters, bool includeEmpty = true)
{
	assert(str != null);
	assert(delimiters != null);

	size_t delimCount = Length(delimiters);
	size_t len = Length(str);	// LAME
	char_t const *start = str, *end = str, *stop = start + len;
	while (start < stop)
	{
		end = FindFirstOf(start, delimiters, delimCount);
		if (end == null)
		{
			end = stop;
		}
		if (end != start || includeEmpty)
		{
			tokens.push_back(container_t::value_type(start, end - start));
		}
		start = end + 1;
	}
}

//////////////////////////////////////////////////////////////////////

string ltrim(string const &s)
{
	auto b = std::find_if_not(s.begin(), s.end(), std::isspace);
	return s.substr(b - s.begin());
}

//////////////////////////////////////////////////////////////////////

static inline string rtrim(string const &s)
{
	auto e = std::find_if_not(s.rbegin(), s.rend(), std::isspace);
	return s.substr(0, e.base() - s.begin());
}

//////////////////////////////////////////////////////////////////////

static inline string trim(string const &s)
{
	auto b = std::find_if_not(s.begin(), s.end(), std::isspace);
	auto e = std::find_if_not(s.rbegin(), s.rend(), std::isspace);
	return s.substr(b - s.begin(), e.base() - b);
}

//////////////////////////////////////////////////////////////////////

static inline string GetLastErrorText(DWORD err = 0)
{
	LPTSTR lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		err ? err : GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	string r(lpMsgBuf);
	LocalFree(lpMsgBuf);
	return r;
}

//////////////////////////////////////////////////////////////////////

static inline void error(wchar const *fmt, ...)
{
	va_list v;
	va_start(v, fmt);
	vfwprintf(stderr, fmt, v);
}

//////////////////////////////////////////////////////////////////////

template <class K, class Allocator = std::allocator<K>> struct safe_queue
{
private:
	using lock = std::lock_guard<std::mutex>;

	std::list<K, Allocator> mList;
	std::mutex mMutex;

public:
	void push(K value)
	{
		lock lk(mMutex);
		mList.push_back(value);
	}

	K &pop()
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