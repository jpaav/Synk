#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>

#pragma GCC diagnostic ignored "-Wwrite-strings"

struct Pairs
{
	char source[256];
	char dest[256];
};

Pairs parseData();
int copyFile(char[], char[]);
int copyDir(char[], char[]);

int main(int argc, char const *argv[])
{
	//FILE* f;
	int pairNum = 1;	//Zero means no init
	//Read Num of pairs from file
	//Read pairs from file
	Pairs exPair = {"/home/joseph/Documents/C_C++/Synk/Example_Source", "/home/joseph/Documents/C_C++/Synk/Example_Dest"};		//Example pair
	Pairs pairsList[pairNum];

	pairsList[0] = exPair;

	/*for (int i = 0; i < pairNum; ++i)
	{
		parseData();
	}*/
	struct stat pathStat;
	for (int k = 0; k < pairNum; ++k)
	{
		if(stat(pairsList[k].source, &pathStat) == -1)	//Gets source info
		{
			printf("ERROR: stat() failed (%s)\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		if(S_ISDIR(pathStat.st_mode))	//Checks if source is a directory
		{
			if(copyDir(pairsList[k].source, pairsList[k].dest) == 1)
			{
				printf("ERROR: copyDir failed on source %s\n", pairsList[pairNum].source);
				return 1;
			}
		}
		else	//Source is not a directory
		{
			if(copyFile(pairsList[k].source, pairsList[k].dest) == -1)
			{
				printf("ERROR: copyFile failed on source %s\n", pairsList[pairNum].source);
				return 1;
			}
		}
	}

	return 0;
}

Pairs parseData()
{
	struct Pairs pr;
	return pr;
}

int copyFile(char src[], char dst[])
{
	std::ifstream  srcStream;
    std::ofstream  dstStream;

	const int LEN=8192;		//4k buffer
	char buffer_out[LEN];
	char buffer_in[LEN];
	if (LEN) {
	srcStream.rdbuf()->pubsetbuf(buffer_in, LEN );
	dstStream.rdbuf()->pubsetbuf(buffer_out, LEN);
	} else {
	srcStream.rdbuf()->pubsetbuf(NULL, 0);
	dstStream.rdbuf()->pubsetbuf(NULL, 0);
	}
	srcStream.open(src, std::ios::in | std::ios::binary);
	dstStream.open(dst, std::ios::out | std::ios::binary);

	dstStream << srcStream.rdbuf();

	dstStream.close();
	srcStream.close();

	return 0;
}

int copyDir(char src[], char dst[])
{
	DIR *dir;
	struct dirent *ent;
	if((dir = opendir(src)) != NULL)
	{
		while((ent = readdir(dir)) != NULL)
		{
			if (ent->d_type == DT_DIR && strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
			{
				char newDest[256];
				strncpy(newDest, dst, 254);
				strncat(newDest, "/", 1);	//Will check if string already has "/" in the future
				strncat(newDest, ent->d_name, 255-sizeof(newDest));
				strncat(newDest, "\0", 1);


				char newSource[256];
				strncpy(newSource, src, 254);
				strncat(newSource, "/", 1);	//Will check if string already has "/" in the future
				strncat(newSource, ent->d_name, 255-sizeof(newSource));
				strncat(newSource, "\0", 1);
				
				printf("Directory at: %s\n", ent->d_name);

				struct stat *statInfo;

				if (stat(newDest, statInfo) == -1)
				{
					mkdir(newDest, 0777);
				}

				if(copyDir(newSource, newDest) == 1)
				{
					printf("ERROR: copyDir() failed (%s)\n", errno);
				}
			}
			else if (ent->d_type == DT_REG)
			{
				char newDest[256];
				strncpy(newDest, dst, 254);
				strncat(newDest, "/", 1);	//Will check if string already has "/" in the future
				strncat(newDest, ent->d_name, 255-sizeof(newDest));
				strncat(newDest, "\0", 1);

				char newSource[256];
				strncpy(newSource, src, 254);
				strncat(newSource, "/", 1);
				strncat(newSource, ent->d_name, 255-sizeof(newSource));
				strncat(newSource, "\0", 1);
`

				printf("File at: %s\n", newSource);

				if(copyFile(newSource, newDest) == 1)
				{
					printf("ERROR: copyFile() failed (%s)\n", errno);
				}
			}
		}
		printf("End of directory (%s)\n", src);
	}
	else
	{
		//set errno
		return 1;
	}



	return 0;
}