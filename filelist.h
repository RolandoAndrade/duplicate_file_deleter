struct FileList
{
    struct dirent *file;
    char *path;
    int fileDescriptor;
    char *data;
    int size;
    struct stat sstat;
    FileList *sig;
};

FileList * createNode(struct dirent *file, char *path)
{
    struct stat sstat;
    stat(path,&sstat);
    FileList * newFile = (FileList*)malloc(sizeof(FileList));
    newFile->file = file;
    newFile->sig = NULL;
    newFile->path=path;
    newFile->fileDescriptor=open(path, O_RDONLY);
    newFile->data=NULL;
    newFile->sstat=sstat;
    newFile->size=sstat.st_size;
    return newFile;
}

void add(FileList *file, FileList ** list)
{
    FileList * p = *list;
    if(!p)
    {
        *list=file;
    }
    else 
    {
        while (p->sig)
            p = p->sig;
        p->sig = file;
  }
}

void removeNode(FileList ** list)
{
    FileList *p= *list;
    if(p->sig)
    {
        FileList *t=p->sig;
        p->sig=p->sig->sig;
        free(t);
    }           
    else
        *list=NULL;

} 
void removeByIndex(int index,FileList ** list)
{
    FileList *p= *list;
    if(!index&&p)
    {
        *list=p->sig;
        remove(p->path);
        free(p);
    }
    while(index>1&&p&&p->sig)
    {
        p=p->sig;
        index--;
    }
    if(index==1&&p)
    {
        remove(p->sig->path);
        removeNode(&p);
    }

} 

void show(FileList *list)
{
    printf("\n\nArchivos duplicados:\n");
    int i=1;
    while(list)
    {
        printf("\t(%i) %s\n", i++,list->path);
        list=list->sig;
    }
    printf("\nPresione ");
    for(int j=1;j<i;j++)
    {
        printf("%i para eliminar (%i), ",j,j);
    }
    printf("s para siguiente: ");
} 