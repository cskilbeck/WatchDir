//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////

using string = std::string;
template<typename T> using vector = std::vector<T>;

//////////////////////////////////////////////////////////////////////

template <class container_t, class string_t, class char_t>
void tokenize(string_t const &str, container_t &tokens, char_t const *delimiters, bool includeEmpty = true) {
	string_t::size_type end = 0, start = 0, len = str.size();
	while (end < len) {
		end = str.find_first_of(delimiters, start);
		if (end == string_t::npos) {
			end = len;
		}
		if (end != start || includeEmpty) {
			tokens.push_back(container_t::value_type(str.data() + start, end - start));
		}
		start = end + 1;
	}
}

//////////////////////////////////////////////////////////////////////

template <class container_t, class char_t>
void tokenize(char_t const *str, container_t &tokens, char_t const *delimiters, bool includeEmpty = true) {
	assert(str != null);
	assert(delimiters != null);

	size_t delimCount = Length(delimiters);
	size_t len = Length(str);	// LAME
	char_t const *start = str, *end = str, *stop = start + len;
	while (start < stop) 	{
		end = FindFirstOf(start, delimiters, delimCount);
		if (end == null) {
			end = stop;
		}
		if (end != start || includeEmpty) {
			tokens.push_back(container_t::value_type(start, end - start));
		}
		start = end + 1;
	}
}

//////////////////////////////////////////////////////////////////////

string ltrim(string const &s) {
	auto b = std::find_if_not(s.begin(), s.end(), std::isspace);
	return s.substr(b - s.begin());
}

//////////////////////////////////////////////////////////////////////

static inline string rtrim(string const &s) {
	auto e = std::find_if_not(s.rbegin(), s.rend(), std::isspace);
	return s.substr(0, e.base() - s.begin());
}

//////////////////////////////////////////////////////////////////////

static inline string trim(string const &s) {
	auto b = std::find_if_not(s.begin(), s.end(), std::isspace);
	auto e = std::find_if_not(s.rbegin(), s.rend(), std::isspace);
	return s.substr(b - s.begin(), e.base() - b);
}

//////////////////////////////////////////////////////////////////////

struct Condition {
	char const *name;
	DWORD flag;

	bool operator == (string const &n) {
		return _stricmp(trim(n).c_str(), name) == 0;
	}
};

//////////////////////////////////////////////////////////////////////

vector<Condition> condition_defs = {
	{ "FILE_NAME", FILE_NOTIFY_CHANGE_FILE_NAME },
	{ "DIR_NAME", FILE_NOTIFY_CHANGE_DIR_NAME },
	{ "ATTRIBUTES", FILE_NOTIFY_CHANGE_ATTRIBUTES },
	{ "SIZE", FILE_NOTIFY_CHANGE_SIZE },
	{ "LAST_WRITE", FILE_NOTIFY_CHANGE_LAST_WRITE },
	{ "SECURITY", FILE_NOTIFY_CHANGE_SECURITY }
};

//////////////////////////////////////////////////////////////////////

int _tmain()
{
	vector<HANDLE> handles;
	string s;
	int line = 0;
	while (std::getline(std::cin, s)) {
		vector<string> tokens;
		tokenize(s, tokens, ",", true);

		if (tokens.size() == 4) {
			// 0 - dir
			// 1 - conditions
			// 2 - recursive
			// 3 - action
			DWORD flags = 0;
			vector<string> conditions;
			tokenize(tokens[1], conditions, "+", true);
			for (auto const &condition : conditions) {
				auto c = std::find(condition_defs.begin(), condition_defs.end(), condition);
				if (c < condition_defs.end()) {
					flags += c->flag;
				}
			}
			string rc = trim(tokens[2]);
			BOOL recurse = _stricmp(rc.c_str(), "true") == 0 || _stricmp(rc.c_str(), "recursive") == 0;

//			HANDLE h = FindFirstChangeNotification(tokens[0].c_str(), recurse, flags);
			printf("DIR: %s, CONDITIONS: %08x, RECURSE: %d, ACTION: %s\n", tokens[0].c_str(), flags, recurse, tokens[3].c_str());
		}
		else {
			std::cerr << "Bad input at line " << line << std::endl;
		}
		++line;
	}
	getchar();
    return 0;
}
