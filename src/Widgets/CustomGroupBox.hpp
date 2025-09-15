#pragma once
#include <QGroupBox>

class CustomGroupBox : public QGroupBox {
	Q_OBJECT

public:
	explicit CustomGroupBox(QWidget *parent = nullptr);

	// Set a custom widget in the title area, aligned left or right.
	void setTitleWidget(QWidget *w, bool rightAligned = true);

	void resizeEvent(QResizeEvent *ev) override;
	void showEvent(QShowEvent *ev) override;

private:
	void updateTitleGeometry();

	QWidget *m_titleWidget;
	bool m_rightAligned;
};
