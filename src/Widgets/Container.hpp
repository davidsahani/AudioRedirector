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

namespace _::internal {
	// Concept to ensure type is a QLayout (C++20)
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
}

namespace _::internal {
	struct LayoutArgs {
		std::optional<QString> objectName = std::nullopt;
		std::optional<Qt::Alignment> alignment = std::nullopt;
		std::optional<QBoxLayout::Direction> direction = std::nullopt;
		std::optional<QLayout::SizeConstraint> sizeConstraint = std::nullopt;
		std::optional<QMargins> contentsMargins = std::nullopt;
		std::optional<int> spacing = std::nullopt;
		std::optional<int> stretch = std::nullopt;
		QWidget* parent = nullptr;
		std::initializer_list<_::internal::BoxLayoutItem> children;
	};
}

template <_::internal::BoxLayoutT T>
T *Layout(std::initializer_list<_::internal::BoxLayoutItem> children, QWidget *parent = nullptr) {
	T *layout = new T(parent);

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

template <_::internal::BoxLayoutT T>
T *Layout(const _::internal::LayoutArgs &args) {
	T *layout = Layout<T>(args.children, args.parent);

	if (args.objectName.has_value()) {
		layout->setObjectName(args.objectName.value());
	}
	if (args.alignment.has_value()) {
		layout->setAlignment(args.alignment.value());
	}
	if (args.direction.has_value()) {
		layout->setDirection(args.direction.value());
	}
	if (args.sizeConstraint.has_value()) {
		layout->setSizeConstraint(args.sizeConstraint.value());
	}
	if (args.contentsMargins.has_value()) {
		layout->setContentsMargins(args.contentsMargins.value());
	}
	if (args.spacing.has_value()) {
		layout->setSpacing(args.spacing.value());
	}
	if (args.stretch.has_value()) {
		layout->setStretchFactor(layout, args.stretch.value());
	}
	return layout;
}

template <_::internal::BoxLayoutT T>
class Container : public QWidget {
public:
	Container(std::initializer_list<_::internal::BoxLayoutItem> children, QWidget *parent = nullptr)
		: QWidget(parent)
	{
		m_layout = Layout<T>(children, this);
		this->setLayout(m_layout);
	}

	Container(_::internal::LayoutArgs args)
		: QWidget(args.parent)
	{
		args.parent = this;
		m_layout = Layout<T>(args);
		this->setLayout(m_layout);
	}

	T *getLayout() const { return m_layout; }

private:
	T *m_layout;
};

// -------------------------------
// Widget helper functions
// -------------------------------

template <typename T>
T *Margins(const QMargins &margins, T *obj)
    requires requires(T *t, const QMargins &m) {
        { t->setContentsMargins(m) };
    }
{
    obj->setContentsMargins(margins);
    return obj;
}

template <_::internal::WidgetT T>
T *SizePolicy(T *widget, QSizePolicy::Policy hor, QSizePolicy::Policy ver)
{
    widget->setSizePolicy(hor, ver);
	return widget;
}
