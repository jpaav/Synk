#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
    int fd;

    /*Log file initialization
    FILE* f = fopen("/home/joseph/Documents/C_C++/Synk/guilog.txt", "w");
    if (f == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }*/

    char* myfifo = "/tmp/synkFifo";

    /* create the FIFO (named pipe) */
    if (mkfifo(myfifo, 0666) < 0)
    {
        //fprintf(f, "Failed: mkfifo() raised an error.\n");
    }

    /* write "Hi" to the FIFO */
    fd = open(myfifo, O_WRONLY);
    write(fd, "Hi", sizeof("Hi"));
    close(fd);

    /* remove the FIFO */
    unlink(myfifo);

    return 0;
}