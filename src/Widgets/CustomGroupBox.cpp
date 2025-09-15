#include "CustomGroupBox.hpp"

#include <QResizeEvent>
#include <QShowEvent>
#include <QStyleOptionGroupBox>

CustomGroupBox::CustomGroupBox(QWidget *parent) : QGroupBox(parent) {
	m_titleWidget = nullptr;
	m_rightAligned = true;
}

void CustomGroupBox::setTitleWidget(QWidget *w, bool rightAligned) {
	if (m_titleWidget) {
		m_titleWidget->setParent(nullptr);
	}
	m_titleWidget = w;
	m_rightAligned = rightAligned;

	if (m_titleWidget) {
		m_titleWidget->setParent(this); // important: direct child of groupbox
		m_titleWidget->show();
		m_titleWidget->raise();
		updateTitleGeometry();
	}
}

void CustomGroupBox::resizeEvent(QResizeEvent *ev) {
	QGroupBox::resizeEvent(ev);
	updateTitleGeometry();
}

void CustomGroupBox::showEvent(QShowEvent *ev) {
	QGroupBox::showEvent(ev);
	updateTitleGeometry();
}

void CustomGroupBox::updateTitleGeometry() {
	if (!m_titleWidget) return;

	QStyleOptionGroupBox opt;
	opt.initFrom(this);
	opt.text = QLatin1String(" "); // ensure label rect exists

	QRect labelRect = style()->subControlRect(QStyle::CC_GroupBox, &opt, QStyle::SC_GroupBoxLabel, this);

	QSize sh = m_titleWidget->sizeHint();
	int w = sh.width();
	int h = sh.height();

	int x;
	if (m_rightAligned) {
		const int rightMargin = 8;
		x = width() - w - rightMargin;
	} else {
		x = labelRect.left();
	}

	// center the widget vertically in the title area
	int y = labelRect.center().y() - h / 2;
	if (y < 0) y = 0;

	m_titleWidget->setGeometry(x, y, w, h);
	m_titleWidget->raise();
}
