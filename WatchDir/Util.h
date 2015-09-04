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

template <typename T> size_t Length(T const *str)
{
	assert(str != null);

	T const *p = str;
	while(*p++)
	{
	}
	return p - str;
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

static inline wstring WString(string const &str)
{
	vector<wchar> temp;
	int bufSize = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, null, 0);
	if(bufSize > 0)
	{
		temp.resize(bufSize);
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, temp.data(), bufSize);
		return wstring(temp.data());
	}
	return wstring();
}

//////////////////////////////////////////////////////////////////////

static inline string String(wstring const &str)
{
	vector<char> temp;
	int bufSize = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, NULL, 0, NULL, FALSE);
	if(bufSize > 0)
	{
		temp.resize(bufSize);
		WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, temp.data(), bufSize, NULL, FALSE);
		return string(temp.data());
	}
	return string();
}

//////////////////////////////////////////////////////////////////////

static inline string String(string const &str)
{
	return str;
}

//////////////////////////////////////////////////////////////////////

static inline wstring WString(wstring const &str)
{
	return str;
}

//////////////////////////////////////////////////////////////////////

static inline tstring TString(string const &str)
{
#ifdef UNICODE
	return WString(str);
#else
	return str;
#endif
}

//////////////////////////////////////////////////////////////////////

static inline tstring TString(wstring const &str)
{
#ifdef UNICODE
	return str;
#else
	return String(str);
#endif
}

//////////////////////////////////////////////////////////////////////

static inline int icmp(wstring const &a, wstring const &b)
{
	return _wcsicmp(a.c_str(), b.c_str());
}

//////////////////////////////////////////////////////////////////////

static inline int icmp(wchar const *a, wchar const *b)
{
	return _wcsicmp(a, b);
}

//////////////////////////////////////////////////////////////////////

static inline int icmp(string const &a, string const &b)
{
	return _stricmp(a.c_str(), b.c_str());
}

//////////////////////////////////////////////////////////////////////

static inline int icmp(char const *a, char const *b)
{
	return _stricmp(a, b);
}

//////////////////////////////////////////////////////////////////////

static inline int icmp(wstring const &a, string const &b)
{
	return icmp(a, WString(b));
}

//////////////////////////////////////////////////////////////////////

static inline int icmp(string const &a, wstring const &b)
{
	return icmp(WString(a), b);
}

//////////////////////////////////////////////////////////////////////

static inline int icmp(wchar const *a, char const *b)
{
	return icmp(wstring(a), WString(b));
}

//////////////////////////////////////////////////////////////////////

static inline int icmp(char const *a, wchar const *b)
{
	return icmp(WString(a), b);
}

//////////////////////////////////////////////////////////////////////

static inline void error(tchar const *fmt, ...)
{
	va_list v;
	va_start(v, fmt);
	_vftprintf_s(stderr, fmt, v);
}

//////////////////////////////////////////////////////////////////////

enum FileOrFolder
{
	Error = -1,
	File = 1,
	Folder = 2
};

static inline FileOrFolder IsFileOrFolder(tchar const *filename)
{
	DWORD dwAttrib = GetFileAttributes(filename);
	if (dwAttrib == INVALID_FILE_ATTRIBUTES)
	{
		return Error;
	}
	if ((dwAttrib & FILE_ATTRIBUTE_DIRECTORY) != 0)
	{
		return Folder;
	}
	return File;
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
	int r = SHCreateDirectory(null, WString(name).c_str());
	return (r == ERROR_SUCCESS ||
			r == ERROR_ALREADY_EXISTS ||
			r == ERROR_FILE_EXISTS)
		? S_OK : HRESULT_FROM_WIN32(r);
}

//////////////////////////////////////////////////////////////////////

static inline int LoadFile(tchar const *filename, ptr<byte> &buffer)
{
	int err = success;

	Handle hFile = CreateFile(filename,    // file to open
					GENERIC_READ,          // open for reading
					FILE_SHARE_READ,       // share for reading
					NULL,                  // default security
					OPEN_EXISTING,         // existing file only
					FILE_ATTRIBUTE_NORMAL, // normal file
					NULL);                 // no attr. template

	if (!hFile.IsValid())
	{
		return err_file_not_found;
	}

	LARGE_INTEGER fileSize;
	if (!GetFileSizeEx(hFile, &fileSize) || fileSize.QuadPart > 0xffffffff)
	{
		return err_file_err;
	}

	ptr<byte> data(new byte[fileSize.QuadPart + 2]);
	DWORD got;
	if (!ReadFile(hFile, data.get(), (DWORD)fileSize.QuadPart, &got, NULL))
	{
		return err_file_err;
	}

	data.get()[fileSize.QuadPart] = 0;		// 2 null bytes in case of wide chars
	data.get()[fileSize.QuadPart + 1] = 0;

	buffer.reset(data.release());

	return err;
}

//////////////////////////////////////////////////////////////////////

