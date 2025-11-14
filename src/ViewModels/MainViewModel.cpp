#include "MainViewModel.hpp"
#include "MAConvert.hpp"
#include "Utils.hpp"

MainViewModel::MainViewModel(
    const MainUIState &loopbackUIState,
    const MainUIState &captureUIState,
    QObject *parent
) : QObject(parent),
    m_loopbackUIState(loopbackUIState),
    m_captureUIState(captureUIState)
{
    ResultVoid result = AudioRedirector::Initialize();

	if (!result.has_value()) {
		this->errorOccurred(
            "Failed to initialize audio redirector",
            QString::fromStdString(result.error().str())
		);
	}
}

MainViewModel::~MainViewModel() {
    AudioRedirector::Uninitialize();
}

void MainViewModel::loadDevices() {
    auto result = AudioRedirector::GetAudioDevices();

    if (result.has_value()) {
        m_audioDevices = result.value();
    } else {
        this->errorOccurred(
            "Failed to get audio devices",
            QString::fromStdString(result.error().str())
        );
    }

    this->populateDropdowns();      // populate dropdowns
    this->setDefaults();            // setup config defaults
    this->connectCaptureSignals();  // connect signals and slots
    this->connectLoopbackSignals(); // connect signals and slots
}

void MainViewModel::populateDropdowns() {
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
        m_captureUIState.inputDropdown->addItem(icon, name);
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
        m_loopbackUIState.inputDropdown->addItem(icon, name);
        m_loopbackUIState.outputDropdown->addItem(icon, name);
        m_captureUIState.outputDropdown->addItem(icon, name);
    }

    m_captureUIState.inputDropdown->setCurrentIndex(defaultCaptureIndex);
    m_captureUIState.outputDropdown->setCurrentIndex(defaultPlaybackIndex);
    m_loopbackUIState.inputDropdown->setCurrentIndex(defaultPlaybackIndex);

    // -----------------------------------
    // Populate formats and sample rates
    // -----------------------------------

    for (const ma_format &format : AudioRedirector::Formats) {
        const QString fmt = QString::fromStdString(ma::convert::to_string(format));
        m_loopbackUIState.formatDropdown->addItem(fmt);
        m_captureUIState.formatDropdown->addItem(fmt);
    }

    for (const ma_uint32 &sampleRate : AudioRedirector::SampleRates) {
        const QString rate = QStringLiteral("%1 Hz").arg(sampleRate);
        m_loopbackUIState.sampleRateDropdown->addItem(rate);
        m_captureUIState.sampleRateDropdown->addItem(rate);
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

    m_loopbackUIState.volumeBoostDropdown->addItems(items);
    m_captureUIState.volumeBoostDropdown->addItems(items);
}

void MainViewModel::setDefaults() {
    const QString loopbackFormat = QString::fromStdString(ma::convert::to_string(AudioRedirector::GetLoopbackFormat()));
    const QString loopbackSampleRate = QStringLiteral("%1 Hz").arg(AudioRedirector::GetLoopbackSampleRate());

    const QString duplexFormat = QString::fromStdString(ma::convert::to_string(AudioRedirector::GetDuplexFormat()));
    const QString duplexSampleRate = QStringLiteral("%1 Hz").arg(AudioRedirector::GetDuplexSampleRate());

    m_loopbackUIState.formatDropdown->setCurrentText(loopbackFormat);
    m_loopbackUIState.sampleRateDropdown->setCurrentText(loopbackSampleRate);

    m_captureUIState.formatDropdown->setCurrentText(duplexFormat);
    m_captureUIState.sampleRateDropdown->setCurrentText(duplexSampleRate);
}

