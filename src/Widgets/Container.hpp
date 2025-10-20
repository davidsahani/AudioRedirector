#pragma once
#include <concepts>
#include <variant>
#include <type_traits>
#include <initializer_list>

#include <QWidget>
#include <QBoxLayout>
#include <QLayoutItem>

struct Spacing {
	int size;
};

struct Stretch {
	int factor;
};

namespace __internal {
	template <typename T>
	concept LayoutT = std::derived_from<T, QLayout>;

	template <typename T>
	concept WidgetT = std::derived_from<T, QWidget>;

	inline void addChild(QLayout *layout, QWidget *widget) {
		layout->addWidget(widget);
	}
	inline void addChild(QBoxLayout *layout, QLayout *sublayout) {
		layout->addLayout(sublayout);
	}
	inline void addChild(QLayout *layout, QLayoutItem *item) {
		layout->addItem(item);
	}
	inline void addChild(QBoxLayout *layout, Spacing spacing) {
		layout->addSpacing(spacing.size);
	}
	inline void addChild(QBoxLayout *layout, Stretch stretch) {
		layout->addStretch(stretch.factor);
	}

	// Concept that checks whether addChild(layout, arg) is valid for the Layout type.
	template <typename T, typename Arg>
	concept AddableToLayout = requires(T *l, Arg &&a) {
		// expression must be valid
		addChild(l, std::forward<Arg>(a));
	};
} // namespace __internal

template <__internal::LayoutT T, typename... Args>
	requires(__internal::AddableToLayout<T, Args> && ...)
T *Layout(T *layout, Args &&...children) {
	assert(layout != nullptr && "Layout: layout pointer must not be null");

	// Call the corresponding addChild for each argument
	((__internal::addChild(layout, std::forward<Args>(children))), ...);

	layout->setContentsMargins(0, 0, 0, 0);
	return layout;
}

template <__internal::LayoutT T, typename... Args>
	requires(__internal::AddableToLayout<T, Args> && ...)
T *Layout(Args &&...children) {
	return Layout<T>(new T(), children...);
}

template <__internal::LayoutT T, typename... Args>
	requires(__internal::AddableToLayout<T, Args> && ...)
T *Layout(T *layout, std::function<void(T *)> callable, Args &&...children) {
	Layout<T>(layout, children...);
	callable(layout); // invoke callback
	return layout;
}

template <__internal::LayoutT T, typename... Args>
	requires(__internal::AddableToLayout<T, Args> && ...)
T *Layout(std::function<void(T *)> callable, Args &&...children) {
	return Layout<T>(new T(), callable, children...);
}

template <__internal::WidgetT T>
T *Container(T *widget, QLayout *layout) {
	widget->setLayout(layout);
	return widget;
}

template <__internal::LayoutT T, typename... Args>
	requires(__internal::AddableToLayout<T, Args> && ...)
QWidget *Container(Args &&...children) {
	QWidget *widget = new QWidget();
	T *layout = new T(widget);
	widget->setLayout(Layout<T>(layout, children...));
	return widget;
}

template <__internal::WidgetT T>
T *Container(T *widget, std::function<void(T *)> callable) {
	callable(widget);
	return widget;
}

template <__internal::WidgetT T>
T *Container(T *widget, std::function<void(T *)> callable, QLayout *layout) {
	callable(widget);
	widget->setLayout(layout);
	return widget;
}

// --- helpers: token pasting and expansion ---
#define UIX_PP_CAT(a, b) a##b
#define UIX_PP_EXPAND(x) x

// --- count __VA_ARGS__ up to 16 ---
#define UIX_PP_NARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, N, ...) N
#define UIX_PP_NARGS(...) UIX_PP_EXPAND( \
    UIX_PP_NARGS_IMPL(__VA_ARGS__, \
        16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0))

