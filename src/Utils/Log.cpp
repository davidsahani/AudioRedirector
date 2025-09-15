#include "Log.hpp"
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <mutex>

// Log to file "log.txt" by default.
std::string g_logFilepath = "log.txt";
// Mutex for thread-safe logging
std::mutex g_logMutex;

void Log::SetLogFile(const std::string &filepath) {
	g_logFilepath = filepath;
}

void Log::private_::LogMessage(Level level, const std::string &message) {
	std::string levelStr = GetLevelString(level);
	// Lock mutex before IO to avoid race conditions
	std::lock_guard<std::mutex> lock(g_logMutex);

#ifndef NDEBUG
	std::ostream &outStream = (level == Level::Error || level == Level::Fatal) ? std::cerr : std::cout;
	std::ostringstream oss;
	oss << "[" << levelStr << "] " << message;
	outStream << oss.str() << '\n'; // write to console
#else
	const std::string timestamp = GetTimestamp();
	std::ofstream logFile(g_logFilepath, std::ios::app);

	std::string msg; // token
	bool timestampAdded = false;
	std::istringstream iss(message);

	while (std::getline(iss, msg, '\n')) {
		if (msg.empty()) continue;

		if (!timestampAdded) {
			logFile << "[" << timestamp << "] " << levelStr << ": " << msg << '\n';
			timestampAdded = true;
			continue;
		}
		// Align continuation lines; 3 => 2 for brackets + 1 for space
		std::string spacing = std::string(timestamp.size() + 3, ' ');
		logFile << spacing << msg << '\n';
	}
#endif
}

std::string Log::private_::GetLevelString(Level level) {
	switch (level) {
		case Level::Debug:
			return "DEBUG";
		case Level::Info:
			return "INFO";
		case Level::Warning:
			return "WARNING";
		case Level::Error:
			return "ERROR";
		case Level::Fatal:
			return "FATAL";
		default:
			return "UNKNOWN";
	}
}

std::string Log::private_::GetTimestamp() {
	std::time_t now = std::time(nullptr);
	std::tm localTime{};

#if defined(_WIN32) || defined(_WIN64)
	// Windows: secure version
	localtime_s(&localTime, &now);
#else
	// POSIX: thread-safe version
	localtime_r(&now, &localTime);
#endif

	std::ostringstream oss;
	oss << std::put_time(&localTime, "%d-%m-%Y %H:%M:%S");
	return oss.str();
}

void Log::private_::LogWithTracebackImpl(
	const char *file, int line, const char *func, Level level, const std::string &message
) {
	std::ostringstream oss;
#ifndef NDEBUG
	oss << message << "\nTraceback: " << file << ":" << line << " in function: " << func;
#else
	std::filesystem::path p(file);
	std::string filename = p.filename().string();
	oss << message << "\nTraceback: " << filename << ":" << line << " in function: " << func;
#endif
	LogMessage(level, oss.str());
}
