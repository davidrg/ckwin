/*
    static key array used to process keystrokes in KClient
*/

#include "kuidef.h"

UINT keyArray[] = { 0                // 0x00
                  , 0                // 0x01
                  , 0                // 0x02
                  , 0                // 0x03
                  , 0                // 0x04
                  , 0                // 0x05
                  , 0                // 0x06
                  , 0                // 0x07
                  , 0//VK_BACK          // 0x08
                  , 0//VK_TAB           // 0x09
                  , 0                // 0x0A
                  , 0                // 0x0B
                  , VK_CLEAR         // 0x0C
                  , VK_RETURN        // 0x0D
                  , 0                // 0x0E
                  , 0                // 0x0F
                  , 0                // 0x10
                  , 0                // 0x11
                  , 0                // 0x12
                  , VK_PAUSE         // 0x13
                  , VK_CAPITAL       // 0x14
                  , 0                // 0x15
                  , 0                // 0x16
                  , 0                // 0x17
                  , 0                // 0x18
                  , 0                // 0x19
                  , 0                // 0x1A
                  , 0//VK_ESCAPE        // 0x1B
                  , 0                // 0x1C
                  , 0                // 0x1D
                  , 0                // 0x1E
                  , 0                // 0x1F
                  , 0 //VK_SPACE         // 0x20
                  , VK_PRIOR         // 0x21
                  , VK_NEXT          // 0x22
                  , VK_END           // 0x23
                  , VK_HOME          // 0x24
                  , VK_LEFT          // 0x25
                  , VK_UP            // 0x26
                  , VK_RIGHT         // 0x27
                  , VK_DOWN          // 0x28
                  , VK_SELECT        // 0x29
                  , VK_PRINT         // 0x2A
                  , VK_EXECUTE       // 0x2B
                  , VK_SNAPSHOT      // 0x2C
                  , VK_INSERT        // 0x2D
                  , VK_DELETE        // 0x2E
                  , VK_HELP          // 0x2F
                  , 0                // 0x30
                  , 0                // 0x31
                  , 0                // 0x32
                  , 0                // 0x33
                  , 0                // 0x34
                  , 0                // 0x35
                  , 0                // 0x36
                  , 0                // 0x37
                  , 0                // 0x38
                  , 0                // 0x39
                  , 0                // 0x3A
                  , 0                // 0x3B
                  , 0                // 0x3C
                  , 0                // 0x3D
                  , 0                // 0x3E
                  , 0                // 0x3F
                  , 0                // 0x40
                  , 0                // 0x41
                  , 0                // 0x42
                  , 0                // 0x43
                  , 0                // 0x44
                  , 0                // 0x45
                  , 0                // 0x46
                  , 0                // 0x47
                  , 0                // 0x48
                  , 0                // 0x49
                  , 0                // 0x4A
                  , 0                // 0x4B
                  , 0                // 0x4C
                  , 0                // 0x4D
                  , 0                // 0x4E
                  , 0                // 0x4F
                  , 0                // 0x50
                  , 0                // 0x51
                  , 0                // 0x52
                  , 0                // 0x53
                  , 0                // 0x54
                  , 0                // 0x55
                  , 0                // 0x56
                  , 0                // 0x57
                  , 0                // 0x58
                  , 0                // 0x59
                  , 0                // 0x5A
                  , VK_LWIN          // 0x5B
                  , VK_RWIN          // 0x5C
                  , VK_APPS          // 0x5D
                  , 0                // 0x5E
                  , 0                // 0x5F
                  , 0                // 0x60  VK_NUMPAD0
                  , 0                // 0x61  VK_NUMPAD1
                  , 0                // 0x62  VK_NUMPAD2
                  , 0                // 0x63  VK_NUMPAD3
                  , 0                // 0x64  VK_NUMPAD4
                  , 0                // 0x65  VK_NUMPAD5
                  , 0                // 0x66  VK_NUMPAD6
                  , 0                // 0x67  VK_NUMPAD7
                  , 0                // 0x68  VK_NUMPAD8
                  , 0                // 0x69  VK_NUMPAD9
                  , 0//VK_MULTIPLY      // 0x6A
                  , 0//VK_ADD           // 0x6B
                  , 0//VK_SEPARATOR     // 0x6C
                  , 0//VK_SUBTRACT      // 0x6D
                  , 0//VK_DECIMAL       // 0x6E
                  , 0//VK_DIVIDE        // 0x6F
                  , VK_F1            // 0x70
                  , VK_F2            // 0x71
                  , VK_F3            // 0x72
                  , VK_F4            // 0x73
                  , VK_F5            // 0x74
                  , VK_F6            // 0x75
                  , VK_F7            // 0x76
                  , VK_F8            // 0x77
                  , VK_F9            // 0x78
                  , VK_F10           // 0x79
                  , VK_F11           // 0x7A
                  , VK_F12           // 0x7B
                  , VK_F13           // 0x7C
                  , VK_F14           // 0x7D
                  , VK_F15           // 0x7E
                  , VK_F16           // 0x7F
                  , VK_F17           // 0x80
                  , VK_F18           // 0x81
                  , VK_F19           // 0x82
                  , VK_F20           // 0x83
                  , VK_F21           // 0x84
                  , VK_F22           // 0x85
                  , VK_F23           // 0x86
                  , VK_F24           // 0x87
                  , 0                // 0x88
                  , 0                // 0x89
                  , 0                // 0x8A
                  , 0                // 0x8B
                  , 0                // 0x8C
                  , 0                // 0x8D
                  , 0                // 0x8E
                  , 0                // 0x8F
                  , VK_NUMLOCK       // 0x90
                  , VK_SCROLL        // 0x91
};
