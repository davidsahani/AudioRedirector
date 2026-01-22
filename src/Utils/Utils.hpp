#pragma once
#include <string>
#include <Windows.h>

#include "Result.hpp"
#include "Error.hpp"

namespace Utils {
	Result<std::wstring, Error> GetDeviceIconPath(const wchar_t *deviceId);
	HICON ExtractDeviceIcon(const std::wstring &iconPath);
} // namespace Utils
