#include "MainWindow.hpp"
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>

#include "Container.hpp"

#include "MAConvert.hpp"
#include "Utils.hpp"

MainWindow::MainWindow() : QMainWindow()
{
    this->setupMainUI(); // setup UI widgets
    ResultVoid result = AudioRedirector::Initialize();

    if (!result.has_value()) {
        this->showWarning(
            "Failed to initialize audio redirector",
            QString::fromStdString(result.error().str())
        );
    }

    QTimer::singleShot(250, this, [this]() {
        auto devicesResult = AudioRedirector::GetAudioDevices();

        if (devicesResult.has_value()) {
            m_audioDevices = devicesResult.value();
        } else {
            this->showWarning(
                "Failed to get audio devices",
                QString::fromStdString(devicesResult.error().str())
            );
        }

        this->populateDropdowns();  // populate dropdowns
		this->setupDefaults();      // setup config defaults
        this->connectSignals();     // connect signals and slots
        this->update();             // Trigger UI update
	});
}

MainWindow::~MainWindow() {
    AudioRedirector::Uninitialize();
}

static QWidget *createMainUI(
    _In_ QLabel *inputLabel,
    _Out_ QComboBox *&inputDropdown,
    _Out_ QComboBox *&outputDropdown,
    _Out_ QComboBox *&sampleRateDropdown,
    _Out_ QComboBox *&formatDropdown,
    _Out_ QComboBox *&volumeBoostDropdown,
    _Out_ SmoothSlider *&volumeSlider,
    _Out_ QLabel *&volumeLabel,
    _Out_ QPushButton *&startButton
) {
    return Container<QVBoxLayout>({
        Container<QLabel>(
            inputLabel,
            APPLY(setContentsMargins(4, 0, 0, 2))
        ),
        inputDropdown = new QComboBox(),
        Spacing(5),
        Container<QLabel>(
            new QLabel("Select Output Playback Device:"),
            APPLY(setContentsMargins(4, 0, 0, 2))
        ),
        outputDropdown = new QComboBox(),
        Spacing(15),
        Layout<QVBoxLayout>(APPLY(
            setContentsMargins(5, 0, 0, 0)
        ), {
            Layout<QHBoxLayout>({
                new QLabel("Sample Rate:"), 
                sampleRateDropdown = new QComboBox()
            }),
            Layout<QHBoxLayout>({
                new QLabel("Format:"), 
                formatDropdown = new QComboBox()
            }),
            Layout<QHBoxLayout>({
                new QLabel("Volume Boost:"),
                volumeBoostDropdown = new QComboBox()
            }),
            Spacing(15),
            Layout<QHBoxLayout>({
                new QLabel("Volume:"),
                [&]() {
                    volumeSlider = new SmoothSlider(Qt::Horizontal);
                    volumeSlider->setRange(0, 100);
                    volumeSlider->setValue(100);
                    volumeSlider->setSingleStep(2);
                    return volumeSlider;
                }(),
                volumeLabel = new QLabel("100%")
            })
        }),
        Spacing(15),
        Stretch(1), startButton = new QPushButton("Start")
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
            APPLY(setObjectName("InputModeContainer")),                
            Layout<QHBoxLayout>(
                APPLY(setAlignment(Qt::AlignRight)),
                { m_loopbackButton, m_captureButton }
            )
        );
    }();

    // Create loopback ui container
    QWidget *loopbackUIContainer = createMainUI(
        new QLabel("Select Input Loopback Device:"),
        m_loopbackUI.inputDropdown,
        m_loopbackUI.outputDropdown,
        m_loopbackUI.sampleRateDropdown,
        m_loopbackUI.formatDropdown,
        m_loopbackUI.volumeBoostDropdown,
        m_loopbackUI.volumeSlider,
        m_loopbackUI.volumeLabel,
        m_loopbackUI.startButton
    );

    // Create capture ui container
    QWidget *captureUIContainer = createMainUI(
        new QLabel("Select Input Capture Device:"),
        m_captureUI.inputDropdown,
        m_captureUI.outputDropdown,
        m_captureUI.sampleRateDropdown,
        m_captureUI.formatDropdown,
        m_captureUI.volumeBoostDropdown,
        m_captureUI.volumeSlider,
        m_captureUI.volumeLabel,
        m_captureUI.startButton
    );

    QLayout *containerlayout = loopbackUIContainer->layout();
    containerlayout->setAlignment(Qt::AlignTop);
    containerlayout->setContentsMargins(10, 5, 10, 10);

    containerlayout = captureUIContainer->layout();
    containerlayout->setAlignment(Qt::AlignTop);
    containerlayout->setContentsMargins(10, 5, 10, 10);

    m_stack = new QStackedLayout();
    m_stack->addWidget(loopbackUIContainer);
    m_stack->addWidget(captureUIContainer);

    m_groupbox = new CustomGroupBox();
    m_groupbox->setTitleWidget(inputModeContainer);
    m_groupbox->setLayout(m_stack);

	this->setCentralWidget(Container<QVBoxLayout>({ Spacing(2), m_groupbox }));
}

