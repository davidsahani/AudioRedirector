#include "MainWindow.hpp"
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedLayout>
#include <QButtonGroup>
#include <QMessageBox>

#include "Container.hpp"
#include "CustomGroupBox.hpp"
#include "MainViewModel.hpp"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	setupMainUI();
}

void MainWindow::showEvent(QShowEvent *event) {
    QMainWindow::showEvent(event);

	MainUIState loopbackUIState, captureUIState;
	loopbackUIState.inputLabel = new QLabel("Select Input Playback Device:");
	captureUIState.inputLabel = new QLabel("Select Input Capture Device:");

	QWidget *loopbackView = CreateMainView(loopbackUIState);
	QWidget *captureView = CreateMainView(captureUIState);

	// Parent it to prevent flickering on initial show
	loopbackView->setParent(m_groupbox);
	captureView->setParent(m_groupbox);

	m_viewModel = new MainViewModel(loopbackUIState, captureUIState, this);

	m_stack->addWidget(loopbackView);
	m_stack->addWidget(captureView);

	// Set default input mode to capture
	m_captureButton->setChecked(true);
	m_stack->setCurrentIndex(1);
	m_groupbox->resizeEvent(nullptr);

	connectSignals();
	m_viewModel->loadDevices();
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

	m_groupbox = new CustomGroupBox();
	m_groupbox->setTitleWidget(inputModeContainer);

	m_stack = new QStackedLayout();
	m_groupbox->setLayout(m_stack);

	setCentralWidget(Container<QVBoxLayout>(Spacing(2), m_groupbox));
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
