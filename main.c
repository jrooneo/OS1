/*********************************************************
 * $Author: o-rooneo $
 * $Date: 2015/02/10 07:36:49 $
 * $Log: main.c,v $
 * Revision 1.7  2015/02/10 07:36:49  o-rooneo
 * Finished project with bugs. View readme for full listing
 *
 * Revision 1.6  2015/02/09 21:53:05  o-rooneo
 * it compiles! Added queue functionality
 *
 * Revision 1.5  2015/02/09 01:31:51  o-rooneo
 * Merged functions.c into the tail end main.c
 *
 * Revision 1.4  2015/02/05 16:06:39  o-rooneo
 * cleaned up debug messages. added case '?'
 *
 * Revision 1.3  2015/02/03 00:51:42  o-rooneo
 * Added keywords for RCS. Added flags and getopt
 *
 ********************************************************/

#include <stdio.h>	//perror, printf 
#include <stdlib.h>	//
#include <unistd.h>	//
#include <sys/stat.h>   //
#include <sys/queue.h>  //TAIL_QUEUE
#include <limits.h>     //PATH_MAX
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

TAILQ_HEAD(tailhead, entry) head;
struct tailhead *headPtr;

struct entry{ //struct to define the nodes used for the tail queue
	TAILQ_ENTRY(entry) entries;
	char path[PATH_MAX];
};

int sizePathFun(char *path);
int breadthFirstApply(char *path, int pathFun(char *path1));
char executable[15];
char error[PATH_MAX+30];
int flags[7]; //To keep includes lower using int instead of flag. Global to allow access from all funcs

int main(int argc, char **argv)
{
	strncpy(executable, argv[0], 15);
	char opt;
	int i;  
	float flagSize = 0;
	int size = 0;
	char path[PATH_MAX];
	for(i = 0; i<7; i++) flags[i]=0; //initialize flags to 0
	while((opt = getopt(argc,argv, "ashHkLx")) != -1){ //assign flag values
		switch(opt){ 
		case 'a':
			if(flags[1]){
				printf("Cannot set -a with -s\n");
				break;
			}	
			flags[0]=1;
			break;
		case 's':
			if(flags[0]){ 
				printf("Cannot set -s with -a\n");
				break;
			}
			flags[1]=1;
			break;
		case 'h':
			if(flags[3]){ 
				printf("Cannot set -h with -H\n");
				break;
			}
			//flags[2]=1;
			break;
		case 'H':
			if(flags[2]){ 
				printf("Cannot set -h with -H\n");
				break;
			}
			//flags[3]=1;
			break;
		case 'k':
			flags[4]=1;
			break;
		case 'L':
			flags[5]=1;
			break;
		case 'x':
			flags[6]=1;
			break;
		case '?':
		default:
			sprintf(error, "%s %s", executable,opt);
			perror(error);
			break;
		}
	} 
	/* Code to support only a single file parameter
		if(optind >= argc){
			strcpy(path,".");
			size = breadthFirstApply(path,sizePathFun);
			printf("%i\t%s\n",size,path);
		}else{
			strcpy(path,argv[optind]);
		}
	*/
	 //Probably unnecessary but du supports multiple files so I added the functionality
	 if(optind < argc){
		while(optind < argc){
			strcpy(path,argv[optind]);
				breadthFirstApply(path,sizePathFun);
			optind++;
		}
		return 0;
	}
	strcpy(path,".");
	breadthFirstApply(path,sizePathFun);		
	return 0;
}

int sizePathFun(char *path)
{
	struct stat statBuffer;
	if(stat(path, &statBuffer) == -1){
		sprintf(error, "%s %s", executable,path);
		perror(error);
		return -1;	
   	 }
	return statBuffer.st_blocks/2;
}

int breadthFirstApply(char *path, int pathFun(char *path1))
{
	struct dirent *direntPtr;
	struct stat statBuffer; 
	struct entry *holder, *tailPtr, *traversalPtr;
	DIR *currentDir;
	char fullPath[PATH_MAX];
	char tempPath[PATH_MAX];
	int size = 0;
	int sum = 0;
	int directorySize = 0;
	int classSize = 0;
	char sizeClass = 'B';
	float tempSize = 0;
	float flagValue = 0.0;
	TAILQ_INIT(&head);
	headPtr = &head;

	holder = (struct entry *) malloc(sizeof(struct entry));
	strcpy(holder->path, path);
	TAILQ_INSERT_TAIL(headPtr,holder,entries);

	while(!TAILQ_EMPTY(headPtr)){
		strcpy(tempPath, headPtr->tqh_first->path);
		tailPtr = headPtr->tqh_first; //Get the next item in the queue
		TAILQ_REMOVE(headPtr,headPtr->tqh_first,entries);
		free(tailPtr);

		directorySize = 0;

		if ((currentDir = opendir(tempPath)) == NULL) {
			if(headPtr->tqh_first){
				TAILQ_REMOVE(headPtr,headPtr->tqh_first,entries);
			}
			sprintf(error, "%s %s", executable,tempPath);
			perror(error);
			continue;
		}
		while(currentDir != NULL && (direntPtr = readdir(currentDir))){
			snprintf(fullPath, PATH_MAX, "%s/%s", tempPath, direntPtr->d_name);
			if(stat(fullPath, &statBuffer) == -1){
				printf("File not found");
				continue;
			}
			switch(statBuffer.st_mode & S_IFMT){
			case S_IFREG:
				size = pathFun(fullPath)/2;
				if(size > -1){
					if(flags[2]){
						while((tempSize / 1024.0) > 1){
							tempSize/=1024.0;
							classSize++;
						}
					}
					if(flags[3]){
						while((tempSize / 1000.0) > 1){
							tempSize/=1000.0;
							classSize++;
						}
					}
					if(!flags[2] || !flags[3]) directorySize += size;
					if(flags[0]){
						if(flags[2] || flags[3]){
							switch(classSize){
							case 1: sizeClass = 'K'; break;
							case 2: sizeClass = 'M'; break;
							}
							if(classSize > 2) sizeClass = 'G';
							printf("%i %c\t%s\n",tempSize,fullPath);
						}else{
							printf("%i\t%s\n",size,fullPath);
						}
					}
				}
				break;
			case S_IFDIR:
				if(strcmp(direntPtr->d_name, ".") && strcmp(direntPtr->d_name, "..")) {
					holder = (struct entry *)malloc(sizeof(struct entry));
					strcpy(holder->path, fullPath);
					TAILQ_INSERT_TAIL(headPtr, holder, entries);
				}
				break;
			case S_IFLNK:
				if(!flags[5]){
					if(lstat(fullPath, &statBuffer) == -1){
						sprintf(error, "%s %s", executable,fullPath);
						perror(error);
						continue;
					}
   	 			}
				size =  pathFun(fullPath);
			//Following will be treated as special files. No size printed.
			case S_IFBLK:
				printf("Block Device: %s\n", fullPath);
				break;
			case S_IFCHR:
				printf("Character Device: %s\n",path);
				break;
			case S_IFIFO:   
				printf("FIFO Pipe: %s\n",path);
				break;
			}	
		}
		if(!flags[1] || !flags[2] || !flags[3]){
			printf("%i\t %s \n", directorySize, tempPath);
		}
		if(flags[2] || flags[3]){
			printf("%i %c\t%s\n",tempSize,fullPath);
		}
		if(!flags[2] || !flags[3]){
			sum += directorySize;
		}
		closedir(currentDir);
	}
	return sum;
}
