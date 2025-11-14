#include "MainWindow.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QMessageBox>
#include <QTimer>

#include "Container.hpp"
#include "MainViewModel.hpp"

MainWindow::MainWindow() : QMainWindow() {
	this->setupMainUI();    // setup UI widgets
	this->connectSignals(); // connect signals and slots

	QTimer::singleShot(250, this, [this]() {
		m_viewModel->loadDevices();
	});
}

void MainWindow::setupMainUI() {
	QWidget *inputModeContainer = [this]() {
		m_loopbackButton = new QPushButton("Loopback");
		m_captureButton = new QPushButton("Capture");

		m_loopbackButton->setCheckable(true);
		m_captureButton->setCheckable(true);

		QButtonGroup *group = new QButtonGroup();
		group->setExclusive(true);
		group->addButton(m_loopbackButton);
		group->addButton(m_captureButton);

		return Container<QWidget>(
			new QWidget(),
			APPLY_EX(setObjectName("InputModeContainer")),
			Layout<QHBoxLayout>(
                APPLY_EX(setAlignment(Qt::AlignRight)),
                m_loopbackButton, m_captureButton
            )
		);
	}();

	MainUIState loopbackUIState;
	MainUIState captureUIState;
	loopbackUIState.inputLabel = new QLabel("Select Input Playback Device:");
	captureUIState.inputLabel = new QLabel("Select Input Capture Device:");

	QWidget *loopbackView = CreateMainView(loopbackUIState);
	QWidget *captureView = CreateMainView(captureUIState);

	m_viewModel = new MainViewModel(loopbackUIState, captureUIState, this);

	m_stack = new QStackedLayout();
	m_stack->addWidget(loopbackView);
	m_stack->addWidget(captureView);

	m_groupbox = new CustomGroupBox();
	m_groupbox->setTitleWidget(inputModeContainer);
	m_groupbox->setLayout(m_stack);

	// Set default input mode to capture
	m_captureButton->setChecked(true);
	m_stack->setCurrentIndex(1);
	m_groupbox->resizeEvent(nullptr);

	this->setCentralWidget(Container<QVBoxLayout>(Spacing(2), m_groupbox));
}

void MainWindow::connectSignals() {
	connect(m_loopbackButton, &QPushButton::clicked, this, [this]() {
		m_stack->setCurrentIndex(0);
		m_groupbox->resizeEvent(nullptr);
	});

	connect(m_captureButton, &QPushButton::clicked, this, [this]() {
		m_stack->setCurrentIndex(1);
		m_groupbox->resizeEvent(nullptr);
	});

	connect(
		m_viewModel, &MainViewModel::errorOccurred, this,
        [this](const QString &title, const QString &message) {
			QMessageBox::warning(this, title, message, QMessageBox::Ok);
		}
	);
}
