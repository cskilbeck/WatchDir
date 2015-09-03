//////////////////////////////////////////////////////////////////////
// Push empty params with a flag saying they have no value

#include "stdafx.h"
#include <io.h>

//////////////////////////////////////////////////////////////////////

#if defined(_DEBUG)
static void Trace(tchar const *s, ...)
{
	va_list v;
	va_start(v, s);
	OutputDebugString(Format_V(s, v).c_str());
}
#else
#define Trace(...) do {} while(0)
#endif

//////////////////////////////////////////////////////////////////////

struct Console
{
	using uint = unsigned int;
	template <typename T> using vector = std::vector<T>;

	Console(FILE *f)
		: mFilePtr(f)
		, mAttr(0x000f)
	{
		mFileNum = _fileno(mFilePtr);
		mIsTTY = _isatty(mFileNum) != 0;
		mFileHandle = (HANDLE)_get_osfhandle(mFileNum);
		mParameters.reserve(16);
	}

	void AddParameter(int p)
	{
		mParameters.push_back(p);
	}

	void Write(void const *data, size_t amount)
	{
		if (mAttrChanged && mIsTTY)
		{
			SetConsoleTextAttribute(mFileHandle, mAttr);
			mAttrChanged = false;
		}
		if (amount != 0 && data != null)
		{
			if (mIsTTY)
			{
				DWORD wrote;
				WriteConsole(mFileHandle, data, (DWORD)amount, &wrote, NULL);
			}
			else
			{
				fwrite(data, sizeof(tchar), amount, mFilePtr);
			}
		}
	}

	bool ProcessCommand(int c)
	{
		auto f = ControlDefinitions.find(c);
		if (f == ControlDefinitions.end())
		{
			Trace($("Unknown command: %c\n"), c);
			return false;
		}
		auto func = f->second.mFunction;
		Trace($("Function: %c (%s), Parameters (%d) : "), c, f->second.mName, mParameters.size());
		for (auto p : mParameters)
		{
			Trace($("%d "), p);
		}
		
		if (f->second.mMinParameters > mParameters.size())
		{
			Trace($(" Too few parameters!\n"));
			return false;
		}
		if (f->second.mMaxParameters < mParameters.size())
		{
			Trace($(" Too many parameters!\n"));
			return false;
		}
		Trace($("\n"));
		(this->*func)();
		mParameters.clear();
		return true;
	}

private:

	static inline int bitswap(uint i, uint j, uint b)
	{
		uint x = ((b >> i) ^ (b >> j)) & 1;
		return b ^ ((x << i) | (x << j));
	}

	void ResetAttr()
	{
		mAttr = 0x000f;
		mAttrChanged = true;
	}

	void SetFGBold()
	{
		mAttr |= 0x0008;
		mAttrChanged = true;
	}

	void ClearFGBold()
	{
		mAttr &= ~0x0008;
		mAttrChanged = true;
	}

	void SetBGBold()
	{
		mAttr |= 0x0080;
		mAttrChanged = true;
	}

	void ClearBGBold()
	{
		mAttr &= ~0x0080;
		mAttrChanged = true;
	}

	void SetFGColor(int c)
	{
		c &= 7;
		mAttr = (mAttr & 0xfff8) | bitswap(0, 2, c);
		mAttrChanged = true;
	}

	void SetBGColor(int c)
	{
		c &= 7;
		mAttr = (mAttr & 0xff8f) | (bitswap(0, 2, c) << 4);
		mAttrChanged = true;
	}

	//////////////////////////////////////////////////////////////////////

	typedef bool (Console::*ControlFunction)();

	struct ControlDefinition
	{
		ControlFunction	mFunction;
		uint			mMinParameters;
		uint			mMaxParameters;
		tchar const *	mName;
	};

	static std::map<int, Console::ControlDefinition> Console::ControlDefinitions;

	bool MoveCursor(int x, int y)
	{
		CONSOLE_SCREEN_BUFFER_INFO info;
		GetConsoleScreenBufferInfo(mFileHandle, &info);
		SetConsoleCursorPosition(mFileHandle, COORD{
			max(info.srWindow.Top, min(info.srWindow.Bottom, y + info.dwCursorPosition.Y)),
			max(info.srWindow.Left, min(info.srWindow.Right, x + info.dwCursorPosition.X))
		});
		return true;

	}

	bool SetCursor(int x, int y)
	{
		CONSOLE_SCREEN_BUFFER_INFO info;
		GetConsoleScreenBufferInfo(mFileHandle, &info);
		SetConsoleCursorPosition(mFileHandle, COORD{
			max(info.srWindow.Top, min(info.srWindow.Bottom, y)),
			max(info.srWindow.Left, min(info.srWindow.Right, x))
		});
		return true;
	}

	COORD GetCursorPos()
	{
		CONSOLE_SCREEN_BUFFER_INFO info;
		GetConsoleScreenBufferInfo(mFileHandle, &info);
		return (COORD)info.dwCursorPosition;
	}

