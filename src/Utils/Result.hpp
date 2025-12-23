#pragma once
#include <type_traits>
#include <utility>
#include <variant>
#include <stdexcept>

namespace result::private_ {

template <typename T>
struct OkTag { T value; };

template <typename E>
struct ErrTag { E error; };

// Traits to detect any OkTag<*> / ErrTag<*>
template <class X> struct is_ok_tag  : std::false_type {};
template <class U> struct is_ok_tag<OkTag<U>> : std::true_type {};

template <class X> struct is_err_tag : std::false_type {};
template <class V> struct is_err_tag<ErrTag<V>> : std::true_type {};

template <class X>
inline constexpr bool is_ok_tag_v = is_ok_tag<std::remove_cvref_t<X>>::value;

template <class X>
inline constexpr bool is_err_tag_v = is_err_tag<std::remove_cvref_t<X>>::value;

} // namespace result::private_

template <typename T, typename E>
class Result {
    static_assert(!std::is_same_v<T, E>,
        "Result<T,E> requires T != E because holds_alternative/get<T> "
        "need a unique T in the variant."
    );

public:
    // OK (implicit constructor): constructible as T, not constructible as E
    template <class U>
    requires (
        std::is_constructible_v<T, U&&> &&
        !std::is_same_v<std::remove_cvref_t<U>, Result> &&
        !result::private_::is_ok_tag_v<U> &&
        !result::private_::is_err_tag_v<U> &&
        !std::is_same_v<std::remove_cvref_t<U>, E>
    )
    Result(U &&value) : _data(std::in_place_type<T>, std::forward<U>(value)) {}

    // ERR (implicit constructor): constructible as E, not constructible as T
    template <class U>
    requires (
        std::is_same_v<std::remove_cvref_t<U>, E> &&
        !std::is_same_v<std::remove_cvref_t<U>, Result> &&
        !result::private_::is_ok_tag_v<U> &&
        !result::private_::is_err_tag_v<U>
    )
    Result(U &&error) : _data(std::in_place_type<E>, std::forward<U>(error)) {}

    // OK: Explicit constructor for OkTag<U>
    template <class U> requires std::is_constructible_v<T, U&&>
    Result(result::private_::OkTag<U>&& ok) : _data(std::in_place_type<T>, std::forward<U>(ok.value)) {}

    // ERR: Explicit constructor for ErrTag<V>
    template <class V> requires std::is_same_v<std::remove_cvref_t<V>, E>
    Result(result::private_::ErrTag<V>&& err) : _data(std::in_place_type<E>, std::forward<V>(err.error)) {}

    // OK: Static named constructor
    [[nodiscard]] static Result Ok(T value) {
        return Result(std::move(value), ok_ctor_t{});
    }

    // ERR: Static named constructor
    [[nodiscard]] static Result Err(E error) {
        return Result(std::move(error), err_ctor_t{});
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

    struct ok_ctor_t {};
    struct err_ctor_t {};

    // Private constructors
    explicit Result(T &&value, ok_ctor_t) : _data(std::in_place_type<T>, std::move(value)) {}
    explicit Result(E &&error, err_ctor_t) : _data(std::in_place_type<E>, std::move(error)) {}
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