void MainViewModel::connectLoopbackSignals() {
    connect(m_loopbackUIState.inputDropdown, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index >= 0) this->restartLoopbackRedirect();
    });

    connect(m_loopbackUIState.outputDropdown, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index >= 0) this->restartLoopbackRedirect();
    });

    connect(m_loopbackUIState.formatDropdown, &QComboBox::currentTextChanged, this, [this](const QString &text) {
        std::optional<ma_format> formatOpt = ma::convert::to_format(text.toStdString());
        if (!formatOpt.has_value()) {
            return this->errorOccurred(
                "Unknown Format String",
                QStringLiteral("The selected format (%1) is not recognized.").arg(text)
            );
        }

        AudioRedirector::SetLoopbackFormat(formatOpt.value());
        this->restartLoopbackRedirect();  // Restart redirect if running to apply new format
    });

    connect(m_loopbackUIState.sampleRateDropdown, &QComboBox::currentTextChanged, this, [this](const QString &text) {
        bool ok = false;
        QString str = text;
        int sampleRate = str.replace(" Hz", "").toInt(&ok);
        if (!ok) {
            return this->errorOccurred(
                "Conversion Error: Invalid Sample Rate",
                QStringLiteral("Could not convert string (%1) to number.").arg(text)
            );
        }

        AudioRedirector::SetLoopbackSampleRate(sampleRate);
        this->restartLoopbackRedirect();  // Restart redirect if running to apply new sample rate
    });

    connect(m_loopbackUIState.volumeBoostDropdown, &QComboBox::currentIndexChanged, this, [this](int index) {
        m_loopbackUIState.volumeSlider->setRange(0, 100 * (index + 1));
    });

    connect(m_loopbackUIState.volumeSlider, &QSlider::valueChanged, this, [this](int value) {
        ma_result result = AudioRedirector::SetPlaybackVolume((value / 100.0f) + 0.1);

        if (result == MA_SUCCESS) {
            m_loopbackUIState.volumeLabel->setText(QString("%1%").arg(value));
        } else {
            this->errorOccurred(
                "Volume Error",
                QStringLiteral("Failed to set output volume (%1).").arg(ma::convert::to_string(result))
            );
        }
    });

    connect(m_loopbackUIState.startButton, &QPushButton::clicked, this, [this]() {
        if (m_loopbackUIState.startButton->text() == "Start") {
            if (this->startLoopbackRedirect()) {
                m_loopbackUIState.startButton->setText("Stop");
            }
        } else {
            ResultVoid result = AudioRedirector::StopLoopbackRedirect();

            if (result.has_value()) {
                m_loopbackUIState.startButton->setText("Start");
            } else {
                this->errorOccurred(
                    "Error stopping loopback audio redirect",
                    QString::fromStdString(result.error().str())
                );
            }
        }
    });
}

void MainViewModel::connectCaptureSignals() {
    connect(m_captureUIState.inputDropdown, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index >= 0) this->restartCaptureRedirect();
    });

    connect(m_captureUIState.outputDropdown, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index >= 0) this->restartCaptureRedirect();
    });

    connect(m_captureUIState.formatDropdown, &QComboBox::currentTextChanged, this, [this](const QString &text) {
        std::optional<ma_format> formatOpt = ma::convert::to_format(text.toStdString());
        if (!formatOpt.has_value()) {
            return this->errorOccurred(
                "Unknown Format String",
                QStringLiteral("The selected format (%1) is not recognized.").arg(text)
            );
        }

        AudioRedirector::SetDuplexFormat(formatOpt.value());
        this->restartCaptureRedirect();  // Restart redirect if running to apply new format
    });

    connect(m_captureUIState.sampleRateDropdown, &QComboBox::currentTextChanged, this, [this](const QString &text) {
        bool ok = false;
        QString str = text;
        int sampleRate = str.replace(" Hz", "").toInt(&ok);
        if (!ok) {
            return this->errorOccurred(
                "Conversion Error: Invalid Sample Rate",
                QStringLiteral("Could not convert string (%1) to number.").arg(text)
            );
        }

        AudioRedirector::SetDuplexSampleRate(sampleRate);
        this->restartCaptureRedirect();  // Restart redirect if running to apply new sample rate
    });

    connect(m_captureUIState.volumeBoostDropdown, &QComboBox::currentIndexChanged, this, [this](int index) {
        m_captureUIState.volumeSlider->setRange(0, 100 * (index + 1));
    });

    connect(m_captureUIState.volumeSlider, &QSlider::valueChanged, this, [this](int value) {
        ma_result result = AudioRedirector::SetDuplexVolume((value / 100.0f) + 0.1);

        if (result == MA_SUCCESS) {
            m_captureUIState.volumeLabel->setText(QString("%1%").arg(value));
        } else {
            this->errorOccurred(
                "Volume Error",
                QStringLiteral("Failed to set output volume (%1).").arg(ma::convert::to_string(result))
            );
        }
    });

    connect(m_captureUIState.startButton, &QPushButton::clicked, this, [this]() {
        if (m_captureUIState.startButton->text() == "Start") {
            if (this->startCaptureRedirect()) {
                m_captureUIState.startButton->setText("Stop");
            }
        } else {
            ResultVoid result = AudioRedirector::StopDuplexRedirect();

            if (result.has_value()) {
                m_captureUIState.startButton->setText("Start");
            } else {
                this->errorOccurred(
                    "Error stopping capture audio redirect",
                    QString::fromStdString(result.error().str())
                );
            }
        }
    });
}

