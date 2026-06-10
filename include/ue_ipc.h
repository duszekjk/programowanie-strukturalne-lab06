#ifndef UE_IPC_H
#define UE_IPC_H

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define UE_PIPE_PATH "/tmp/ue_character_commands"

static int send_command(const char *command)
{
    int fd = open(UE_PIPE_PATH, O_WRONLY);
    if (fd < 0)
    {
        perror("open");
        return -1;
    }

    if (write(fd, command, strlen(command)) < 0)
    {
        perror("write command");
        close(fd);
        return -1;
    }

    if (write(fd, "\n", 1) < 0)
    {
        perror("write newline");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

#endif