	bool ClearConsole()
	{
		CONSOLE_SCREEN_BUFFER_INFO info;
		if (GetConsoleScreenBufferInfo(mFileHandle, &info))
		{
			DWORD wrote;
			DWORD screenSize = (info.srWindow.Right - info.srWindow.Left) * (info.srWindow.Bottom - info.srWindow.Top);
			FillConsoleOutputCharacter(mFileHandle, ' ', screenSize, COORD{ 0, 0 }, &wrote);
			FillConsoleOutputAttribute(mFileHandle, info.wAttributes, screenSize, COORD{ 0, 0 }, &wrote);
			SetConsoleCursorPosition(mFileHandle, COORD{ 0, 0 });
			return true;
		}
		return false;
	}

	int GetParam(int n, int def)
	{
		return (mParameters.size() > n) ? mParameters[n] : def;
	}

	bool CursorUp()
	{
		return MoveCursor(0, -GetParam(0, 1));
	}
	bool CursorDown()
	{
		return MoveCursor(0, GetParam(0, 1));
	}
	bool CursorForward()
	{
		return MoveCursor(GetParam(0, 1), 0);
	}
	bool CursorBack()
	{
		return MoveCursor(-GetParam(0, 1), 0);
	}
	bool CursorNextLine()
	{
		return MoveCursor(std::numeric_limits<int>::lowest(), GetParam(0, 1));
	}
	bool CursorPrevLine()
	{
		return MoveCursor(std::numeric_limits<int>::lowest(), -GetParam(0, 1));
	}
	bool CursorColumn()
	{
		return SetCursor(GetParam(0, 0), GetCursorPos().Y);
	}
	bool CursorPosition()
	{
		return SetCursor(GetParam(0, 1), GetParam(1, 1));
	}
	bool EraseDisplay()
	{
		// TODO (charlie): respect parameter as follows:
		//	0 : from top left to cursor
		//	1 : from cursor to bottom right
		//	2 : whole screen
		return ClearConsole();
	}
	bool EraseInLine()
	{
		return false;
	}
	bool ScrollUp()
	{
		return false;
	}
	bool ScrollDown()
	{
		return false;
	}
	bool SetColor()
	{
		for (auto i : mParameters)
		{
			switch (i)
			{
			case 0:	ResetAttr();	break;
			case 1:	SetFGBold();	break;
			case 2:	ClearFGBold();	break;
			case 3:	SetBGBold();	break;
			case 6:	ClearBGBold();	break;
			default:
				if (i >= 30 && i <= 37)
				{
					SetFGColor(i - 30);
				}
				else if (i >= 40 && i <= 47)
				{
					SetBGColor(i - 40);
				}
				break;
			}
		}
		return true;
	}
	bool SaveCursorPosition()
	{
		return false;
	}
	bool RestoreCursorPosition()
	{
		return false;
	}
	bool HideCursor()
	{
		if (mParameters[0] == 25)
		{
			CONSOLE_CURSOR_INFO info;
			GetConsoleCursorInfo(mFileHandle, &info);
			info.bVisible = FALSE;
			SetConsoleCursorInfo(mFileHandle, &info);
			return true;
		}
		return false;
	}
	bool ShowCursor()
	{
		if (mParameters[0] == 25)
		{
			CONSOLE_CURSOR_INFO info;
			GetConsoleCursorInfo(mFileHandle, &info);
			info.bVisible = TRUE;
			SetConsoleCursorInfo(mFileHandle, &info);
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////

	FILE *		mFilePtr;
	HANDLE		mFileHandle;
	bool		mIsTTY;
	int			mFileNum;
	WORD		mAttr;
	bool		mAttrChanged;
	vector<int>	mParameters;
};

//////////////////////////////////////////////////////////////////////


struct AnsiParser
{
	Console &	mFile;
	int				mNumber;
	int				mState;
	tchar			mEsc1Char;		// Usually [
	tchar			mEsc2Char;		// Usually 0 or ? or =
	tchar const *	mString;
	tchar const *	mText;
	tchar const *	mTextEnd;
	tchar const *	mCursor;

	static tchar const *sControlChars;

	enum State
	{
		Begin,
		ESC1,
		ESC2,
		ESC3,
		Text,
		Digits,
		PostDigits,
		Done
	};

	typedef int (AnsiParser::*StateFunction)(int);
	static std::map<int, StateFunction> StateFunctions;

	void Flush(tchar const *end)
	{
		mFile.Write(mText, end - mText);
	}

	int begin(int c)
	{
		mText = null;
		mEsc1Char = 0;
		mEsc2Char = 0;
		if (c == 27)
		{
			return ESC1;
		}
		mText = mCursor - 1;
		return Text;
	}

	int esc1(int c)
	{
		mEsc1Char = c;
		return (c == '[') ? ESC2 : Begin;
	}

	int esc2(int c)
	{
		mEsc2Char = c;
		if (c == '?' || c == '=')
		{
			return ESC3;
		}
		if (iswspace(c) || c == ';')
		{
			return ESC2;
		}
		if (isdigit(c))
		{
			mNumber = c - '0';
			return Digits;
		}
		return Begin;
	}

	int esc3(int c)
	{
		if (iswspace(c) || c == ';')
		{
			return ESC3;
		}
		if (isdigit(c))
		{
			mNumber = c - '0';
			return Digits;
		}
		return Begin;
	}

	int text(int c)
	{
		if (c == 27)
		{
			Flush(mCursor - 1);
			return ESC1;
		}
		return Text;
	}

	int digits(int c)
	{
		if (isdigit(c))
		{
			mNumber = mNumber * 10 + c - '0';
			return Digits;
		}
		if (iswspace(c))
		{
			mFile.AddParameter(mNumber);
			return PostDigits;
		}
		if (c == ';')
		{
			mFile.AddParameter(mNumber);
			return ESC2;
		}
		mFile.AddParameter(mNumber);
		mFile.ProcessCommand(c);
		return Begin;
	}

	int post_digits(int c)
	{
		if (iswspace(c))
		{
			return PostDigits;
		}
		if (c == ';')
		{
			return ESC2;
		}
		mFile.ProcessCommand(c);
		return Begin;
	}

	bool ProcessChar(int c)
	{
		if (c == 0)
		{
			return false;
		}
		auto f = StateFunctions.find(mState);
		if (f != StateFunctions.end())
		{
			auto func = f->second;
			mState = (this->*func)(c);
			return true;
		}
		return false;
	}

	AnsiParser(tchar const *str, Console &file)
		: mState(Begin)
		, mString(str)
		, mFile(file)
	{
	}
		
	void ProcessString()
	{
		mCursor = mString;
		while (ProcessChar(*mCursor++))
		{
		}
		Flush(mCursor - 1);
	}
};

//////////////////////////////////////////////////////////////////////

tchar const *AnsiParser::sControlChars = $("ABCDEFGHJKSTfmsu");

std::map<int, Console::ControlDefinition> Console::ControlDefinitions =
{
	{ (int)'A',{ &CursorUp, 0, 1, $("CursorUp") } },
	{ (int)'B',{ &CursorDown, 0, 1, $("CursorDown") } },
	{ (int)'C',{ &CursorForward, 0, 1, $("CursorForward") } },
	{ (int)'D',{ &CursorBack, 0, 1, $("CursorBack") } },
	{ (int)'E',{ &CursorNextLine, 0, 1, $("CursorNextLine") } },
	{ (int)'F',{ &CursorPrevLine, 0, 1, $("CursorPrevLine") } },
	{ (int)'G',{ &CursorColumn, 0, 1, $("CursorColumn") } },
	{ (int)'H',{ &CursorPosition, 0, 2, $("CursorPosition") } },
	{ (int)'J',{ &EraseDisplay, 0, 1, $("EraseDisplay") } },
	{ (int)'K',{ &EraseInLine, 0, 1, $("EraseInLine") } },
	{ (int)'S',{ &ScrollUp, 0, 1, $("ScrollUp") } },
	{ (int)'T',{ &ScrollDown, 0, 1, $("ScrollDown") } },
	{ (int)'f',{ &CursorPosition, 0, 2, $("CursorPosition") } },
	{ (int)'m',{ &SetColor, 1, -1, $("SetColor") } },
	{ (int)'s',{ &SaveCursorPosition, 0, 0, $("SaveCursorPosition") } },
	{ (int)'u',{ &RestoreCursorPosition, 0, 0, $("RestoreCursorPosition") } },
	{ (int)'l',{ &HideCursor, 1, 1, $("HideCursor") } },
	{ (int)'h',{ &ShowCursor, 1, 1, $("ShowCursor") } },
};

std::map<int, AnsiParser::StateFunction> AnsiParser::StateFunctions =
{
	{ AnsiParser::Begin,		&begin },
	{ AnsiParser::ESC1,			&esc1 },
	{ AnsiParser::ESC2,			&esc2 },
	{ AnsiParser::ESC3,			&esc3 },
	{ AnsiParser::Text,			&text },
	{ AnsiParser::Digits,		&digits },
	{ AnsiParser::PostDigits,	&post_digits },
};

//////////////////////////////////////////////////////////////////////

void ansi_fwrite(FILE *f, tchar const *str)
{
	Console file(f);
	AnsiParser parser(str, file);
	parser.ProcessString();
}

//////////////////////////////////////////////////////////////////////

void ansi_write(tchar const *str)
{
	ansi_fwrite(stdout, str);
}

//////////////////////////////////////////////////////////////////////

void ansi_printf(tchar const *fmt, ...)
{
	va_list v;
	va_start(v, fmt);
	ansi_write(Format_V(fmt, v).c_str());
}

//////////////////////////////////////////////////////////////////////

void ansi_printf(tstring const &fmt, ...)
{
	va_list v;
	va_start(v, fmt);
	ansi_write(Format_V(fmt.c_str(), v).c_str());
}
