#pragma once
#include "Error.hpp"
#include "Utils.hpp"

namespace error::private_ {
	Error add_win_error(HRESULT hr, Error error) {
		error.message += std::format("\nReason: {}", Utils::FormatWinError(hr));
		return error;
	}
}; // namespace error::private_

#define WinErr(hr, ...) error::private_::add_win_error(hr, Error(##__VA_ARGS__))
