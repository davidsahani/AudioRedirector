#include "SmoothSlider.hpp"
#include <QMouseEvent>

SmoothSlider::SmoothSlider(Qt::Orientation orientation, QWidget *parent)
	: QSlider(orientation, parent)
{
	this->setTickPosition(QSlider::NoTicks); // no tick snapping
}

void SmoothSlider::mousePressEvent(QMouseEvent *event) {
	if (event->button() == Qt::LeftButton) {
		// Map click position to slider value
		int val = 0;
		if (orientation() == Qt::Horizontal) {
			double ratio = static_cast<double>(event->pos().x()) / width();
			val = minimum() + ratio * (maximum() - minimum());
		} else {
			double ratio = static_cast<double>(height() - event->pos().y()) / height();
			val = minimum() + ratio * (maximum() - minimum());
		}
		setValue(val);
		event->accept();
	}
	QSlider::mousePressEvent(event);
}
