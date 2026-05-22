/*
 * SnapSense Debian Serial Watch
 *
 * Reads distance text from STM32 UART.
 *
 * Build:
 *   gcc serial_watch.c -o serial_watch
 *
 * Run examples:
 *   ./serial_watch /dev/ttyUSB0
 *   ./serial_watch /dev/ttyACM0
 *
 * If permission error:
 *   sudo usermod -aG dialout $USER
 *   Then logout and login again.
 *
 * Temporary run:
 *   sudo ./serial_watch /dev/ttyUSB0
 */

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

static volatile int keep_running = 1;

static void handle_sigint(int sig)
{
    (void)sig;
    keep_running = 0;
}

static int setup_serial_port(const char *device)
{
    int fd = open(device, O_RDONLY | O_NOCTTY);

    if (fd < 0)
    {
        perror("open");
        return -1;
    }

    struct termios tty;

    if (tcgetattr(fd, &tty) != 0)
    {
        perror("tcgetattr");
        close(fd);
        return -1;
    }

    cfmakeraw(&tty);

    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~PARENB;          // no parity
    tty.c_cflag &= ~CSTOPB;          // 1 stop bit
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;              // 8 data bits
    tty.c_cflag &= ~CRTSCTS;         // no hardware flow control

    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        perror("tcsetattr");
        close(fd);
        return -1;
    }

    return fd;
}

int main(int argc, char *argv[])
{
    const char *device = "/dev/ttyUSB0";

    if (argc >= 2)
    {
        device = argv[1];
    }

    signal(SIGINT, handle_sigint);

    int fd = setup_serial_port(device);

    if (fd < 0)
    {
        return 1;
    }

    printf("Watching STM32 UART on %s at 115200 baud...\n", device);
    printf("Press Ctrl+C to stop.\n\n");

    char ch;
    char line[256];
    size_t index = 0;

    while (keep_running)
    {
        ssize_t n = read(fd, &ch, 1);

        if (n > 0)
        {
            if (ch == '\n')
            {
                line[index] = '\0';

                if (index > 0)
                {
                    printf("[STM32] %s\n", line);
                    fflush(stdout);
                }

                index = 0;
            }
            else if (ch != '\r')
            {
                if (index < sizeof(line) - 1)
                {
                    line[index++] = ch;
                }
                else
                {
                    index = 0;
                }
            }
        }
        else if (n < 0 && errno != EINTR)
        {
            perror("read");
            break;
        }
    }

    close(fd);
    printf("\nStopped.\n");
    return 0;
}
