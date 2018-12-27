#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include "filelist.h"

char *getPath(char *path, struct dirent *info)
{
    char *fullPath;
    int lenP=strlen(path);
    int lenN=strlen(info->d_name);
    fullPath=(char*)malloc(lenP+lenN+2);
    strcat(fullPath,path);
    if (path[lenP-1]!='/')
        strcat(fullPath,"/");
    strcat(fullPath,info->d_name);
    return fullPath;
}

char searchType(char *path)
{
    struct stat st;

    if (stat(path, &st)==-1)
    {
        return DT_UNKNOWN;
    }

    switch (st.st_mode & S_IFMT)
    {
        case S_IFDIR:  
            return DT_DIR;
        case S_IFREG:  
            return DT_REG;
        default:       
            return DT_UNKNOWN;
    }
}

char getFileType(char *path, struct dirent *info)
{
    char type;
    type=info->d_type;
    if (type==DT_UNKNOWN)
        return searchType(path);
    return type;
}

void search(char *path, FileList **fileList)
{
    DIR *dir;
    struct dirent *infoOfDir;
    
    dir = opendir (path);

    if (dir == NULL)
    {
        printf("No puedo abrir el directorio");
        exit(0);
    }
 
    while ((infoOfDir = readdir (dir)) != NULL)
    {
        if ((strcmp(infoOfDir->d_name, ".")!=0) && (strcmp(infoOfDir->d_name, "..")!=0))
        {
            char type;
            char *newPath;   
            newPath=getPath(path, infoOfDir);
            type=getFileType(newPath, infoOfDir);
            if (type==DT_REG)
            {
                add(createNode(infoOfDir,newPath),fileList); 
            }
            else if (type==DT_DIR)
            {
                search(newPath, fileList);   
            }
        }
    }
  //closedir (dir);
}


int isSameSize(FileList *p, FileList *q)
{
    return p->size==q->size;
}

void readFile(FileList *file)
{
    if(file->data==NULL)
    {
        file->data=(char*)malloc(file->size);
        read(file->fileDescriptor,file->data,file->size);
    }
}

int compare(FileList *p, FileList *q)
{
    if(isSameSize(p,q))
    {
        readFile(p);
        readFile(q);
        return strcmp(p->data,q->data)==0;
    }
    return 0;
}

int userInput(int max)
{
    char *s=(char*)malloc(sizeof(char)*2);
    scanf("%s",s);
    int n=atoi(s);
    if(n!=0&&n<=max)
        return n;
    if(strcmp(s,"s")==0||strcmp(s,"S")==0)
        return 0;
    printf("Por favor introduzca un número (1..%i) o letra (s) válido\n",max);
    return -1;
}

void deleteEqualsFiles(FileList **list)
{
    FileList *p = *list;
    while(p&&p->sig)
    {
        FileList *q = p;
        int n=1;
        FileList *dups=createNode(p->file,p->path);
        while(q&&q->sig)
        {
            if(compare(p,q->sig))
            {
                add(createNode(q->sig->file,q->sig->path),&dups);
                removeNode(&q);
                n++;
            }
            else
            {
                q=q->sig;
            }   
        }
        if(n>1)
        {
            show(dups);
            int input;
            while(n>1&&(input=userInput(n)))
            {
                if(input>0)
                {
                    n--;
                    removeByIndex(input-1, &dups);
                    printf("\nEl archivo (%i) fue eliminado\n", input);
                    if(n==1)
                    {
                        printf("\nEl archivo ya es único, todas las copias fueron resueltas\n");
                        break;
                    }
                }
                show(dups);
            }
        }
        p=p->sig;
    }
}

int main(int argc, char *argv[])
{
    FileList *fileList=NULL;
    printf("Buscando archivos repetidos en el directorio actual...: %s\n", ".");
    search(".",&fileList);
    deleteEqualsFiles(&fileList);
    return 0;
}