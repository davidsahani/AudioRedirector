#pragma once
#include <string>
#include <optional>
#include "miniaudio.h"

namespace ma::convert {
	const char *to_string(ma_format format);
	const char *to_string(ma_result result);
	std::optional<ma_format> to_format(std::string_view formatStr);
} // namespace ma::convert
