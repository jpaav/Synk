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
#include <dirent.h>
#include <tinyxml2.h>

#pragma GCC diagnostic ignored "-Wwrite-strings"

using namespace tinyxml2;

struct Pair
{
	char source[256];
	char dest[256];
};

int parseData(XMLDocument*, Pair[], int);
int copyFile(char[], char[], int);
int copyDir(char[], char[], int);
void constToChar(const char*, char*);

int main(int argc, char const *argv[])
{
	int pairNum = -1;	//Zero means no init
	int bufLen = 1;
	XMLDocument* data = new XMLDocument();
	const char *xmlDocName = "data.xml";
	if (argc > 1)
	{
		xmlDocName = argv[1];
	}

	//NEED ERROR CHECKING FOR THIS LINE!
	//VVVVVVVVVVVVVVVVVVVVVVVVVV
	data->LoadFile( xmlDocName );
	XMLElement* pairsElem = data->FirstChildElement();
	pairsElem->QueryIntAttribute("num", &pairNum);	//Read Num of pairs from file
	pairsElem->QueryIntAttribute("buf", &bufLen);
	bufLen *= 1024;

	if (pairNum == -1)	//Makes sure pairNum was properly assigned and is not 0
	{
		printf("ERROR: pairNum failed to initialize from data.xml\n");
		exit(EXIT_FAILURE);
	}
	else if (pairNum == 0)
	{
		printf("pairNum is set to zero in data. XMLDocument\n");
		exit(EXIT_SUCCESS);
	}
	printf("pairNum: %d\n", pairNum);
	Pair pairsList[pairNum];//q = {std::string(""),  std::string("")};
	parseData(data, pairsList, pairNum);	//Parses XML file for path values and other settings
	delete data;


	struct stat sourceStat;	//used to extract info from stat
	struct stat destStat;
	for (int k = 0; k < pairNum; ++k)
	{
		int asdf = stat(pairsList[k].source, &sourceStat);
		int fdsa = stat(pairsList[k].dest, &destStat);
		if(asdf == -1 || fdsa == -1)	//Gets source/dest info
		{
			printf("ERROR: stat() failed on pair %d (%s)\n", k+1, strerror(errno));
			printf("Source: %s\n", pairsList[k].source);
			return 1;
		}
		if(S_ISDIR(sourceStat.st_mode) && S_ISDIR(destStat.st_mode))	//Checks if source/dest are directories
		{
			if(copyDir(pairsList[k].source, pairsList[k].dest, bufLen) == 1)
			{
				printf("ERROR: copyDir failed on source %s\n", pairsList[pairNum].source);
				return 1;
			}
		}
		else if(S_ISREG(sourceStat.st_mode) && S_ISREG(destStat.st_mode))	//Source and dest are regular files
		{
			if(copyFile(pairsList[k].source, pairsList[k].dest, bufLen) == -1)
			{
				printf("ERROR: copyFile failed on source %s\n", pairsList[pairNum].source);
				return 1;
			}
		}
		else	//Source and dest are not the same type of path
		{
			printf("Source and dest are not the same type of path or are unsupported types. Skipping pair %d.\n", k+1);
		}
	}
	return 0;
}

int parseData(XMLDocument* data, Pair pList[], int pNum)
{
	XMLElement* pairElem = data->FirstChildElement()->FirstChildElement();	//Navigates to "Pairs" -> first "pair" or its equilavalent
	for (int i = 0; i < pNum; ++i)
	{
		if (i > 0)
		{
		pairElem = pairElem->NextSiblingElement();
		}
		//Gets source and destination paths
		const char* xmlSrc = pairElem->FirstChildElement("src")->GetText();
		const char* xmlDst = pairElem->FirstChildElement("dst")->GetText();
		if (xmlSrc != NULL)
		{
			//pList[i].source = strdup(xmlSrc);
			//strcpy(xmlSrc, pList[i].source);
			unsigned int srcLen = strlen(xmlSrc);
			for (int l = 0; l < srcLen; ++l)
			{
				pList[i].source[l] = xmlSrc[l];
			}
			pList[i].source[srcLen] = '\0';
		}
		else
		{
			printf("XML source for pair %d is NULL. Skipping pair.\n", i+1);
			continue;
		}
		if (xmlDst != NULL)
		{
			//pList[i].dest = strdup(xmlDst);
			//strcpy(xmlDst, pList[i].dest);
			unsigned int dstLen = strlen(xmlDst);
			for (int l = 0; l < dstLen; ++l)
			{
				pList[i].dest[l] = xmlDst[l];
			}
			pList[i].dest[dstLen] = '\0';
		}
		else
		{
			printf("XML destination for pair %d is NULL Skipping pair.\n", i+1);
		}
		//Gets list of ignored extensions
		//Check if pair has attribute	
		/*const char *constExtList = pairElem->FirstChildElement("iext")->GetText();//List of ignored exts in one string
		char *extList;
		if (constExtList != NULL && constExtList != "")
		{
			strcpy(extList, constExtList);
			//constToChar(constExtList, extList);
			}/*
			int count = 0;
			char *extArray[3];	//Separated list of exts
			char *buffer;
			buffer = strtok(extList, ";");
			while(buffer != NULL)
			{
				extArray[count] = buffer;
				buffer = strtok(NULL, ";");
				++count;
			}
		}
		*/


	}
	return 0;
}