bool MainViewModel::startLoopbackRedirect() {
    const int inputIndex = m_loopbackUIState.inputDropdown->currentIndex();
    const int outputIndex = m_loopbackUIState.outputDropdown->currentIndex();

    if (inputIndex < 0 || outputIndex < 0) {
        this->errorOccurred(
            "Selection Error",
            QStringLiteral("Please select an %1 device").arg(
                inputIndex < 0 ? "input" : "output")
        );
        return false;
    }

    if (inputIndex >= static_cast<int>(m_audioDevices.playbackDeviceCount)) {
        this->errorOccurred(
            "Selection Error", 
            "Selected input loopback device index is out of range."
        );
        return false;
    }

    if (outputIndex >= static_cast<int>(m_audioDevices.playbackDeviceCount)) {
        this->errorOccurred("Selection Error", "Selected output device index is out of range.");
        return false;
    }

    ResultVoid result = AudioRedirector::StartLoopbackRedirect(
        &m_audioDevices.playbackDeviceInfos[inputIndex].id,
        &m_audioDevices.playbackDeviceInfos[outputIndex].id
    );

    if (!result.has_value()) {
        this->errorOccurred(
            "Error starting loopback audio redirect",
            QString::fromStdString(result.error().str())
        );
        return false;
    }
    return true;
}

bool MainViewModel::startCaptureRedirect() {
    const int inputIndex = m_captureUIState.inputDropdown->currentIndex();
    const int outputIndex = m_captureUIState.outputDropdown->currentIndex();

    if (inputIndex < 0 || outputIndex < 0) {
        this->errorOccurred(
            "Selection Error",
            QStringLiteral("Please select an %1 device").arg(
                inputIndex < 0 ? "input" : "output")
        );
        return false;
    }

    if (inputIndex >= static_cast<int>(m_audioDevices.captureDeviceCount)) {
        this->errorOccurred(
            "Selection Error", 
            "Selected input capture device index is out of range."
        );
        return false;
    }

    if (outputIndex >= static_cast<int>(m_audioDevices.playbackDeviceCount)) {
        this->errorOccurred("Selection Error", "Selected output device index is out of range.");
        return false;
    }

    ResultVoid result = AudioRedirector::StartDuplexRedirect(
        &m_audioDevices.captureDeviceInfos[inputIndex].id,
        &m_audioDevices.playbackDeviceInfos[outputIndex].id
    );

    if (!result.has_value()) {
        this->errorOccurred(
            "Error starting capture audio redirect",
            QString::fromStdString(result.error().str())
        );
        return false;
    } 
    return true;
}

void MainViewModel::restartLoopbackRedirect() {
    if (m_loopbackUIState.startButton->text() == "Stop") {
        m_loopbackUIState.startButton->click(); // Stop
        m_loopbackUIState.startButton->click(); // Start
    }
}

void MainViewModel::restartCaptureRedirect() {
    if (m_captureUIState.startButton->text() == "Stop") {
        m_captureUIState.startButton->click(); // Stop
        m_captureUIState.startButton->click(); // Start
    }
}
