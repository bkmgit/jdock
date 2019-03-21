#include "string.hpp"

//!< Remove leading and trailing white spaces from a string.
string trim(const string&& str)
{
	size_t i, j;
	for (i = 0; i < str.size() && isspace(str[i]); i++);
	for (j = str.size(); j > i && isspace(str[j - 1]); j--);

	return str.substr(i, j - i);
}

//!< Remove leading white spaces from a string.
string trim_start(const string&& str)
{
	size_t i;
	for (i = 0; i < str.size() && isspace(str[i]); i++);

	return str.substr(i);
}

//!< Remove trailing white spaces from a string.
string trim_end(const string&& str)
{
	size_t j;
	for (j = str.size(); j > 0 && isspace(str[j - 1]); j--);

	return str.substr(0, j);
}

//!< Right-aligns the characters by padding them on the left with a specified character, for a specified total length.
string pad_left(const string&& str, size_t total_width, char padding_char)
{
	if (total_width <= str.size())
		return str;
	return string(total_width - str.size(), padding_char) + str;
}

//!< Left-aligns the characters by padding them on the right with a specified character, for a specified total length.
string pad_right(const string&& str, size_t total_width, char padding_char)
{
	if (total_width <= str.size())
		return str;
	return str + string(total_width - str.size(), padding_char);
}
