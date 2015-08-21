//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////
// types

using uint = unsigned int;
using uint8 = unsigned __int8;
using uint16 = unsigned __int16;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;
using int8 = __int8;
using int16 = __int16;
using int32 = __int32;
using int64 = __int64;
using float32 = float;
using float64 = double;
using byte = uint8;

//////////////////////////////////////////////////////////////////////
// conveniences

using wchar = __wchar_t;
using tchar = TCHAR;
using std::wstring;
using std::string;
using tstring = std::basic_string<tchar>;

template<typename T> using list = std::list<T>;
template<typename T> using vector = std::vector<T>;
template<typename T> using ptr = std::unique_ptr<T>;

const nullptr_t null = nullptr;

static int(&tprintf)(const tchar *, ...) = _tprintf;
static int(&ftprintf)(FILE *, const tchar *, ...) = _ftprintf;

#ifdef UNICODE
static auto &tcin = std::wcin;
static auto &tcout = std::wcout;
static auto &tcerr = std::wcerr;
#else
static auto &tcin = std::cin;
static auto &tcout = std::cout;
static auto &tcerr = std::cerr;
#endif

// Ahem
#define $(X) TEXT(X)
