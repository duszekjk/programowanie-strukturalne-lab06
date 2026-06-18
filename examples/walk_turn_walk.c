#include "ue_ipc.h"

#include <stdio.h>
#include <unistd.h>

static int send_or_fail(const char *command)
{
    printf("send: %s\n", command);
    if (send_command(command) != 0)
    {
        fprintf(stderr, "failed to send command: %s\n", command);
        return 1;
    }
    return 0;
}

int main(void)
{
    printf("Walk, turn, walk demo for IpcCharacterWorld.\n");
    printf("Make sure the packaged Linux game is already running.\n");
    printf("FIFO path: %s\n", UE_PIPE_PATH);

    if (send_or_fail("RESET"))
    {
        return 1;
    }
    sleep(1);

    for (int i = 0; i < 8; ++i)
    {
        if (send_or_fail("FORWARD"))
        {
            return 1;
        }
        usleep(250000);
    }

    if (send_or_fail("RIGHT"))
    {
        return 1;
    }
    sleep(1);

    for (int i = 0; i < 8; ++i)
    {
        if (send_or_fail("FORWARD"))
        {
            return 1;
        }
        usleep(250000);
    }

    if (send_or_fail("STOP"))
    {
        return 1;
    }

    printf("Done.\n");
    return 0;
}
