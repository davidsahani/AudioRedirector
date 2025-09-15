#pragma once
#include <QSlider>

class SmoothSlider : public QSlider {
	Q_OBJECT

public:
	explicit SmoothSlider(Qt::Orientation orientation, QWidget *parent = nullptr);

protected:
	void mousePressEvent(QMouseEvent *event) override;
};
