//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

static inline string Format_V(char const *fmt, va_list v)
{
	char buffer[512];
	int l = _vsnprintf_s(buffer, _countof(buffer), _TRUNCATE, fmt, v);
	if(l != -1)
	{
		return string(buffer);
	}
	l = _vscprintf(fmt, v);
	ptr<char> buf(new char[l + 1]);
	l = _vsnprintf_s(buf.get(), l + 1, _TRUNCATE, fmt, v);
	return string(buf.get(), buf.get() + l);
}

//////////////////////////////////////////////////////////////////////

static inline wstring Format_V(wchar const *fmt, va_list v)
{
	wchar buffer[512];
	int l = _vsnwprintf_s(buffer, _countof(buffer), _TRUNCATE, fmt, v);
	if(l != -1)
	{
		return wstring(buffer);
	}
	l = _vscwprintf(fmt, v);
	ptr<wchar> buf(new wchar[l + 1]);
	l = _vsnwprintf_s(buf.get(), l + 1, _TRUNCATE, fmt, v);
	return wstring(buf.get(), buf.get() + l);
}

//////////////////////////////////////////////////////////////////////

static inline wstring Format(wchar const *fmt, ...)
{
	va_list v;
	va_start(v, fmt);
	return Format_V(fmt, v);
}

//////////////////////////////////////////////////////////////////////

static inline string Format(char const *fmt, ...)
{
	va_list v;
	va_start(v, fmt);
	return Format_V(fmt, v);
}

//////////////////////////////////////////////////////////////////////

template <typename T> T const *FindFirstOf(T const *str, T const *find, size_t numDelimiters)
{
	assert(str != null);
	assert(find != null);

	T ch;
	while((ch = *str))
	{
		for(size_t i = 0; i < numDelimiters; ++i)
		{
			if(ch == find[i])
			{
				return str;
			}
		}
		++str;
	}
	return null;
}

//////////////////////////////////////////////////////////////////////

template <typename T> T const *FindFirstOf(T const *str, T const *find)
{
	return FindFirstOf(str, find, Length(find));
}

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

	size_t delimCount = _tcslen(delimiters);
	size_t len = _tcslen(str);	// LAME
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

template<typename chr> std::basic_string<chr> ltrim(std::basic_string<chr> const &s)
{
	auto b = std::find_if_not(s.begin(), s.end(), std::isspace);
	return s.substr(b - s.begin());
}

//////////////////////////////////////////////////////////////////////

template<typename chr> std::basic_string<chr> rtrim(std::basic_string<chr> const &s)
{
	auto e = std::find_if_not(s.rbegin(), s.rend(), std::isspace);
	return s.substr(0, e.base() - s.begin());
}

//////////////////////////////////////////////////////////////////////

template<typename chr> std::basic_string<chr> trim(std::basic_string<chr> const &s)
{
	auto b = std::find_if_not(s.begin(), s.end(), std::isspace);
	auto e = std::find_if_not(s.rbegin(), s.rend(), std::isspace);
	return s.substr(b - s.begin(), e.base() - b);
}

//////////////////////////////////////////////////////////////////////

static inline tstring GetLastErrorText(DWORD err = 0)
{
	LPTSTR lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		err ? err : GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	tstring r(lpMsgBuf);
	LocalFree(lpMsgBuf);
	return r;
}

//////////////////////////////////////////////////////////////////////

static inline wstring WideStringFromString(string const &str)
{
	vector<wchar> temp = { 0 };
	int bufSize = MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), temp.data(), (int)str.size());
	if(bufSize > 0)
	{
		temp.resize(bufSize + 1);
		temp[bufSize] = (wchar)0;
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), temp.data(), bufSize);
		temp[bufSize] = 0;
	}
	return wstring(temp.data());
}

//////////////////////////////////////////////////////////////////////

static inline string StringFromWideString(wstring const &str)
{
	vector<char> temp;
	int bufSize = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.size() + 1, NULL, 0, NULL, FALSE);
	if(bufSize > 0)
	{
		temp.resize(bufSize);
		WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.size() + 1, &temp[0], bufSize, NULL, FALSE);
		return string(temp.data());
	}
	return string();
}

//////////////////////////////////////////////////////////////////////