void MainWindow::setupDefaults() {
    // Set default input mode to capture
    m_captureButton->setChecked(true);
    m_stack->setCurrentIndex(1);
    m_groupbox->resizeEvent(nullptr);

    const QString loopbackFormat = QString::fromStdString(ma::convert::to_string(AudioRedirector::GetLoopbackFormat()));
    const QString loopbackSampleRate = QStringLiteral("%1 Hz").arg(AudioRedirector::GetLoopbackSampleRate());

    const QString duplexFormat = QString::fromStdString(ma::convert::to_string(AudioRedirector::GetDuplexFormat()));
    const QString duplexSampleRate = QStringLiteral("%1 Hz").arg(AudioRedirector::GetDuplexSampleRate());

    m_loopbackUI.formatDropdown->setCurrentText(loopbackFormat);
    m_loopbackUI.sampleRateDropdown->setCurrentText(loopbackSampleRate);

    m_captureUI.formatDropdown->setCurrentText(duplexFormat);
    m_captureUI.sampleRateDropdown->setCurrentText(duplexSampleRate);
}

void MainWindow::populateDropdowns() {
    const QIcon microphoneIcon(":/icons/microphone.ico");
    int defaultCaptureIndex = 0; /* Active default capture device */

    for (ma_uint32 i = 0; i < m_audioDevices.captureDeviceCount; ++i) {
        const ma_device_info device_info = m_audioDevices.captureDeviceInfos[i];

        auto result = Utils::GetDeviceIconPath(device_info.id.wasapi);
        QIcon icon = microphoneIcon;

        if (result.has_value()) {
            HICON hIcon = Utils::ExtractDeviceIcon(result.value());
            if (hIcon) {
                QImage image = QImage::fromHICON(hIcon);
                DestroyIcon(hIcon); // Clean up the HICON
                icon = QIcon(QPixmap::fromImage(image));
            }
        }

        if (device_info.isDefault) {
            defaultCaptureIndex = i;
        }

        const QString name = QString::fromUtf8(device_info.name);
        m_captureUI.inputDropdown->addItem(icon, name);
    }

    const QIcon speakerIcon(":/icons/speaker.ico");
    int defaultPlaybackIndex = 0; /* Active default playback device */

    for (ma_uint32 i = 0; i < m_audioDevices.playbackDeviceCount; ++i) {
        ma_device_info device_info = m_audioDevices.playbackDeviceInfos[i];

        auto result = Utils::GetDeviceIconPath(device_info.id.wasapi);
        QIcon icon = speakerIcon;

        if (result.has_value()) {
            HICON hIcon = Utils::ExtractDeviceIcon(result.value());
            if (hIcon) {
                QImage image = QImage::fromHICON(hIcon);
                DestroyIcon(hIcon); // Clean up the HICON
                icon = QIcon(QPixmap::fromImage(image));
            }
        }

        if (device_info.isDefault) {
            defaultPlaybackIndex = i;
        }

        const QString name = QString::fromUtf8(device_info.name);
        m_loopbackUI.inputDropdown->addItem(icon, name);
        m_loopbackUI.outputDropdown->addItem(icon, name);
        m_captureUI.outputDropdown->addItem(icon, name);
    }

    m_captureUI.inputDropdown->setCurrentIndex(defaultCaptureIndex);
    m_captureUI.outputDropdown->setCurrentIndex(defaultPlaybackIndex);
    m_loopbackUI.inputDropdown->setCurrentIndex(defaultPlaybackIndex);

    // -----------------------------------
    // Populate formats and sample rates
    // -----------------------------------

    for (const ma_format &format : AudioRedirector::Formats) {
        const QString fmt = QString::fromStdString(ma::convert::to_string(format));
        m_loopbackUI.formatDropdown->addItem(fmt);
        m_captureUI.formatDropdown->addItem(fmt);
    }

    for (const ma_uint32 &sampleRate : AudioRedirector::SampleRates) {
        const QString rate = QStringLiteral("%1 Hz").arg(sampleRate);
        m_loopbackUI.sampleRateDropdown->addItem(rate);
        m_captureUI.sampleRateDropdown->addItem(rate);
    }

    // Populate volume boost dropdown

    const QStringList items = {
        "1x Boost — Normal",
        "2x Boost — Double",
        "3x Boost — Triple",
        "4x Boost — Quadruple",
        "5x Boost — Quintuple",
        "6x Boost — Sextuple",
        "7x Boost — Septuple",
        "8x Boost — Octuple",
        "9x Boost — Nonuple",
        "10x Boost — Decuple"
    };

    m_loopbackUI.volumeBoostDropdown->addItems(items);
    m_captureUI.volumeBoostDropdown->addItems(items);
}

