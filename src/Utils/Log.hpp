#pragma once
#include <string>
#include <format>
#include <codecvt>

namespace Log {
	enum class Level
	{
		Debug,
		Info,
		Warning,
		Error,
		Fatal
	};

	namespace private_ {
		void LogMessage(Level level, const std::string &message);
		std::string GetLevelString(Level level);
		std::string GetTimestamp(); // Get local time stamp
	} // namespace private_

	void SetLogFile(const std::string &filepath);

	template <typename... Args>
	static void Debug(std::format_string<Args...> fmtStr, Args &&...args) {
#ifndef NDEBUG
		private_::LogMessage(Level::Debug, std::format(fmtStr, std::forward<Args>(args)...));
#else
		(void)fmtStr; // avoid unused warning
					  // arguments are unused — compiler will optimize everything away
#endif
	}

	template <typename... Args>
	void Info(std::format_string<Args...> fmtStr, Args &&...args) {
		private_::LogMessage(Level::Info, std::format(fmtStr, std::forward<Args>(args)...));
	}

	template <typename... Args>
	void Warning(std::format_string<Args...> fmtStr, Args &&...args) {
		private_::LogMessage(Level::Warning, std::format(fmtStr, std::forward<Args>(args)...));
	}

	template <typename... Args>
	void Error(std::format_string<Args...> fmtStr, Args &&...args) {
		private_::LogMessage(Level::Error, std::format(fmtStr, std::forward<Args>(args)...));
	}

	template <typename... Args>
	void Fatal(std::format_string<Args...> fmtStr, Args &&...args) {
		private_::LogMessage(Level::Fatal, std::format(fmtStr, std::forward<Args>(args)...));
	}
} // namespace Log

// Formatter for std::wstring
template <>
struct std::formatter<std::wstring, char> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	template <typename FormatContext>
	auto format(const std::wstring &wstr, FormatContext &ctx) const {
		std::string utf8Str = {};

		// NOTE: deprecated in C++17 but still available
		if (!wstr.empty()) {
			std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
			utf8Str = conv.to_bytes(wstr);
		}
		return std::format_to(ctx.out(), "{}", utf8Str);
	}
};

inline std::wstring operator"" _wstr(const wchar_t *str, std::size_t len) {
	return std::wstring(str, len);
}

inline std::wstring wstr(const wchar_t *s) {
	return std::wstring(s);
}

namespace Log::private_ {
	void LogWithTracebackImpl(const char *file, int line, const char *func, Level level, const std::string &message);

	template <typename... Args>
	void LogWithTraceback(
		const char *file, int line, const char *func, Level level, std::format_string<Args...> fmtStr, Args &&...args
	) {
		LogWithTracebackImpl(file, line, func, level, std::format(fmtStr, std::forward<Args>(args)...));
	}
} // namespace Log::private_

#ifndef NDEBUG
	#define LOG_DEBUG(...) Log::private_::LogWithTraceback(__FILE__, __LINE__, __func__, Log::Level::Debug, ##__VA_ARGS__)
#else
	#define LOG_DEBUG(...) ((void)0)
#endif
#define LOG_INFO(...) Log::private_::LogWithTraceback(__FILE__, __LINE__, __func__, Log::Level::Info, ##__VA_ARGS__)
#define LOG_WARNING(...) Log::private_::LogWithTraceback(__FILE__, __LINE__, __func__, Log::Level::Warning, ##__VA_ARGS__)
#define LOG_ERROR(...) Log::private_::LogWithTraceback(__FILE__, __LINE__, __func__, Log::Level::Error, ##__VA_ARGS__)
#define LOG_FATAL(...) Log::private_::LogWithTraceback(__FILE__, __LINE__, __func__, Log::Level::Fatal, ##__VA_ARGS__)
