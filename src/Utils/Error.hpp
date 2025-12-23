#pragma once
#include <string>
#include <sstream>
#include <filesystem>

#define _SHOULD_USE_DETAILED_FUNCTION_NAME_IN_SOURCE_LOCATION 0
#include "source_location.h"

struct Error {
	std::string message;
	std::string traceback;

	Error(const std::string &msg, const std::string &traceback)
		: message(msg), traceback(traceback) {}

	Error(const std::string &msg, source_location loc = source_location::current())
		: message(msg)
	{
		std::ostringstream oss;
#ifndef NDEBUG
		oss << loc.file_name();
#else
		oss << std::filesystem::path(loc.file_name()).filename().string();
#endif
		oss << ":" << loc.line() << " in function: " << loc.function_name();
		traceback = oss.str();
	}

	const std::string str() const {
		std::ostringstream oss;
		oss << message;
		oss << "\nTraceback: " << traceback;
		return oss.str();
	}
};
