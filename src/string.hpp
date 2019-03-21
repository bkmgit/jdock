#pragma once
#ifndef IDOCK_STRING_HPP
#define IDOCK_STRING_HPP

#include <string>
using namespace std;

//!< Remove leading and trailing white spaces from a string.
string trim(const string&& str);

//!< Remove leading white spaces from a string.
string trim_start(const string&& str);

//!< Remove trailing white spaces from a string.
string trim_end(const string&& str);

//!< Right-aligns the characters by padding them on the left with a specified character, for a specified total length.
string pad_left(const string&& str, size_t total_width, char padding_char);

//!< Left-aligns the characters by padding them on the right with a specified character, for a specified total length.
string pad_right(const string&& str, size_t total_width, char padding_char);
#endif