#pragma once
#include <QWidget>
#include <QLayout>
#include <QSpacerItem>
#include <initializer_list>
#include <variant>
#include <type_traits>
#include <concepts>

// Concept to ensure type is a QLayout (C++20)
template <typename T>
concept LayoutT = std::derived_from<T, QLayout>;

struct Spacing {
	int size;
};

struct Stretch {
	int factor;
};

class ContainerItem {
public:
	using ItemVariant = std::variant<QWidget *, QLayout *, QSpacerItem *, Spacing, Stretch>;

	ContainerItem(QWidget *w) : item(w) {}
	ContainerItem(QLayout *l) : item(l) {}
	ContainerItem(QSpacerItem *s) : item(s) {}
	ContainerItem(Spacing sp) : item(sp) {}
	ContainerItem(Stretch st) : item(st) {}

	const ItemVariant &get() const {
		return item;
	}

private:
	ItemVariant item;
};

template <typename LayoutT>
LayoutT *Layout(std::initializer_list<ContainerItem> children, QWidget *parent = nullptr) {
	LayoutT *layout = new LayoutT(parent);

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

template <LayoutT LayoutType>
class Container : public QWidget {
public:
	Container(std::initializer_list<ContainerItem> children, QWidget *parent = nullptr)
		: QWidget(parent)
	{
		m_layout = Layout<LayoutType>(children, this);
		this->setLayout(m_layout);
	}

	LayoutType *getLayout() const { return m_layout; }

	Container<LayoutType> *setLayoutSpacing(int spacing) {
		m_layout->setSpacing(spacing);
		return this;
	}

	Container<LayoutType> *setLayoutAlignment(Qt::Alignment alignment) {
		m_layout->setAlignment(alignment);
		return this;
	}

	Container<LayoutType> *setLayoutContentMargins(int left, int top, int right, int bottom) {
		m_layout->setContentsMargins(left, top, right, bottom);
		return this;
	}

	Container<LayoutType> *setLayoutContentMargins(const QMargins &margins) {
		m_layout->setContentsMargins(margins);
		return this;
	}

private:
	LayoutType *m_layout;
};

// -------------------------------
// Widget helper functions
// -------------------------------

template <typename T>
T *Margins(T *obj, const QMargins &margins)
	requires requires(T *t, const QMargins &m) {
		{ t->setContentsMargins(m) };
	}
{
	obj->setContentsMargins(margins);
	return obj;
}