static inline tstring ExpandEnvironment(tstring const &src)
{
	DWORD req = ExpandEnvironmentStrings(src.c_str(), null, 0);
	if(req != 0)
	{
		vector<tchar> buffer(req);
		DWORD act = ExpandEnvironmentStrings(src.c_str(), buffer.data(), req);
		if (act != 0)
		{
			buffer[buffer.size() - 1] = 0;
			return buffer.data();
		}
	}
	error($("Can't expand environment strings in %s (%08x)\n"), src, GetLastError());
	return tstring();
}

//////////////////////////////////////////////////////////////////////

static inline tstring Replace(tstring str, tstring const &findStr, tstring const &replaceStr)
{
	size_t pos = 0;
	while(true)
	{
		pos = str.find(findStr, pos);
		if (pos == std::string::npos)
		{
			break;
		}
		str.replace(pos, findStr.length(), replaceStr);
		pos += replaceStr.length();
	}
	return str;
}

//////////////////////////////////////////////////////////////////////

static inline int GetPathDepth(tstring const &path)
{
	int n = 0;
	tchar const *p = path.c_str();
	for (size_t i = 0, l = path.length() - 1; i < l; ++i)
	{
		if (p[i] == '\\')
		{
			++n;
		}
	}
	return n;
}

//////////////////////////////////////////////////////////////////////

static inline bool ParseDouble(tchar const *d, double &v)
{
	tchar *e;
	errno = 0;
	double x = _tcstod(d, &e);
	if (errno != 0)
	{
		return false;
	}
	v = x;
	return true;
}

//////////////////////////////////////////////////////////////////////

enum ValueRequired
{
	Optional,
	Required
};

static inline void xmlGetDouble(xml_base<> *node, double &val, ValueRequired required, tchar const *name)
{
	if(node != null)
	{
		if(!ParseDouble(TString(node->val()).c_str(), val))
		{
			error($("Error, invalid value for %s (%s)\n"), name, node->val().c_str());
			throw err_bad_input;
		}
	}
	else if(required == Required)
	{
		error($("Missing value for %s\n"), name);
		throw err_bad_input;
	}
}

static inline void xmlGetDoubleNode(xml_node<> *node, double &val, ValueRequired required, tchar const *name)
{
	xmlGetDouble(node->first_node(name), val, required, name);
}

static inline void xmlGetDoubleAttr(xml_node<> *node, double &val, ValueRequired required, tchar const *name)
{
	xmlGetDouble(node->first_attribute(name), val, required, name);
}

//////////////////////////////////////////////////////////////////////

static inline void xmlGetBool(xml_base<> *node, bool &val, ValueRequired required, tchar const *name)
{
	if (node != null)
	{
		bool isTrue = icmp(node->val(), "true") == 0;
		bool isFalse = icmp(node->val(), "false") == 0;
		if (!(isTrue || isFalse))
		{
			error($("Error, invalid value for %s (%s)\n"), name, node->val().c_str());
			throw err_bad_input;
		}
		else
		{
			val = isTrue;
		}
	}
	else if (required == Required)
	{
		error($("Missing value for %s\n"), name);
		throw err_bad_input;
	}
}

static inline void xmlGetBoolNode(xml_node<> *node, bool &val, ValueRequired required, tchar const *name)
{
	xmlGetBool(node->first_node(name), val, required, name);
}

static inline void xmlGetBoolAttr(xml_node<> *node, bool &val, ValueRequired required, tchar const *name)
{
	xmlGetBool(node->first_attribute(name), val, required, name);
}

//////////////////////////////////////////////////////////////////////

static inline tstring GetFolder(tstring const &filename)
{
	tchar *name;
	tchar buffer[4096];
	DWORD len = GetFullPathName(filename.c_str(), _countof(buffer) - 1, buffer, &name);
	if (len == 0)
	{
		return $("");
	}
	return tstring(buffer, name - buffer);
}

//////////////////////////////////////////////////////////////////////

static inline tstring GetCanonicalPath(tstring const &folder)
{
	wstring fldr = WString(folder);
	ptr<wchar> path(new wchar[32768]);
	ptr<wchar> canonicalPath(new wchar[32768]);
	memcpy(path.get(), fldr.c_str(), sizeof(wchar) * (fldr.size() + 1));
	if (FAILED(PathCchCanonicalizeEx(canonicalPath.get(), 32768, path.get(), PATHCCH_ALLOW_LONG_PATHS)))
	{
		throw err_bad_input;
	}
	return TString(canonicalPath.get());
}

//////////////////////////////////////////////////////////////////////

static inline tstring GetRelativePath(tstring const &folder, tstring const &file)
{
	tchar out[MAX_PATH];
	return GetCanonicalPath(PathRelativePathTo(out, file.c_str(), GetFileAttributes(file.c_str()), folder.c_str(), GetFileAttributes(folder.c_str())) ? out : $("./"));
}

//////////////////////////////////////////////////////////////////////

template<typename T, typename U> inline tstring join(T start, T end, U const &separator)
{
	tstring r;
	while(start != end)
	{
		r += *start++;
		if(start != end)
		{
			r += separator;
		}
	}
	return r;
}

//////////////////////////////////////////////////////////////////////

template<typename T, typename U> inline tstring join(T const &container, U const &separator)
{
	return join(container.begin(), container.end(), separator);
}
