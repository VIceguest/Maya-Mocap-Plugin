////////////////////////////////////////////////////////////////////////////
//
//  MoCap File Importer
//  Copyright(c) Shun Cox (shuncox@gmail.com)
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
////////////////////////////////////////////////////////////////////////////

#ifndef __ICONVERTER_H__
#define __ICONVERTER_H__

#include <sstream>
#include <cctype>	// for toupper
#include <string>
#include <algorithm>

const std::string spaceCharacters(" \t\r\n");
const std::string commentCharacters("#");

///////////////////////////////////////////////////////////////////////////////
// conversion functions
//
///////////////////////////////////////////////////////////////////////////////
// conversion functions
//
template<typename T>
inline T fromString(const std::string &s, std::ios_base &(*f)(std::ios_base&) = std::dec) {
	istringstream is(s);
	T t;
	is >> f >> t;
	return t;
}

template<typename T>
inline std::string toString(const T &t, std::ios_base &(*f)(std::ios_base&) = std::dec) {
	ostringstream os;
	os << f << t;
	return os.str();
}

inline void toUppercase(std::string &s) {
	transform(s.begin(), s.end(), s.begin(), (int(*)(int))toupper);
}

inline void toLowercase(std::string &s) {
	transform(s.begin(), s.end(), s.begin(), (int(*)(int))tolower);
}

inline std::string trimRight(const std::string &s, const std::string &t = spaceCharacters) {
	std::string d(s); 
	std::string::size_type i(d.find_last_not_of(t));
	if (i == std::string::npos)
		return "";
	else
		return d.erase(i + 1) ; 
}

inline std::string trimLeft(const std::string &s, const std::string &t = spaceCharacters) { 
	std::string d(s); 
	return d.erase(0, s.find_first_not_of(t)) ; 
}

inline std::string trimString(const std::string &s, const std::string &t = spaceCharacters) { 
	std::string d(s); 
	return trimLeft(trimRight(d, t), t) ; 
}

inline void tokenizeString(const std::string &s, std::vector<std::string> &words, const std::string &t = spaceCharacters) {
	std::string::size_type lastPos = 0;
	std::string::size_type pos = s.find_first_of(t, lastPos);
	while (std::string::npos != pos || std::string::npos != lastPos) {
		std::string d(s.substr(lastPos, pos - lastPos));
		if (!d.empty()) words.push_back(d);
		lastPos = s.find_first_not_of(t, pos);
		pos = s.find_first_of(t, lastPos);
	}
}

inline std::string uncommentString(const std::string &s, const std::string &t = commentCharacters) {
	std::string d(s);
	std::string::size_type i(d.find_first_of(t));
	if (i != std::string::npos) {
		return trimRight(d.erase(i));
	} else {
		return d;
	}
}

inline int compareNoncase(const std::string &s1, const std::string &s2) {
	std::string d1(s1), d2(s2);
	toUppercase(d1);
	toUppercase(d2);
	return d1.compare(d2);
}

#endif	// #ifndef __ICONVERTER_H__
