#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define FIFO_PATH "/tmp/unity_ipc_character_commands"

static void send_command(const char *command)
{
    int fd = open(FIFO_PATH, O_WRONLY);
    if (fd == -1) {
        perror("open FIFO");
        exit(1);
    }

    dprintf(fd, "%s\n", command);
    close(fd);
    usleep(300000);
}

int main(void)
{
    if (mkfifo(FIFO_PATH, 0666) == -1) {
        /* Ignored: FIFO may already exist. */
    }

    send_command("RESET");

    for (int side = 0; side < 4; ++side) {
        send_command("FORWARD");
        send_command("FORWARD");
        send_command("RIGHT");
    }

    send_command("JUMP");
    return 0;
}