void MainWindow::connectSignals() {
    this->connect(m_loopbackButton, &QPushButton::clicked, this, [this]() {
        m_stack->setCurrentIndex(0);
        m_groupbox->resizeEvent(nullptr);
    });

    this->connect(m_captureButton, &QPushButton::clicked, this, [this]() {
        m_stack->setCurrentIndex(1);
        m_groupbox->resizeEvent(nullptr);
    });

    this->connectLoopbackSignals();
    this->connectCaptureSignals();
}

void MainWindow::connectLoopbackSignals() {
    this->connect(m_loopbackUI.inputDropdown, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index >= 0) this->restartLoopbackRedirect();
    });

    this->connect(m_loopbackUI.outputDropdown, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index >= 0) this->restartLoopbackRedirect();
    });

    this->connect(m_loopbackUI.formatDropdown, &QComboBox::currentTextChanged, this, [this](const QString &text) {
        std::optional<ma_format> formatOpt = ma::convert::to_format(text.toStdString());
        if (!formatOpt.has_value()) {
            return this->showWarning(
                "Unknown Format String",
                QStringLiteral("The selected format (%1) is not recognized.").arg(text)
            );
        }

        AudioRedirector::SetLoopbackFormat(formatOpt.value());
        this->restartLoopbackRedirect();  // Restart redirect if running to apply new format
    });

    this->connect(m_loopbackUI.sampleRateDropdown, &QComboBox::currentTextChanged, this, [this](const QString &text) {
        bool ok = false;
        QString str = text;
        int sampleRate = str.replace(" Hz", "").toInt(&ok);
        if (!ok) {
            return this->showWarning(
                "Conversion Error: Invalid Sample Rate",
                QStringLiteral("Could not convert string (%1) to number.").arg(text)
            );
        }

        AudioRedirector::SetLoopbackSampleRate(sampleRate);
        this->restartLoopbackRedirect();  // Restart redirect if running to apply new sample rate
    });

    this->connect(m_loopbackUI.volumeBoostDropdown, &QComboBox::currentIndexChanged, this, [this](int index) {
        m_loopbackUI.volumeSlider->setRange(0, 100 * (index + 1));
    });

    this->connect(m_loopbackUI.volumeSlider, &QSlider::valueChanged, this, [this](int value) {
        ma_result result = AudioRedirector::SetPlaybackVolume((value / 100.0f) + 0.1);

        if (result == MA_SUCCESS) {
            m_loopbackUI.volumeLabel->setText(QString("%1%").arg(value));
        } else {
            this->showWarning(
                "Volume Error",
                QStringLiteral("Failed to set output volume (%1).").arg(ma::convert::to_string(result))
            );
        }
    });

    this->connect(m_loopbackUI.startButton, &QPushButton::clicked, this, [this]() {
        if (m_loopbackUI.startButton->text() == "Start") {
            if (this->startLoopbackRedirect()) {
                m_loopbackUI.startButton->setText("Stop");
            }
        } else {
            ResultVoid result = AudioRedirector::StopLoopbackRedirect();

            if (result.has_value()) {
                m_loopbackUI.startButton->setText("Start");
            } else {
                this->showWarning(
                    "Error stopping loopback audio redirect",
                    QString::fromStdString(result.error().str())
                );
            }
        }
    });
}

