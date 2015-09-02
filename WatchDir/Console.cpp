//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <io.h>

//////////////////////////////////////////////////////////////////////

static int bitswap(uint i, uint j, uint b)
{
	uint x = ((b >> i) ^ (b >> j)) & 1;
	return b ^ ((x << i) | (x << j));
}

// Begin		*
// CSI1			27
// CSI2			'['
// Digits		'0'-'9'
// Semicolon	';'
// Terminate	'm'

//////////////////////////////////////////////////////////////////////
// if FILE *f is a console, ansi escape sequences for coloring are interpreted, otherwise they're stripped

void ConsoleFWrite(FILE *f, tchar const *str)
{
	static WORD attr = 0x000f;	// defaults to bold on because dim looks terrible

	int fileNum = _fileno(f);
	bool isTTY = _isatty(fileNum) != 0;
	HANDLE hFile = (HANDLE)_get_osfhandle(fileNum);
	
	tchar const *s = str;
	vector<int> parameters;
	parameters.reserve(16);
	bool gotParam = false;
	bool gotAttr = false;
	while (*str)
	{
		tchar c = *str++;
		if (c == 27 && *str == '[')
		{
			if ((str - s) > 1)
			{
				if (isTTY)
				{
					if (gotAttr)
					{
						SetConsoleTextAttribute(hFile, attr);
						gotAttr = false;
					}
					DWORD wrote;
					WriteConsole(hFile, s, (DWORD)(str - s - 1), &wrote, NULL);
				}
				else
				{
					fwrite(s, sizeof(tchar), (DWORD)(str - s - 1), f);
				}
				s = str;
			}
			int n = 0;
			++str;
			parameters.resize(0);
			while (*str)
			{
				tchar c = *str++;
				if (c == ';')
				{
					if (gotParam)
					{
						parameters.push_back(n);
						gotParam = false;
					}
					n = 0;
				}
				else if (c == 'm')
				{
					if (gotParam)
					{
						parameters.push_back(n);
					}
					for (auto i : parameters)
					{
						if (i == 0)
						{
							attr = 0x000f;
						}
						else if (i == 1)	// bold (intensity)
						{
							attr |= 0x0008;
						}
						else if (i == 2)	// 2 means dim it (bold is on by default)
						{
							attr &= ~0x0008;
						}
						else if (i == 3)
						{
							attr |= 0x0080;
						}
						else if (i == 6)
						{
							attr &= ~0x0080;
						}
						else if (i >= 30 && i <= 37)
						{
							attr = (attr & 0xfff8) | bitswap(0, 2, i - 30);
						}
						else if (i >= 40 && i <= 47)
						{
							attr = (attr & 0xff8f) | (bitswap(0, 2, i - 40) << 4);
						}
					}
					gotAttr = true;
					break;
				}
				else if (c >= '0' && c <= '9')
				{
					n = n * 10 + c - '0';
					gotParam = true;
				}
// 				else if (c == ' ')		// spaces not supported
// 				{
// 				}
				else
				{
					break;	// unrecognised - terminate parameter sequence and ignore
				}
			}
			s = str;
		}
	}
	if (isTTY && gotAttr)
	{
		SetConsoleTextAttribute(hFile, attr);
	}
	if (str != s)
	{
		if(isTTY)
		{
			DWORD wrote;
			WriteConsole(hFile, s, (DWORD)(str - s), &wrote, NULL);
		}
		else
		{
			fwrite(s, sizeof(tchar), (DWORD)(str - s), f);
		}
	}
}

//////////////////////////////////////////////////////////////////////

void ConsoleWrite(tchar const *str)
{
	ConsoleFWrite(stdout, str);
}

//////////////////////////////////////////////////////////////////////

void ConsoleFPrintf(tchar const *fmt, ...)
{
	va_list v;
	va_start(v, fmt);
	ConsoleWrite(Format_V(fmt, v).c_str());
}

//////////////////////////////////////////////////////////////////////

void ConsoleFPrintf(tstring const &fmt, ...)
{
	va_list v;
	va_start(v, fmt);
	ConsoleWrite(Format_V(fmt.c_str(), v).c_str());
}
