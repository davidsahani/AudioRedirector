#pragma once
#include <concepts>
#include <variant>
#include <type_traits>
#include <initializer_list>
#include <optional>

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
	concept BoxLayoutT = std::derived_from<T, QBoxLayout>;

	template <typename T>
	concept WidgetT = std::derived_from<T, QWidget>;

	class BoxLayoutItem {
	public:
		using ItemVariant = std::variant<QWidget *, QLayout *, QLayoutItem *, Spacing, Stretch>;

		BoxLayoutItem(QWidget *w) : item(w) {}
		BoxLayoutItem(QLayout *l) : item(l) {}
		BoxLayoutItem(QLayoutItem *i) : item(i) {}
		BoxLayoutItem(Spacing sp) : item(sp) {}
		BoxLayoutItem(Stretch st) : item(st) {}

		const ItemVariant &get() const { return item; }

	private:
		ItemVariant item;
	};

	struct LayoutArgs {
		std::optional<QString> objectName = std::nullopt;
		std::optional<Qt::Alignment> alignment = std::nullopt;
		std::optional<QBoxLayout::Direction> direction = std::nullopt;
		std::optional<QLayout::SizeConstraint> sizeConstraint = std::nullopt;
		std::optional<QMargins> contentsMargins = std::nullopt;
		std::optional<int> spacing = std::nullopt;
		std::optional<int> stretch = std::nullopt;
		QWidget *parent = nullptr;
		std::initializer_list<__internal::BoxLayoutItem> children;
	};
} // namespace __internal

template <__internal::BoxLayoutT T>
T *Layout(T *layout, std::initializer_list<__internal::BoxLayoutItem> children) {
	for (auto &child : children) {
		std::visit(
			[&](auto &&ptr) {
				using T = std::decay_t<decltype(ptr)>;
				if constexpr (std::is_same_v<T, QWidget *>) {
					if (ptr) layout->addWidget(ptr);
				} else if constexpr (std::is_same_v<T, QLayout *>) {
					if (ptr) layout->addLayout(ptr);
				} else if constexpr (std::is_same_v<T, QSpacerItem *>) {
					if (ptr) layout->addItem(ptr);
				} else if constexpr (std::is_same_v<T, Spacing>) {
					layout->addSpacing(ptr.size);
				} else if constexpr (std::is_same_v<T, Stretch>) {
					layout->addStretch(ptr.factor);
				}
			},
			child.get()
		);
	}

	layout->setContentsMargins(0, 0, 0, 0);
	return layout;
}

template <__internal::BoxLayoutT T>
T *Layout(T *layout, const __internal::LayoutArgs &args) {
	T *l = Layout<T>(layout, args.children);

	if (args.objectName.has_value()) {
		l->setObjectName(args.objectName.value());
	}
	if (args.alignment.has_value()) {
		l->setAlignment(args.alignment.value());
	}
	if (args.direction.has_value()) {
		l->setDirection(args.direction.value());
	}
	if (args.sizeConstraint.has_value()) {
		l->setSizeConstraint(args.sizeConstraint.value());
	}
	if (args.contentsMargins.has_value()) {
		l->setContentsMargins(args.contentsMargins.value());
	}
	if (args.spacing.has_value()) {
		l->setSpacing(args.spacing.value());
	}
	if (args.stretch.has_value()) {
		l->setStretchFactor(l, args.stretch.value());
	}
	return l;
}

template <__internal::BoxLayoutT T>
T *Layout(const __internal::LayoutArgs &args) {
	return Layout<T>(new T(args.parent), args);
}

template <__internal::BoxLayoutT T>
T *Layout(std::initializer_list<__internal::BoxLayoutItem> children) {
	return Layout<T>(new T(), children);
}

template <__internal::BoxLayoutT T>
T *Layout(QWidget *parent, std::initializer_list<__internal::BoxLayoutItem> children) {
	return Layout<T>(new T(parent), children);
}

