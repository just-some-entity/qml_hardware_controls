import QtQuick

import BrightnessControllerP

Window
{
    width:  400
    height: 400

    visible: true

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
}