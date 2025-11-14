#pragma once
#include <QLabel>
#include <QPushButton>
#include <QComboBox>

#include "SmoothSlider.hpp"

struct MainUIState {
    QLabel *inputLabel;
    QComboBox *inputDropdown;
    QComboBox *outputDropdown;
    QComboBox *sampleRateDropdown;
    QComboBox *formatDropdown;
    QComboBox *volumeBoostDropdown;
    SmoothSlider *volumeSlider;
    QLabel *volumeLabel;
    QPushButton *startButton;
};

QWidget *CreateMainView(MainUIState &uiState);