namespace __internal {
	struct ContainerArgs {
		std::optional<QString> objectName = std::nullopt;
		std::optional<QSize> baseSize = std::nullopt;
		std::optional<QSize> fixedSize = std::nullopt;
		std::optional<int> fixedWidth = std::nullopt;
		std::optional<int> fixedHeight = std::nullopt;
		std::optional<QSize> minimumSize = std::nullopt;
		std::optional<int> minimumWidth = std::nullopt;
		std::optional<int> minimumHeight = std::nullopt;
		std::optional<QSize> maximumSize = std::nullopt;
		std::optional<int> maximumWidth = std::nullopt;
		std::optional<int> maximumHeight = std::nullopt;
		std::optional<QMargins> contentsMargins = std::nullopt;
		std::optional<QSizePolicy> sizePolicy = std::nullopt;
		std::optional<QString> tooltip = std::nullopt;
		std::optional<int> tooltipDuration = std::nullopt;
		QLayout *layout = nullptr; // Layout for container
	};
} // namespace __internal

template <__internal::WidgetT T>
T *Container(T *widget, const __internal::ContainerArgs &args) {
	if (args.objectName.has_value()) {
		widget->setObjectName(args.objectName.value());
	}

	if (args.baseSize.has_value()) {
		widget->setBaseSize(args.baseSize.value());
	}
	if (args.fixedSize.has_value()) {
		widget->setFixedSize(args.fixedSize.value());
	}
	if (args.fixedWidth.has_value()) {
		widget->setFixedWidth(args.fixedWidth.value());
	}
	if (args.fixedHeight.has_value()) {
		widget->setFixedHeight(args.fixedHeight.value());
	}
	if (args.minimumSize.has_value()) {
		widget->setMinimumSize(args.minimumSize.value());
	}
	if (args.minimumWidth.has_value()) {
		widget->setMinimumWidth(args.minimumWidth.value());
	}
	if (args.minimumHeight.has_value()) {
		widget->setMinimumHeight(args.minimumHeight.value());
	}
	if (args.maximumSize.has_value()) {
		widget->setMaximumSize(args.maximumSize.value());
	}
	if (args.maximumWidth.has_value()) {
		widget->setMaximumWidth(args.maximumWidth.value());
	}
	if (args.maximumHeight.has_value()) {
		widget->setMaximumHeight(args.maximumHeight.value());
	}

	if (args.contentsMargins.has_value()) {
		widget->setContentsMargins(args.contentsMargins.value());
	}
	if (args.sizePolicy.has_value()) {
		widget->setSizePolicy(args.sizePolicy.value());
	}
	if (args.tooltip.has_value()) {
		widget->setToolTip(args.tooltip.value());
	}
	if (args.tooltipDuration.has_value()) {
		widget->setToolTipDuration(args.tooltipDuration.value());
	}

	if (args.layout != nullptr) {
		widget->setLayout(args.layout);
	}

	return widget;
}

template <__internal::WidgetT T>
T *Container(T *widget, QLayout *layout) {
	widget->setLayout(layout);
	return widget;
}

template <__internal::BoxLayoutT T>
QWidget *Container(__internal::LayoutArgs args) {
	QWidget *widget = new QWidget(args.parent);
	args.parent = widget;
	T *layout = Layout<T>(args);
	widget->setLayout(layout);
	return widget;
}

template <__internal::BoxLayoutT T>
QWidget *Container(std::initializer_list<__internal::BoxLayoutItem> children) {
	QWidget *widget = new QWidget();
	T *layout = Layout<T>(widget, children);
	widget->setLayout(layout);
	return widget;
}

template <__internal::BoxLayoutT T>
QWidget *Container(QWidget *parent, std::initializer_list<__internal::BoxLayoutItem> children) {
	QWidget *widget = new QWidget(parent);
	T *layout = Layout<T>(children, widget);
	widget->setLayout(layout);
	return widget;
}