/*char* strcat_copy(const char *str1, char *str2)
{
	int str1_len, str2_len;
	char *new_str;
	if (str1 == NULL || str2 == NULL)
	{
		printf("One of these strings is NULL!\n");
		return NULL;
	}
	
	str1_len = strlen(str1);
	str2_len = strlen(str2);

	new_str = malloc(str1_len + str2_len +1);
	memcpy(new_str, str1, str1_len);
	memcpy(new_str + str1_len, str2, str2_len);
	return new_str;

}*/

int copyFile(char src[], char dst[], int LEN)
{
	std::ifstream  srcStream;
    std::ofstream  dstStream;

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

int copyDir(char src[], char dst[], int LEN)
{
	DIR *dir;
	struct dirent *ent;
	if((dir = opendir(src)) != NULL)	//Opens source directory
	{
		while((ent = readdir(dir)) != NULL)		//Reads elements (files/dirs) from parent directory
		{
			if (ent->d_type == DT_DIR && strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
			{									//^^^Checks if element is a directory and not "."/".."^^^
				char newDest[256];		//Creates new destination for recursive call
				strncpy(newDest, dst, 255);
				if (newDest[-1] != '/')
				{
					strncat(newDest, "/", 1);
				}
				strncat(newDest, ent->d_name, 255-strlen(newDest));

				
				char newSource[256];		//Creates new source for recursive call
				strncpy(newSource, src, 255);
				printf("Directory at: %s\n", newSource);
				if (newSource[-1] != '/')
				{
					strncat(newSource, "/", 1);
				}
				strncat(newSource, ent->d_name, 255-strlen(newSource));

				
				printf("Directory at: %s\n", newSource);

				struct stat *statInfo;

				if (stat(newDest, statInfo) == -1)	//Makes a dir if there isn't one yet
				{
					mkdir(newDest, 0777);
				}

				if(copyDir(newSource, newDest, LEN) == 1)		//Recursive call
				{
					printf("ERROR: copyDir() failed (%s)\n", errno);
				}
			}
			else if (ent->d_type == DT_REG)
			{								//Checks if element is a regular file
				char newDest[256];	//Creates new destination for recursive call
				strncpy(newDest, dst, 255);
				if (newDest[-1] != '/')
				{
					strncat(newDest, "/", 1);
				}
				strncat(newDest, ent->d_name, 255-sizeof(newDest));

				char newSource[256];	//Creates new source for recursive call
				strncpy(newSource, src, 255);
				if (newSource[-1] != '/')
				{
					strncat(newSource, "/", 1);
				}
				strncat(newSource, ent->d_name, 255-sizeof(newSource));


				printf("File at: %s\n", newSource);

				if(copyFile(newSource, newDest, LEN) == 1)	//Recursive call
				{
					printf("ERROR: copyFile() failed (%s)\n", errno);
				}
			}
		}
		printf("End of directory (%s)\n", src);
		return 0;
	}
	else
	{
		printf("ERROR: opendir failed on source %s\n", src);
		return 1;
	}
}
void constToChar(const char* source, char* destination)
{
	unsigned int srcLen = strlen(source);
	for (int l = 0; l < srcLen; ++l)
	{
		printf("%d\n", l);
		destination[l] = source[l];
	}
	destination[srcLen] = '\0';
}












