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
#define MAXIEXT 3

using namespace tinyxml2;

struct Pair
{
	char source[256];
	char dest[256];
};

int parseData(XMLDocument*, Pair[], int, char*[]);
int copyFile(char[], char[], int, char*[]);
int copyDir(char[], char[], int, char*[]);
void constToChar(const char*, char*);
int checkExtension(char*, char*[]);

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
	char* extArray[MAXIEXT];
	Pair pairsList[pairNum];//q = {std::string(""),  std::string("")};
	parseData(data, pairsList, pairNum, extArray);	//Parses XML file for path values and other settings
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
			if(copyDir(pairsList[k].source, pairsList[k].dest, bufLen, extArray) == 1)
			{
				printf("ERROR: copyDir failed on source %s\n", pairsList[pairNum].source);
				return 1;
			}
		}
		else if(S_ISREG(sourceStat.st_mode) && S_ISREG(destStat.st_mode))	//Source and dest are regular files
		{
			if(copyFile(pairsList[k].source, pairsList[k].dest, bufLen, extArray) == -1)
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

	char testing[] = "malware.exe";

	
	return 0;
}

int parseData(XMLDocument* data, Pair pList[], int pNum, char* extArray[])
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
		const char *constExtList = pairElem->FirstChildElement("iext")->GetText();//List of ignored exts in one string
		
		if (constExtList != NULL && constExtList != "")
		{
			char *extList = (char*)malloc(sizeof(constExtList)+1);				//Same list but non-const
			constToChar(constExtList, extList);
			int count = 0;
			//char *extArray[MAXIEXT];											//Separated list of exts
			char *buffer;
			buffer = strtok(extList, ";");
			while(buffer != NULL && count < MAXIEXT)
			{
				extArray[count] = buffer;
				printf("BUFFER: %s\n", buffer);
				buffer = strtok(NULL, ";");
				++count;
			}
			free(extList);

		}



	}
	return 0;
}

int copyFile(char src[], char dst[], int LEN, char* exts[])
{
	if (checkExtension(src, exts) == 1)
	{
		printf("File skipped because of its extension.\n");
		return 0;
	}
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

int copyDir(char src[], char dst[], int LEN, char* exts[])
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
				if (newDest[strlen(newDest)-1] != '/')
				{
					strncat(newDest, "/", 1);
				}
				strncat(newDest, ent->d_name, 255-strlen(newDest));

				
				char newSource[256];		//Creates new source for recursive call
				strncpy(newSource, src, 255);
				printf("Directory at: %s\n", newSource);
				if (newSource[strlen(newDest)-1] != '/')
				{
					strncat(newSource, "/", 1);
				}
				strncat(newSource, ent->d_name, 255-strlen(newSource));

				
				printf("Directory at: %s\n", newSource);

				struct stat statInfo;

				if (stat(newDest, &statInfo) == -1)	//Makes a dir if there isn't one yet
				{
					mkdir(newDest, 0777);
				}

				if(copyDir(newSource, newDest, LEN, exts) == 1)		//Recursive call
				{
					printf("ERROR: copyDir() failed (%s)\n", errno);
				}
			}
			else if (ent->d_type == DT_REG)
			{								//Checks if element is a regular file
				char newDest[256];	//Creates new destination for recursive call
				strncpy(newDest, dst, 255);
				if (newDest[strlen(newDest)-1] != '/')
				{
					strncat(newDest, "/", 1);
				}
				strncat(newDest, ent->d_name, 255-sizeof(newDest));

				char newSource[256];	//Creates new source for recursive call
				strncpy(newSource, src, 255);
				if (newSource[strlen(newDest)-1] != '/')
				{
					strncat(newSource, "/", 1);
				}
				strncat(newSource, ent->d_name, 255-sizeof(newSource));


				printf("File at: %s\n", newSource);

				if(copyFile(newSource, newDest, LEN, exts) == 1)	//Recursive call
				{
					printf("ERROR: copyFile() failed (%s)\n", errno);
				}
			}
		}
		printf("End of directory (%s)\n", src);
		closedir(dir);
		return 0;
	}
	else
	{
		printf("ERROR: opendir failed on source %s\n", src);
		closedir(dir);
		return 1;
	}
}

void constToChar(const char* source, char* destination)
{
	unsigned int srcLen = strlen(source);
	for (int l = 0; l < srcLen; ++l)
	{
		destination[l] = source[l];
	}
	destination[srcLen] = '\0';
}

int checkExtension(char* str, char* exts[])
{
	//Gets the extension or, if there is no extension, the file name. This may need patching later
	const int str_len = strlen(str)-1;
	char extension[str_len];
	int count = 0;
	char c;
	while(str[str_len - count] != '.' && count < strlen(str) && str[str_len - count] != '/')
	{
		c = str[str_len - count];
		extension[count] = c;	
		++count;
	}
	extension[count] = '\0';
	//Reverses extension
	int i, j;
	char tmp;
	i=j=tmp=0;
	j = strlen(extension)-1;
	for (int i = 0; i < j; ++i, --j)
	{
		tmp = extension[i];
		extension[i] = extension[j];
		extension[j] = tmp;
	}

	printf("Extension: \"%s\"\n", extension);
	for (int x = 0; x < MAXIEXT; ++x)
	{
		if (strcmp(exts[x], extension)==0)
		{
			return 1;
		}
	}
	return 0;
}