static inline tstring TStringFromString(string const &str)
{
#ifdef UNICODE
	return WideStringFromString(str);
#else
	return str;
#endif
}

//////////////////////////////////////////////////////////////////////

static inline string StringFromTString(tstring const &str)
{
#ifdef UNICODE
	return StringFromWideString(str);
#else
	return str;
#endif
}

//////////////////////////////////////////////////////////////////////

static inline wstring WStringFromTString(tstring const &str)
{
#ifdef UNICODE
	return str;
#else
	return WideStringFromString(str);
#endif
}

//////////////////////////////////////////////////////////////////////

static inline tstring TStringFromWString(wstring const &str)
{
#ifdef UNICODE
	return str;
#else
	return StringFromWideString(str);
#endif
}

//////////////////////////////////////////////////////////////////////

static inline int icmp(wstring const &a, wstring const &b)
{
	return _wcsicmp(a.c_str(), b.c_str());
}

//////////////////////////////////////////////////////////////////////

static inline int icmp(wstring const &a, string const &b)
{
	return icmp(a, WideStringFromString(b));
}

//////////////////////////////////////////////////////////////////////

static inline int icmp(string const &a, wstring const &b)
{
	return icmp(WideStringFromString(a), b);
}

//////////////////////////////////////////////////////////////////////

static inline int icmp(string const &a, string const &b)
{
	return _stricmp(a.c_str(), b.c_str());
}

//////////////////////////////////////////////////////////////////////

static inline int icmp(wchar const *a, wchar const *b)
{
	return _wcsicmp(a, b);
}

//////////////////////////////////////////////////////////////////////

static inline int icmp(wchar const *a, char const *b)
{
	return icmp(wstring(a), WideStringFromString(b));
}

//////////////////////////////////////////////////////////////////////

static inline int icmp(char const *a, wchar const *b)
{
	return icmp(WideStringFromString(a), b);
}

//////////////////////////////////////////////////////////////////////

static inline int icmp(char const *a, char const *b)
{
	return _stricmp(a, b);
}

//////////////////////////////////////////////////////////////////////

static inline void error(tchar const *fmt, ...)
{
	va_list v;
	va_start(v, fmt);
	_vftprintf_s(stderr, fmt, v);
}

//////////////////////////////////////////////////////////////////////

static inline bool FileOrFolderExists(tchar const *filename)
{
	return GetFileAttributes(filename) != INVALID_FILE_ATTRIBUTES;
}

//////////////////////////////////////////////////////////////////////

static inline bool FileExists(tchar const *filename)
{
	DWORD dwAttrib = GetFileAttributes(filename);
	return dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

//////////////////////////////////////////////////////////////////////

static inline bool FolderExists(tchar const *foldername)
{
	DWORD dwAttrib = GetFileAttributes(foldername);
	return dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

//////////////////////////////////////////////////////////////////////

static inline int CreateFolder(tchar const *name)
{
	int r = SHCreateDirectory(null, WStringFromTString(name).c_str());
	return (r == ERROR_SUCCESS ||
			r == ERROR_ALREADY_EXISTS ||
			r == ERROR_FILE_EXISTS)
		? S_OK : HRESULT_FROM_WIN32(r);
}

//////////////////////////////////////////////////////////////////////

static inline int LoadFile(tchar const *filename, ptr<byte> &buffer)
{
	int err = success;
	FILE *f = null;
	try
	{
		int e = _tfopen_s(&f, filename, "rb");
		if(e != 0)
		{
			throw err_file_not_found;
		}
		if(fseek(f, 0, SEEK_END) != 0)
		{
			throw err_file_err;
		}
		long size = ftell(f);
		if(size == -1)
		{
			throw err_file_err;
		}
		if(fseek(f, 0, SEEK_SET) != 0)
		{
			throw err_file_err;
		}
		buffer.reset(new byte[size + 1]);
		if(fread(buffer.get(), sizeof(byte), (size_t)size, f) != size)
		{
			throw err_file_err;
		}
		byte *p = buffer.get();
		p[size] = 0;
	}
	catch(int e)
	{
		err = e;
	}
	if(f != NULL)
	{
		fclose(f);
	}
	return err;
}