// --- apply m(obj, arg) to each arg (1..16) ---
#define UIX_PP_FOREACH_1(m, obj, a1) m(obj, a1)
#define UIX_PP_FOREACH_2(m, obj, a1, a2) m(obj,a1) m(obj,a2)
#define UIX_PP_FOREACH_3(m, obj, a1, a2, a3) m(obj,a1) m(obj,a2) m(obj,a3)
#define UIX_PP_FOREACH_4(m, obj, a1, a2, a3, a4) m(obj,a1) m(obj,a2) m(obj,a3) m(obj,a4)
#define UIX_PP_FOREACH_5(m, obj, a1, a2, a3, a4, a5) m(obj,a1) m(obj,a2) m(obj,a3) m(obj,a4) m(obj,a5)
#define UIX_PP_FOREACH_6(m, obj, a1, a2, a3, a4, a5, a6) m(obj,a1) m(obj,a2) m(obj,a3) m(obj,a4) m(obj,a5) m(obj,a6)
#define UIX_PP_FOREACH_7(m, obj, a1, a2, a3, a4, a5, a6, a7) m(obj,a1) m(obj,a2) m(obj,a3) m(obj,a4) m(obj,a5) m(obj,a6) m(obj,a7)
#define UIX_PP_FOREACH_8(m, obj, a1, a2, a3, a4, a5, a6, a7, a8) m(obj,a1) m(obj,a2) m(obj,a3) m(obj,a4) m(obj,a5) m(obj,a6) m(obj,a7) m(obj,a8)
#define UIX_PP_FOREACH_9(m, obj, a1, a2, a3, a4, a5, a6, a7, a8, a9) m(obj,a1) m(obj,a2) m(obj,a3) m(obj,a4) m(obj,a5) m(obj,a6) m(obj,a7) m(obj,a8) m(obj,a9)
#define UIX_PP_FOREACH_10(m, obj, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) m(obj,a1) m(obj,a2) m(obj,a3) m(obj,a4) m(obj,a5) m(obj,a6) m(obj,a7) m(obj,a8) m(obj,a9) m(obj,a10)
#define UIX_PP_FOREACH_11(m, obj, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) m(obj,a1) m(obj,a2) m(obj,a3) m(obj,a4) m(obj,a5) m(obj,a6) m(obj,a7) m(obj,a8) m(obj,a9) m(obj,a10) m(obj,a11)
#define UIX_PP_FOREACH_12(m, obj, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) m(obj,a1) m(obj,a2) m(obj,a3) m(obj,a4) m(obj,a5) m(obj,a6) m(obj,a7) m(obj,a8) m(obj,a9) m(obj,a10) m(obj,a11) m(obj,a12)
#define UIX_PP_FOREACH_13(m, obj, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) m(obj,a1) m(obj,a2) m(obj,a3) m(obj,a4) m(obj,a5) m(obj,a6) m(obj,a7) m(obj,a8) m(obj,a9) m(obj,a10) m(obj,a11) m(obj,a12) m(obj,a13)
#define UIX_PP_FOREACH_14(m, obj, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14) m(obj,a1) m(obj,a2) m(obj,a3) m(obj,a4) m(obj,a5) m(obj,a6) m(obj,a7) m(obj,a8) m(obj,a9) m(obj,a10) m(obj,a11) m(obj,a12) m(obj,a13) m(obj,a14)
#define UIX_PP_FOREACH_15(m, obj, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15) m(obj,a1) m(obj,a2) m(obj,a3) m(obj,a4) m(obj,a5) m(obj,a6) m(obj,a7) m(obj,a8) m(obj,a9) m(obj,a10) m(obj,a11) m(obj,a12) m(obj,a13) m(obj,a14) m(obj,a15)
#define UIX_PP_FOREACH_16(m, obj, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16) m(obj,a1) m(obj,a2) m(obj,a3) m(obj,a4) m(obj,a5) m(obj,a6) m(obj,a7) m(obj,a8) m(obj,a9) m(obj,a10) m(obj,a11) m(obj,a12) m(obj,a13) m(obj,a14) m(obj,a15) m(obj,a16)

#define UIX_PP_FOREACH_N(n, m, obj, ...) UIX_PP_EXPAND(UIX_PP_CAT(UIX_PP_FOREACH_, n)(m, obj, __VA_ARGS__))
#define UIX_PP_FOREACH(m, obj, ...) UIX_PP_FOREACH_N(UIX_PP_NARGS(__VA_ARGS__), m, obj, __VA_ARGS__)

// --- turn "foo(args...)" into "obj->foo(args...);" ---
#define UIX_PP_MEMBER_CALL(obj, call) obj->call;

// --- public macro: produces a generic-lambda that applies each call to obj ---
#define APPLY_EX(...) ([](auto *obj) { UIX_PP_FOREACH(UIX_PP_MEMBER_CALL, obj, __VA_ARGS__) })
#define APPLY_EX_REF(...) ([&](auto *obj) { UIX_PP_FOREACH(UIX_PP_MEMBER_CALL, obj, __VA_ARGS__) })
#define APPLY_EX_WITH(capture, ...) ([capture](auto *obj) { UIX_PP_FOREACH(UIX_PP_MEMBER_CALL, obj, __VA_ARGS__) })
