//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

void ansi_write(tchar const *str);
void ansi_fwrite(FILE *f, tchar const *str);

void ansi_printf(tchar const *fmt, ...);
void ansi_printf(tstring const &fmt, ...);

void ansi_fprintf(FILE *f, tchar const *fmt, ...);
void ansi_fprintf(FILE *f, tstring const &fmt, ...);

#define ANSI_START	$("\x1b[")
#define ANSI_END	$("m")

#define ANSI_FG_BLACK	$("30;")
#define ANSI_FG_RED		$("31;")
#define ANSI_FG_GREEN	$("32;")
#define ANSI_FG_YELLOW	$("33;")
#define ANSI_FG_BLUE	$("34;")
#define ANSI_FG_MAGENTA	$("35;")
#define ANSI_FG_CYAN	$("36;")
#define ANSI_FG_WHITE	$("37;")

#define ANSI_BG_BLACK	$("40;")
#define ANSI_BG_RED		$("41;")
#define ANSI_BG_GREEN	$("42;")
#define ANSI_BG_YELLOW	$("43;")
#define ANSI_BG_BLUE	$("44;")
#define ANSI_BG_MAGENTA	$("45;")
#define ANSI_BG_CYAN	$("46;")
#define ANSI_BG_WHITE	$("47;")
#define ANSI_FB_BOLD	$("1;")
#define ANSI_FB_DIM		$("2;")
#define ANSI_BG_BOLD	$("3;")
#define ANSI_BG_DIM		$("6;")
#define ANSI_FGBG_RESET	$("0;")

#define FG_BLACK	ANSI_START		ANSI_FG_BLACK		ANSI_END
#define FG_RED		ANSI_START		ANSI_FG_RED			ANSI_END
#define FG_GREEN	ANSI_START		ANSI_FG_GREEN		ANSI_END
#define FG_YELLOW	ANSI_START		ANSI_FG_YELLOW		ANSI_END
#define FG_YELLOW	ANSI_START		ANSI_FG_YELLOW		ANSI_END
#define FG_BLUE		ANSI_START		ANSI_FG_BLUE		ANSI_END
#define FG_CYAN		ANSI_START		ANSI_FG_CYAN		ANSI_END
#define FG_WHITE	ANSI_START		ANSI_FG_WHITE		ANSI_END
#define BG_BLACK	ANSI_START		ANSI_BG_BLACK		ANSI_END
#define BG_RED		ANSI_START		ANSI_BG_RED			ANSI_END
#define BG_GREEN	ANSI_START		ANSI_BG_GREEN		ANSI_END
#define BG_YELLOW	ANSI_START		ANSI_BG_YELLOW		ANSI_END
#define BG_YELLOW	ANSI_START		ANSI_BG_YELLOW		ANSI_END
#define BG_BLUE		ANSI_START		ANSI_BG_BLUE		ANSI_END
#define BG_CYAN		ANSI_START		ANSI_BG_CYAN		ANSI_END
#define BG_WHITE	ANSI_START		ANSI_BG_WHITE		ANSI_END
#define FB_BOLD		ANSI_START		ANSI_FB_BOLD		ANSI_END
#define FB_DIM		ANSI_START		ANSI_FB_DIM			ANSI_END
#define BG_BOLD		ANSI_START		ANSI_BG_BOLD		ANSI_END
#define BG_DIM		ANSI_START		ANSI_BG_DIM			ANSI_END

#define ANSI_RESET	ANSI_START		ANSI_FGBG_RESET		ANSI_END
