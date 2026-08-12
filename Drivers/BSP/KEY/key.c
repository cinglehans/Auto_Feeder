#include "key.h"

//keyup 前一刻无按键被按下，可进行按键检测
uint8_t Key_Scan ( uint8_t mode )
{
    static uint8_t Keyup = 1;
    uint8_t Keyval = 0;

    if (mode == 1)
        Keyup = 1;

    if (Keyup && (Key_Up == 1 || Key0 == 0))
    {
        HAL_Delay(30);
        Keyup = 0;

        if (Key_Up == 1)
            Keyval = Keywk_press;
        if ( Key0 == 0)
            Keyval = Key0_press;

    } else if (Key_Up == 0 &&Key0 == 1)
    {
        Keyup = 1;
    }
    return Keyval;
}
