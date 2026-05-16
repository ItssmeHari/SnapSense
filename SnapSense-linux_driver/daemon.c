// daemon.c — SnapSense Userspace Daemon


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define DEVICE_PATH   "/dev/snapsense"
#define READ_BUF_SIZE 32

static void print_timestamp(void)
{
    time_t     now = time(NULL);
    struct tm *t   = localtime(&now);
    printf("[%04d-%02d-%02d %02d:%02d:%02d] ",
           t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
           t->tm_hour, t->tm_min, t->tm_sec);
}

int main(void)
{
    int  fd;
    char buf[READ_BUF_SIZE];
    ssize_t n;

    printf("SnapSense Daemon starting...\n");
    printf("Reading from %s\n\n", DEVICE_PATH);

    fd = open(DEVICE_PATH, O_RDONLY);
    if (fd < 0) {
        perror("open /dev/snapsense failed");
        fprintf(stderr, "Is the driver loaded? Run: sudo insmod snapsense.ko\n");
        return EXIT_FAILURE;
    }

    while (1) {
        /* Seek back to start before each read */
        lseek(fd, 0, SEEK_SET);

        memset(buf, 0, sizeof(buf));
        n = read(fd, buf, sizeof(buf) - 1);

        if (n < 0) {
            perror("read failed");
            break;
        }

        /* Remove newline for clean output */
        if (n > 0 && buf[n - 1] == '\n') buf[n - 1] = '\0';

        print_timestamp();
        printf("Temperature: %s\n", buf);

        sleep(1);
    }

    close(fd);
    return EXIT_SUCCESS;
}
