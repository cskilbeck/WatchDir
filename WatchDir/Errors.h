//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

enum
{
	success = 0,
	err_usage = -1,
	err_no_input_specified = -2,
	err_input_not_found = -3,
	err_file_not_found = -4,
	err_file_err = -5,
	err_bad_input = -6,
	err_lowest_error = -7
};

//////////////////////////////////////////////////////////////////////

__declspec(selectany) tchar const *error_text[] =
{
	$("completed successfully"),
	$("usage was incorrect"),
	$("input file was not specified"),
	$("input file was not found"),
	$("file was not found"),
	$("file reading failed"),
	$("input file was malformed")
};

//////////////////////////////////////////////////////////////////////

static inline tchar const *GetErrorText(int error)
{
	return (error > 0 || error < err_lowest_error) ? $("?") : error_text[-error];
}

//////////////////////////////////////////////////////////////////////

static inline void error_exit(int error)
{
	ansi_fprintf(stderr, $("Exiting because %s.\n"), GetErrorText(error));
	exit(error);
}