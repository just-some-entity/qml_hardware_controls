# QML Hardware Controls

A Qt/QML module to monitor and control hardware on Linux.

## Current Features

- Detects available **backlight** and **LED devices** under `/sys/class/backlight` and `/sys/class/leds`, and allows writing to them.

## Planned Features

- Memory Stats
- CPU Usage and Temperature Monitoring
- System Uptime and Load Average
- Network Information (Wi-Fi, Ethernet, Signal Strength)
- Battery and Power Management
- Storage Usage and Disk I/O
- GPU Information and Load
- Fan Speed and Thermal Sensors (Depending on difficulty)
- Audio Devices and Volume Control (Maybe)
- Display Configuration and Brightness Profiles (Maybe)

## Currently Available Interfaces


### BrightnessController
| Property | Type | Access | Description |
|-----------|------|---------|-------------|
| `updateDelay` | `int` | Read/Write | Delay between brightness updates (ms). |
| `backlight` | `qreal` | Read/Write | Current backlight value. |
| `backlightNormalized` | `qreal` | Read/Write | Backlight value normalized between 0 and 1. |
| `backlightMax` | `qreal` | Read-only | Maximum backlight value. |
| `backlights` | `QList<BrightnessEntry*>` | Read-only | List of available backlight devices. |
| `leds` | `QList<BrightnessEntry*>` | Read-only | List of available LED devices. |


### BrightnessEntry

| Property | Type | Access | Description |
|-----------|------|---------|-------------|
| `current` | `qreal` | Read/Write | Current brightness value. |
| `currentNormalized` | `qreal` | Read/Write | Brightness value normalized between 0 and 1. |
| `max` | `qreal` | Read-only | Maximum brightness value supported by the device. |
| `id` | `QString` | Read-only | Unique identifier of the device. |


## Example Usage

```qml
import HardwareControls
 
 ...
Text {
    text: "backlight level:" + BrightnessController.backlight
}

Component.onCompleted: {

    console.log("backlights");
    console.log("----------");
    for (const backlight of BrightnessController.backlights)
    {
        console.log(led.id);
        console.log(led.current);
        console.log(led.max);
        console.log("");
    }

    console.log("leds");
    console.log("----------");
    for (const led of BrightnessController.leds)
    {
        console.log(led.id);
        console.log(led.current);
        console.log(led.max);
        console.log("");

        led.current = 10;
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