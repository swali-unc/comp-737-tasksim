#pragma once

#include <string>

std::string to_string_trim(double val);
std::string stringprintf(const char* fmt, ...);

#if defined(_WIN32) || defined(_WIN64)
int vasprintf(char** strp, const char* fmt, va_list ap);
#endif