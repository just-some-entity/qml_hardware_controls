import QtQuick
import QtQuick.Layouts

import HardwareManager

Window
{
    id: win
    width:  400
    height: 400

    visible: true

    Column
    {
        anchors.fill: parent

        Repeater
        {
            model: CpuMonitor.cpus
            delegate: Rectangle {

                width:  win.width
                height: win.height

                Column
                {
                    anchors.fill: parent

                    Repeater
                    {
                        model: modelData.cores

                        delegate: Rectangle {

                            width:  win.width
                            height: 60

                            Text
                            {
                                text: "core-frequency: " + modelData.frequencyMin + " : " + modelData.frequencyMax + " : " + modelData.frequencyCurrent
                            }
                        }
                    }
                }
            }
        }
    }

    Component.onCompleted: {

        /*
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
        */
    }
}