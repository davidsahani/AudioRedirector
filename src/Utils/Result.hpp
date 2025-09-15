#pragma once
#include <string>
#include <variant>
#include <stdexcept>

namespace result::private_ {
	template <typename T>
	struct OkTag {
		T value;
	};

	template <typename E>
	struct ErrTag {
		E error;
	};
} // namespace result::private_

template <typename T, typename E>
class Result {
public:
	// Template constructor for OkTag<U>
	template <typename U, typename = std::enable_if_t<std::is_convertible_v<U, T>>, typename = void>
	Result(result::private_::OkTag<U> &&ok) : _data(std::move(ok.value)) {}

	// Template constructor for ErrTag<V>
	template <typename V, typename = std::enable_if_t<std::is_convertible_v<V, E>>, typename = void>
	Result(result::private_::ErrTag<V> &&err) : _data(std::move(err.error)) {}

	// Constructor for success result
	[[nodiscard]] static Result Ok(T value) {
		return Result(std::move(value));
	}

	// Constructor for error result
	[[nodiscard]] static Result Err(E error) {
		return Result(std::move(error), false);
	}

	// Check if result is success
	bool has_value() const {
		return std::holds_alternative<T>(_data);
	}

	explicit operator bool() const {
		return has_value();
	}

	// Access the value (throws if it's an error)
	T &value() noexcept(false) {
		if (!has_value()) throw std::logic_error("Called value() on an Err");
		return std::get<T>(_data);
	}

	const T &value() const noexcept(false) {
		if (!has_value()) throw std::logic_error("Called value() on an Err");
		return std::get<T>(_data);
	}

	// Access the error (throws if it's a value)
	E &error() noexcept(false) {
		if (has_value()) throw std::logic_error("Called error() on an Ok");
		return std::get<E>(_data);
	}

	const E &error() const noexcept(false) {
		if (has_value()) throw std::logic_error("Called error() on an Ok");
		return std::get<E>(_data);
	}

	// Pattern matching-like API
	template <typename OkFn, typename ErrFn>
	auto match(OkFn okFn, ErrFn errFn) const {
		if (has_value())
			return okFn(std::get<T>(_data));
		else
			return errFn(std::get<E>(_data));
	}

private:
	std::variant<T, E> _data;

	// Private constructors
	explicit Result(T &&value) : _data(std::move(value)) {}
	explicit Result(E &&error, bool) : _data(std::move(error)) {}
};

namespace result {
	template <typename T>
	[[nodiscard]] private_::OkTag<std::decay_t<T>> Ok(T &&value) {
		return private_::OkTag<std::decay_t<T>>{std::forward<T>(value)};
	}

	template <typename E>
	[[nodiscard]] private_::ErrTag<std::decay_t<E>> Err(E &&error) {
		return private_::ErrTag<std::decay_t<E>>{std::forward<E>(error)};
	}
} // namespace result
