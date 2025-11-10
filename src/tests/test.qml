import QtQuick
import QtQuick.Layouts

import HardwareManager

Window
{
    id: win
    width:  400
    height: 400

    visible: true

    CpuDataSampler
    {
        id: cpuData


    }

    Text
    {
        text: "cpu-name: " + cpuData.name
    }
}