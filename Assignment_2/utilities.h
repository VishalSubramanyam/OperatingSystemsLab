#ifndef ASSIGNMENT2_UTILITIES_H
#define ASSIGNMENT2_UTILITIES_H

#include <string>

char const *BANNER_STR = "                                                                                     \n"
						 "           88                       88  88    ,ad8888ba,   88888888ba  888888888888  \n"
						 "           88                       88  88   d8\"'    `\"8b  88      \"8b      88       \n"
						 "           88                       88  88  d8'            88      ,8P      88       \n"
						 ",adPPYba,  88,dPPYba,    ,adPPYba,  88  88  88             88aaaaaa8P'      88       \n"
						 "I8[    \"\"  88P'    \"8a  a8P_____88  88  88  88      88888  88\"\"\"\"\"\"'        88       \n"
						 " `\"Y8ba,   88       88  8PP\"\"\"\"\"\"\"  88  88  Y8,        88  88               88       \n"
						 "aa    ]8I  88       88  \"8b,   ,aa  88  88   Y8a.    .a88  88               88       \n"
						 "`\"YbbdP\"'  88       88   `\"Ybbd8\"'  88  88    `\"Y88888P\"   88               88       \n"
						 "                                                                                     \n"
						 "                                                                                     ";

// trim from left
inline std::string &ltrim(std::string &s, const char *t = " \t\n\r\f\v") {
    s.erase(0, s.find_first_not_of(t));
    return s;
}

// trim from right
inline std::string &rtrim(std::string &s, const char *t = " \t\n\r\f\v") {
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

// trim from left & right
inline std::string &trim(std::string &s, const char *t = " \t\n\r\f\v") {
    return ltrim(rtrim(s, t), t);
}

// copying versions

inline std::string ltrim_copy(std::string s, const char *t = " \t\n\r\f\v") {
    return ltrim(s, t);
}

inline std::string rtrim_copy(std::string s, const char *t = " \t\n\r\f\v") {
    return rtrim(s, t);
}

inline std::string trim_copy(std::string s, const char *t = " \t\n\r\f\v") {
    return trim(s, t);
}

#define INSTALL_HANDLER(sig, signal_handler) signal(sig, signal_handler)

#endif //ASSIGNMENT2_UTILITIES_H
