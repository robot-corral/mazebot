using System;

namespace line_sensor.data_collector.ui.ui_component
{
    [Flags]
    public enum UiComponent : uint
    {
        NONE                 = 0x00000000,

        ALL_BLE_DEVICES      = 0x00000001,
        ALL_SERIAL_DEVICES   = 0x00000002,
        POSITION_CONTROLLER  = 0x00000004,
        WIRELESS_LINE_SENSOR = 0x00000008,
    }
}
