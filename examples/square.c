#include "../include/ue_ipc.h"

int main(void)
{
    for (int i = 0; i < 4; ++i)
    {
        if (send_command("FORWARD") != 0)
        {
            return 1;
        }
        if (send_command("RIGHT") != 0)
        {
            return 1;
        }
    }

    return 0;
}
