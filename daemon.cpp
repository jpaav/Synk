#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#define MAX_BUF 1024

struct Address
{
    char* site1;
    char* site2;
    int site1Type;           /*0 for file, 1 for folder*/
    int site2Type;          /*0 for file, 1 for folder*/
};
struct TimeAddress
{
    Address addr;
    int time; /*Find a better type to use here*/
};
struct DriveAddress
{
    Address addr;
    int accessable;
};
struct UpdateList
{
    TimeAddress tAddr[];
    int tSize;
    DriveAddress dAddr[];
    int dSize;
    Address rAddr[];
    int rSize;
};
struct Settings
{
    int updateTime; /*Should I make this into a float?*/
};


int pollLocs();
int pollDrives();
int pollTime();
int updateLocs();
int checkFifo(char *, char[MAX_BUF], int, FILE*);
Settings defaultSettings(Settings);
//void cleanup(void);

int main(void)
{
    int scheduledUpdate = 0, realtimeUpdate = 0, driveUpdate = 0;
    struct Settings settings;
    pid_t pid, sid;

    int fd;
    char * myfifo = "/tmp/synkFifo";
    char buffer[MAX_BUF];


    /* Fork off the parent process */       
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    /* If we got a good PID, then
       we can exit the parent process. */
    if (pid > 0) {
        printf("process_id of child process %d \n", pid);
        exit(EXIT_SUCCESS);
    }
    /* Change the file mode mask */
    umask(0);

    /*Open log files here*/
    FILE *f = fopen("/home/joseph/Documents/C_C++/Synk/daemonlog.txt", "w");
    if (f == NULL)
    {
        printf("Error opening file!\n");
        exit(EXIT_FAILURE);
    }
    fprintf(f, "File start;\n");

    /*Sets sid*/
    sid = setsid();
    if (sid < 0)
    {
        fprintf(f, "Setup failure: setsid() failed\n");
        exit(EXIT_FAILURE);
    }

    /*Sets working dir*/
    if ((chdir("/")) < 0)
    {
        fprintf(f, "Setup failure: chdir() failed\n");
        exit(EXIT_FAILURE);
    }

    /* Close out the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    settings = defaultSettings(settings);
    //atexit(cleanup);


    while (1)
    {
        int shouldUpdate = 0;
        if (realtimeUpdate)
        {
            shouldUpdate = pollLocs();
        }
        if (scheduledUpdate)
        {
            shouldUpdate = pollTime();
        }
        if (driveUpdate)
        {
            shouldUpdate = pollDrives();
        }
        if (shouldUpdate)
        {
            updateLocs();
        }
        fprintf(f, "THIS IS A TEST\n");
        checkFifo(myfifo, buffer, fd, f);

        break; /*testing only*/
        sleep(settings.updateTime);
    }

    /*Cleanup on exit*/
    fprintf(f, "Successfull exit\n*********\n");
    fclose(f);
    close(fd);
    exit(EXIT_SUCCESS);
}

int pollLocs()
{
    /*if no changes*/
    return 0;
    /*if changes*/
    return 1;
}

int pollDrives()
{
    /*if a path on the list that previously didn't exist does now return its path to update*/
}

int pollTime(){}

int updateLocs(){}

int checkFifo(char *fifoPath, char buf[MAX_BUF], int fd, FILE* log)
{
    /* open, read, and display the message from the FIFO */
    fd = open(fifoPath, O_RDWR);
    if (fd < 0)
    {
        fprintf(log, "checkFifo(): open() failed\n");
    }
    if (read(fd, buf, MAX_BUF) < 0)
    {
        fprintf(log, "checkFifo(): read() failed\n");
    }
    fprintf(log, "Received: %s\n", buf);
}

Settings defaultSettings(Settings sett)
{
    sett.updateTime = 30;
}

/*void cleanup(void)
{
    fprintf(f, "Successfull exit\n*********\n");
    fclose(f);
    close(fd);
}*/