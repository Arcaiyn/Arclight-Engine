#pragma once

#include "util/log.h"


template<class... Args>
void __arc_assert(bool condition, const std::string& file, int line, const std::string& message, const Args&... args) {

	if (!condition) {
		Log::error("Assert", "Assertion failed (File " + file + ", line " + std::to_string(line) + "): " + message, args...);
		std::abort();
	}

}


#ifndef NDEBUG
	#define arc_assert(cond, msg, ...)	do {__arc_assert((cond), __FILE__, __LINE__, (msg), ##__VA_ARGS__);} while (false)
#else
	#define arc_assert(cond, msg, ...)
#endif