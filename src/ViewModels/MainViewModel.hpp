#pragma once
#include <QObject>
#include <QString>
#include <QIcon>

#include "MainView.hpp"
#include "AudioRedirector.hpp"

class MainViewModel : public QObject {
	Q_OBJECT

public:
	MainViewModel(
        const MainUIState &loopbackUIState,
        const MainUIState &captureUIState,
        QObject *parent = nullptr
    );
	~MainViewModel();

	void loadDevices();

signals:
	void errorOccurred(const QString &title, const QString &message);

private:
	void setDefaults();
	void populateDropdowns();
	void connectLoopbackSignals();
	void connectCaptureSignals();

	bool startLoopbackRedirect();
	bool startCaptureRedirect();

	void restartLoopbackRedirect();
	void restartCaptureRedirect();

private:
	MainUIState m_loopbackUIState;
	MainUIState m_captureUIState;
	AudioDevices m_audioDevices = { nullptr, 0, nullptr, 0 };
};
