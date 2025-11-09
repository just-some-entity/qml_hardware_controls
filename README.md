# QML Hardware Controls

A Qt/QML module to monitor and control hardware on Linux.

## Features

- Detects available **backlight** and **LED devices** under `/sys/class/backlight` and `/sys/class/leds`, and allows writing to them.

## Example Usage

```qml
import QtQuick

import HardwareControls

Window {
    id: window

    visible: true

    Component.onCompleted: {
        console.log("Hello world", BrightnessController.backlightAbsolute, BrightnessController.backlightPercent);
    }
}
```

## Installation

Clone the repository and run:
```
./install.sh
```

This will build and install the QML module for use in your projects globally at `/usr/lib/qt6/qml/HardwareControls`


## Uninstalling
run `./uninstall.sh` or simply delete the `/usr/lib/qt6/qml/HardwareControls` directory.