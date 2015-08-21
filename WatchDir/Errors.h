//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

enum
{
	success = 0,
	err_usage = -1,
	err_no_input_specified = -2,
	err_input_not_found = -3,
	err_lowest_error = -4
};

//////////////////////////////////////////////////////////////////////

__declspec(selectany) tchar const *error_text[] =
{
	$("completed successfully"),
	$("usage was incorrect"),
	$("input file was not specified"),
	$("input file was not found")
};

//////////////////////////////////////////////////////////////////////

static inline tchar const *GetErrorText(int error)
{
	return (error > 0 || error < err_lowest_error) ? $("?") : error_text[-error];
}

//////////////////////////////////////////////////////////////////////

static inline void error_exit(int error)
{
	ftprintf(stderr, $("Exiting because %s.\n"), GetErrorText(error));
	exit(error);
}