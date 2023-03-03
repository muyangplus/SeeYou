#pragma once

#include <iostream>
#include <string>
#include <string.h>
#include <stdlib.h>

const bool GBK = false;

class Chcp {
public:
	static std::string GbkToUtf8(const char* src_str);
	static std::string Utf8ToGbk(const char* src_str);
};
