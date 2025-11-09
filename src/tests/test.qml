import QtQuick

import BrightnessControllerP

Window
{
    width:  400
    height: 400

    visible: true

    Component.onCompleted: {

        for (const led of BrightnessController.leds)
        {
            console.log(led.id);
            console.log(led.current);
            console.log(led.max);
            console.log("");
        }

        // console.log(BrightnessController.backlight, BrightnessController.backlightMax);
    }
}