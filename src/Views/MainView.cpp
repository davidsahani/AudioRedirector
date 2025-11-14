#include "MainView.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "Container.hpp"

QWidget *CreateMainView(MainUIState &s) {
    QVBoxLayout *layout = Layout<QVBoxLayout>(
        Container<QLabel>(
            s.inputLabel,
            APPLY_EX(setContentsMargins(4, 0, 0, 2))
        ),
        s.inputDropdown = new QComboBox(),
        Spacing(5),
        Container<QLabel>(
            new QLabel("Select Output Playback Device:"),
            APPLY_EX(setContentsMargins(4, 0, 0, 2))
        ),
        s.outputDropdown = new QComboBox(),
        Spacing(15),
        Layout<QVBoxLayout>(
            APPLY_EX(setContentsMargins(5, 0, 0, 0)),

            Layout<QHBoxLayout>(
                new QLabel("Sample Rate:"), 
                s.sampleRateDropdown = new QComboBox()
            ),
            Layout<QHBoxLayout>(
                new QLabel("Format:"), 
                s.formatDropdown = new QComboBox()
            ),
            Layout<QHBoxLayout>(
                new QLabel("Volume Boost:"),
                s.volumeBoostDropdown = new QComboBox()
            ),
            Spacing(15),
            Layout<QHBoxLayout>(
                new QLabel("Volume:"),
                Container<SmoothSlider>(
                    s.volumeSlider = new SmoothSlider(Qt::Horizontal),
                    APPLY_EX(setRange(0, 100), setValue(100), setSingleStep(2))
                ),
                s.volumeLabel = new QLabel("100%")
            )
        ),
        Spacing(15),
        Stretch(1),
        s.startButton = new QPushButton("Start")
    );

    layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(10, 5, 10, 10);
    return Container(new QWidget(), layout);
}
