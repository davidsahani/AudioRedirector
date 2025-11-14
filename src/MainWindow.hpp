#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QStackedLayout>

#include "CustomGroupBox.hpp"

class MainViewModel;

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	MainWindow();

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
