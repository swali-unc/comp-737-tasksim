#include "Utility.hpp"

#include <stdarg.h>

using std::string;
using std::to_string;

string to_string_trim(double val) {
	string str = to_string(val);
	str.erase(str.find_last_not_of('0') + 1, std::string::npos);
	str.erase(str.find_last_not_of('.') + 1, std::string::npos);
	return str;
}

string stringprintf(const char* fmt, ...) {
    char* message = nullptr;
    int msgLen;

    va_list list;
    va_start(list, fmt);
    msgLen = vasprintf(&message, fmt, list); // God bless GNU
    va_end(list);

    if (msgLen == -1) return std::string(""); // allocation error
    if (!message) return std::string(""); // I think only *BSD does this

    std::string retStr = message;

    free(message); // vasprintf requires free

    return retStr;
}

#if defined(_WIN32) || defined(_WIN64)
int vasprintf(char** strp, const char* fmt, va_list ap) {
    // _vscprintf tells you how big the buffer needs to be
    int len = _vscprintf(fmt, ap);
    if (len == -1) {
        return -1;
    }
    size_t size = (size_t)len + 1;
    char* str = (char*)malloc(size);
    if (!str) {
        return -1;
    }
    // _vsprintf_s is the "secure" version of vsprintf
    int r = vsprintf_s(str, (size_t)len + 1, fmt, ap);
    if (r == -1) {
        free(str);
        return -1;
    }
    *strp = str;
    return r;
}
#endif