#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QMessageBox>
#include <QStackedLayout>

#include "CustomGroupBox.hpp"
#include "SmoothSlider.hpp"

#include "AudioRedirector.hpp"

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	MainWindow();
	~MainWindow();

private:
	void setupMainUI();
	void connectSignals();
	void connectLoopbackSignals();
	void connectCaptureSignals();
	void populateDropdowns();
	void setupDefaults();

	bool startLoopbackRedirect();
	bool startCaptureRedirect();

	void restartLoopbackRedirect();
	void restartCaptureRedirect();

private:
	void showWarning(const QString &title, const QString &message) {
		QMessageBox::warning(this, title, message, QMessageBox::Ok);
	}

private:
	struct UIWidgets {
		QComboBox *inputDropdown;
		QComboBox *outputDropdown;
		QComboBox *sampleRateDropdown;
		QComboBox *formatDropdown;
		QComboBox *volumeBoostDropdown;
		SmoothSlider *volumeSlider;
		QLabel *volumeLabel;
		QPushButton *startButton;
	};

	UIWidgets m_loopbackUI;
	UIWidgets m_captureUI;

	QPushButton *m_loopbackButton;
	QPushButton *m_captureButton;

	CustomGroupBox *m_groupbox;
	QStackedLayout *m_stack;

private:
	AudioDevices m_audioDevices = {nullptr, 0, nullptr, 0};
};
