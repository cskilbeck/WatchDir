//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <iostream>

//////////////////////////////////////////////////////////////////////

tstring input_filename;

using option::Option;
vector<Option> options;

//////////////////////////////////////////////////////////////////////

struct Arg: public option::Arg
{
	//////////////////////////////////////////////////////////////////////

	static void printError(const char* msg1, const Option& opt, const char* msg2)
	{
		fprintf(stderr, "ERROR: %s", msg1);
		fwrite(opt.name, opt.namelen, 1, stderr);
		fprintf(stderr, "%s", msg2);
	}

	//////////////////////////////////////////////////////////////////////

	static option::ArgStatus Flag(Option const &option, bool msg)
	{
		return option::ARG_OK;
	}

	//////////////////////////////////////////////////////////////////////

	static option::ArgStatus NonEmpty(const Option& option, bool msg)
	{
		if(option.arg != null && option.arg[0] != 0)
		{
			return option::ARG_OK;
		}

		if(msg)
		{
			printError("Option '", option, "' requires an argument");
		}
		return option::ARG_ILLEGAL;
	}
};

//////////////////////////////////////////////////////////////////////

option::Descriptor const usage[] =
{
	{ UNKNOWN, 0, "", "", option::Arg::None, "USAGE: watchdir options\n\nWatchDir\n\nWatches directories for activity\nand executes commands when things happen.\n\nOptions:" },
	{ 0, 0, null, null, null, null }
};

//////////////////////////////////////////////////////////////////////

option::Parser *args = null;

bool ParseArgs(int argc, char *argv[], vector<Option> &options)
{
	options.clear();
	if(argc > 0)
	{
		--argc;
		++argv;
	}
	option::Stats stats(usage, argc, argv);
	options.resize(stats.options_max);
	vector<Option> buffer(stats.buffer_max);
	static option::Parser parse(usage, argc, argv, &options[0], &buffer[0]);
	args = &parse;
	return parse.error() == false;
}

//////////////////////////////////////////////////////////////////////

void PrintUsage()
{
	option::printUsage(std::cerr, usage);

	ansi_printf($("\nOptions specified:\n\n"));
	for(auto &o : options)
	{
		if(o.desc != null)
		{
			tstring s = $("");
			if(o.arg != null)
			{
				s = Format($("=%s"), o.arg);
			}
			ansi_printf($("%s%s\n"), o.name, s.c_str());
		}
	}
}

//////////////////////////////////////////////////////////////////////

int CheckArgs(int argc, char *argv[])
{
	// parse the args
	if(!ParseArgs(argc, argv, options))
	{
		return err_usage;
	}

	if(args->nonOptionsCount() == 0)
	{
		error($("No source file specified"));
		PrintUsage();
		return err_no_input_specified;
	}

	// check input file exists
	if(!FileExists(TString(args->nonOptions()[0]).c_str()))
	{
		error($("Source file doesn't exist"));
		return err_input_not_found;
	}

	input_filename = TString(args->nonOptions()[0]);
	return success;
}

