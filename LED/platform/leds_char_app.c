#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
 
/* firstdrvtest on
 *   * firstdrvtest off
 *     */
int main(int argc, char **argv)
{
    int fd;
    int val = 1;
    fd = open("/dev/platfor_driver_for_mini2440_led", O_RDWR);
    if (fd < 0)
    {
        printf("can't open platfor_driver_for_mini2440_led!\n");
    }
    if (argc != 2)
    {
        printf("Usage :\n");
        printf("%s <on|off>\n", argv[0]);
        return 0;
    }
 
    if (strcmp(argv[1], "on") == 0) {
        val  = 1;
    } else {
        val = 0;
    }
 
    write(fd, &val, 4);
    return 0;
}
