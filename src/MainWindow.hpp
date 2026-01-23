#pragma once
#include <QMainWindow>

class QPushButton;
class QStackedLayout;
class CustomGroupBox;
class MainViewModel;

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);

protected:
	void showEvent(QShowEvent *event) override;

private:
	void setupMainUI();
	void connectSignals();

private:
	QPushButton *m_loopbackButton;
	QPushButton *m_captureButton;

	CustomGroupBox *m_groupbox;
	QStackedLayout *m_stack;

	MainViewModel *m_viewModel;
};