void MainWindow::connectCaptureSignals() {
    this->connect(m_captureUI.inputDropdown, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index >= 0) this->restartCaptureRedirect();
    });

    this->connect(m_captureUI.outputDropdown, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index >= 0) this->restartCaptureRedirect();
    });

    this->connect(m_captureUI.formatDropdown, &QComboBox::currentTextChanged, this, [this](const QString &text) {
        std::optional<ma_format> formatOpt = ma::convert::to_format(text.toStdString());
        if (!formatOpt.has_value()) {
            return this->showWarning(
                "Unknown Format String",
                QStringLiteral("The selected format (%1) is not recognized.").arg(text)
            );
        }

        AudioRedirector::SetDuplexFormat(formatOpt.value());
        this->restartCaptureRedirect();  // Restart redirect if running to apply new format
    });

    this->connect(m_captureUI.sampleRateDropdown, &QComboBox::currentTextChanged, this, [this](const QString &text) {
        bool ok = false;
        QString str = text;
        int sampleRate = str.replace(" Hz", "").toInt(&ok);
        if (!ok) {
            return this->showWarning(
                "Conversion Error: Invalid Sample Rate",
                QStringLiteral("Could not convert string (%1) to number.").arg(text)
            );
        }

        AudioRedirector::SetDuplexSampleRate(sampleRate);
        this->restartCaptureRedirect();  // Restart redirect if running to apply new sample rate
    });

    this->connect(m_captureUI.volumeBoostDropdown, &QComboBox::currentIndexChanged, this, [this](int index) {
        m_captureUI.volumeSlider->setRange(0, 100 * (index + 1));
    });

    this->connect(m_captureUI.volumeSlider, &QSlider::valueChanged, this, [this](int value) {
        ma_result result = AudioRedirector::SetDuplexVolume((value / 100.0f) + 0.1);

        if (result == MA_SUCCESS) {
            m_captureUI.volumeLabel->setText(QString("%1%").arg(value));
        } else {
            this->showWarning(
                "Volume Error",
                QStringLiteral("Failed to set output volume (%1).").arg(ma::convert::to_string(result))
            );
        }
    });

    this->connect(m_captureUI.startButton, &QPushButton::clicked, this, [this]() {
        if (m_captureUI.startButton->text() == "Start") {
            if (this->startCaptureRedirect()) {
                m_captureUI.startButton->setText("Stop");
            }
        } else {
            ResultVoid result = AudioRedirector::StopDuplexRedirect();

            if (result.has_value()) {
                m_captureUI.startButton->setText("Start");
            } else {
                this->showWarning(
                    "Error stopping capture audio redirect",
                    QString::fromStdString(result.error().str())
                );
            }
        }
    });
}

bool MainWindow::startLoopbackRedirect() {
    const int inputIndex = m_loopbackUI.inputDropdown->currentIndex();
    const int outputIndex = m_loopbackUI.outputDropdown->currentIndex();

    if (inputIndex < 0 || outputIndex < 0) {
        this->showWarning(
            "Selection Error",
            QStringLiteral("Please select an %1 device").arg(
                inputIndex < 0 ? "input" : "output")
        );
        return false;
    }

    if (inputIndex >= static_cast<int>(m_audioDevices.playbackDeviceCount)) {
        this->showWarning(
            "Selection Error", 
            "Selected input loopback device index is out of range."
        );
        return false;
    }

    if (outputIndex >= static_cast<int>(m_audioDevices.playbackDeviceCount)) {
        this->showWarning("Selection Error", "Selected output device index is out of range.");
        return false;
    }

    ResultVoid result = AudioRedirector::StartLoopbackRedirect(
        &m_audioDevices.playbackDeviceInfos[inputIndex].id,
        &m_audioDevices.playbackDeviceInfos[outputIndex].id
    );

    if (!result.has_value()) {
        this->showWarning(
            "Error starting loopback audio redirect",
            QString::fromStdString(result.error().str())
        );
        return false;
    }
    return true;
}

bool MainWindow::startCaptureRedirect() {
    const int inputIndex = m_captureUI.inputDropdown->currentIndex();
    const int outputIndex = m_captureUI.outputDropdown->currentIndex();

    if (inputIndex < 0 || outputIndex < 0) {
        this->showWarning(
            "Selection Error",
            QStringLiteral("Please select an %1 device").arg(
                inputIndex < 0 ? "input" : "output")
        );
        return false;
    }

    if (inputIndex >= static_cast<int>(m_audioDevices.captureDeviceCount)) {
        this->showWarning(
            "Selection Error", 
            "Selected input capture device index is out of range."
        );
        return false;
    }

    if (outputIndex >= static_cast<int>(m_audioDevices.playbackDeviceCount)) {
        this->showWarning("Selection Error", "Selected output device index is out of range.");
        return false;
    }

    ResultVoid result = AudioRedirector::StartDuplexRedirect(
        &m_audioDevices.captureDeviceInfos[inputIndex].id,
        &m_audioDevices.playbackDeviceInfos[outputIndex].id
    );

    if (!result.has_value()) {
        this->showWarning(
            "Error starting capture audio redirect",
            QString::fromStdString(result.error().str())
        );
        return false;
    } 
    return true;
}

void MainWindow::restartLoopbackRedirect() {
    if (m_loopbackUI.startButton->text() == "Stop") {
        m_loopbackUI.startButton->click(); // Stop
        m_loopbackUI.startButton->click(); // Start
    }
}

void MainWindow::restartCaptureRedirect() {
    if (m_captureUI.startButton->text() == "Stop") {
        m_captureUI.startButton->click(); // Stop
        m_captureUI.startButton->click(); // Start
    }
}
