#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <time.h>
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
    int hour; /*0-24*/
    int minute;/*0-59*/
    int wasUsed;/*0-1*/
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
    int scheduledUpdate;
    int realtimeUpdate;
    int driveUpdate;

};


int pollLocs();
int pollDrives();
int pollTime(UpdateList*, FILE**);
int updateLocs();
int checkFifo(char *, char[MAX_BUF], int, FILE*);
int testList(UpdateList*, FILE*);
void initSettings(Settings *, int, int, int, int);
//void cleanup(void);

int main(void)
{
    
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
    FILE **fPtr = &f;
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
    //close(STDOUT_FILENO);
    close(STDERR_FILENO);

    struct Settings *settings = (Settings *)malloc(sizeof *settings);

    initSettings(settings, 1, 0, 0, 30);

    struct UpdateList *masterList = (UpdateList *)malloc(sizeof *masterList);

    if (testList(masterList, f) < 0)
    {
        fprintf(f, "Setup failure: testList() failed\n");
        exit(EXIT_FAILURE);
    }
    //atexit(cleanup);


    fprintf(f, "masterList.tAddr[0].wasUsed: %d (In 'main()')\n", masterList->tAddr[0].wasUsed);









    while (1)
    {
        int shouldUpdate = 0;
        //settings.scheduledUpdate = 1;
        if (settings->realtimeUpdate)
        {
            shouldUpdate = pollLocs();
        }
        fprintf(f, "settings.scheduledUpdate: %d\n", settings->scheduledUpdate);

        /*if (settings->scheduledUpdate)
        {
            /*shouldUpdate = pollTime(masterList, fPtr);    PollTime is giving some trouble so it is excluded from current execution

        }*/
        if (settings->driveUpdate)
        {
            shouldUpdate = pollDrives();
        }
        if (shouldUpdate)
        {
            updateLocs();
        }
        checkFifo(myfifo, buffer, fd, f);


        break; //testing only
        sleep(settings->updateTime);
    }

    /*Cleanup on exit*/
    fprintf(f, "Successfull exit\n*********\n");
    fclose(f);
    close(fd);
    exit(EXIT_SUCCESS);
}















int pollLocs()
{
    return 0;
}

int pollDrives()
{
    /*if a path on the list that previously didn't exist does now return its path to update*/
    return 0;
}

int pollTime(UpdateList* list, FILE** fLog)
{
    int returnVal = 0;
    time_t rawTime;
    struct tm *cTime;
    fputs("BEFORE ERROR STATEMENT\n", *fLog);
    time(&rawTime);
    fputs("after first fprintf", *fLog);
    cTime = localtime(&rawTime);            //This is the line causing all the problems
    fprintf(*fLog, "list.tAddr[0].wasUsed: %d (Inside of pollTime)\n", list->tAddr[0].wasUsed);
    fputs("after sencond fprintf", *fLog);
    TimeAddress *timesList = new TimeAddress[list->tSize];
    int arraySize = list->tSize;
    memcpy(timesList, list->tAddr, sizeof(*timesList)); //Copies from masterList to times list

    fprintf(*fLog, "timesList[0].wasUsed: %d\n", timesList[0].wasUsed);

    for (int i = 0; i < 1/*arraySize*/; ++i)/*Remember to set this to loop through all of the list in the future*/
    {
        if (timesList[i].hour >= cTime->tm_hour && timesList[i].minute >= cTime->tm_min && timesList[i].wasUsed == 0)
        {
            timesList[i].wasUsed = 1;
            fprintf(*fLog, "Updating Locations: its %d:%d\n", timesList[i].hour, timesList[i].minute);
            returnVal = 1;
        }
        else
        {
            fprintf(*fLog, "Time comparison failed.\ntimesList[%d].hour:\t%d\ncTime->tm_hour:\t\t%d\ntimesList[%d].minute:\t%d\ncTime->tm_min:\t\t%d\ntimesList[%d].wasUsed:\t%d\n", i, timesList[i].hour, cTime->tm_hour, i, timesList[i].minute, cTime->tm_min, i, timesList[i].wasUsed);
        }
    }
    return returnVal;
}

int updateLocs(){ return 0; }

int checkFifo(char *fifoPath, char buf[MAX_BUF], int fd, FILE* fLog)
{
    /* open, read, and display the message from the FIFO */
    fd = open(fifoPath, O_RDWR);
    if (fd < 0)
    {
        fprintf(fLog, "checkFifo(): open() failed\n");
        return 0;
    }
    if (read(fd, buf, MAX_BUF) < 0)
    {
        fprintf(fLog, "checkFifo(): read() failed\n");
        return 0;
    }
    fprintf(fLog, "Received: %s\n", buf);
    return 1;
}


int testList(UpdateList* list, FILE* eff)
{
    time_t curTime = time(NULL);
    struct tm *localTime;
    localTime = localtime(&curTime);

    Address testAddr;
    TimeAddress testtAddr;

    testtAddr.addr = testAddr;

    testtAddr.hour = localTime->tm_hour;
    testtAddr.minute = localTime->tm_min;
    testtAddr.wasUsed = 0;
    testtAddr.minute = testtAddr.minute + 1; /*This is used for testing pollTime()*/
    // fprintf(eff, "testtAddr.wasUsed: %d\n", testtAddr.wasUsed);

    list->tAddr[0] = testtAddr;
    // fprintf(eff, "list.tAddr[0].wasUsed: %d\n", list.tAddr[0].wasUsed);
    return 1;
}

void initSettings(Settings *sett, int sUp, int dUp, int rUp, int upRate)
{
    sett -> scheduledUpdate = sUp;
    sett -> driveUpdate = dUp;
    sett -> realtimeUpdate = rUp;
    sett -> updateTime = upRate;
}

/*void cleanup(void)
{
    fprintf(f, "Successfull exit\n*********\n");
    fclose(f);
    close(fd);
}*/