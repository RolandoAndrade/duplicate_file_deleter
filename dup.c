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

void remove(FileList ** list)
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

void show(FileList *list)
{
    printf("Archivos encontrados\n");
    while(list)
    {
        printf("%s\n", list->path);
        list=list->sig;
    }
} 

int isADirectory(char *dir)
{
    struct stat statbuf;
    if (stat(dir,&statbuf) ==-1) {
        fprintf(stderr,"No se puede obtener la información del archivo %s:%s\n", dir, strerror(errno));
        exit(1);
    }
    if(strcmp(dir,".")==0||strcmp(dir,"..")==0)
        return 0;
    return statbuf.st_mode & S_IFDIR;

}
/*
*/






void error(const char *s)
{
  /* perror() devuelve la cadena S y el error (en cadena de caracteres) que tenga errno */
  perror (s);
  exit(EXIT_FAILURE);
}

char *getFullName(char *ruta, struct dirent *ent)
{
  char *nombrecompleto;
  int tmp;

  tmp=strlen(ruta);
  nombrecompleto=(char*)malloc(tmp+strlen(ent->d_name)+2); /* Sumamos 2, por el \0 y la barra de directorios (/) no sabemos si falta */
  if (ruta[tmp-1]=='/')
    sprintf(nombrecompleto,"%s%s", ruta, ent->d_name);
  else
    sprintf(nombrecompleto,"%s/%s", ruta, ent->d_name);
 
  return nombrecompleto;
}

char *generaPosStr(int niv)
{
  int i;
  char *tmp=(char*)malloc(niv*2+1);    /* Dos espacios por nivel más terminador */
  for (i=0; i<niv*2; ++i)
    tmp[i]=' ';
  tmp[niv*2]='\0';
  return tmp;
}




/* stat() vale para mucho más, pero sólo queremos el tipo ahora */
unsigned char statFileType(char *fname)
{
  struct stat sdata;

  /* Intentamos el stat() si no funciona, devolvemos tipo desconocido */
  if (stat(fname, &sdata)==-1)
    {
      return DT_UNKNOWN;
    }

  switch (sdata.st_mode & S_IFMT)
    {
    case S_IFBLK:  return DT_BLK;
    case S_IFCHR:  return DT_CHR;
    case S_IFDIR:  return DT_DIR;
    case S_IFIFO:  return DT_FIFO;
    case S_IFLNK:  return DT_LNK;
    case S_IFREG:  return DT_REG;
    case S_IFSOCK: return DT_SOCK;
    default:       return DT_UNKNOWN;
    }
}
unsigned char getFileType(char *nombre, struct dirent *ent)
{
  unsigned char tipo;

  tipo=ent->d_type;
  if (tipo==DT_UNKNOWN)
    {
      tipo=statFileType(nombre);
    }

  return tipo;
}

FileList *fileList=NULL;

int search(char *path, int niv)
{
    DIR *dir;
  /* en *ent habrá información sobre el archivo que se está "sacando" a cada momento */
    struct dirent *infoOfDir;
    int numfiles=0;          /* Ficheros en el directorio actual */
    char type;       /* Tipo: fichero /directorio/enlace/etc */
    char *name;     /* Nombre completo del fichero */
    char *posstr;         /* Cadena usada para posicionarnos horizontalmente */
    dir = opendir (path);

    /* Miramos que no haya error */
    if (dir == NULL)
        error("No puedo abrir el directorio");
 
    while ((infoOfDir = readdir (dir)) != NULL)
    {
        if ((strcmp(infoOfDir->d_name, ".")!=0) && (strcmp(infoOfDir->d_name, "..")!=0))
        {
            name=getFullName(path, infoOfDir);
            type=getFileType(name, infoOfDir);
            if (type==DT_REG)
            {
                ++numfiles;
                add(createNode(infoOfDir,name),&fileList); 
            }
            else if (type==DT_DIR)
            {
                posstr=generaPosStr(niv);
                printf("%sEntrando en: %s\n", posstr, name);
                printf("%s%s . Total: %u archivos ", posstr, name, search(name, niv+1));
                /* Podemos poner las líneas que queramos */
                printf("\n");
                free(posstr);
            }
        }
    }
  //closedir (dir);
 
  return numfiles;
}

void deleteRepeatedFiles(FileList ** fileList)
{
    FileList *p = *fileList;
    int i=0;
    while(p&&p->sig)
    {
        FileList * q = p;
        while(q&&q->sig)
        {
            printf("%s %s %i\n", q->sig->file->d_name, p->file->d_name, strcmp(q->file->d_name,p->file->d_name)==0);
            if(strcmp(q->sig->file->d_name,p->file->d_name)==0)
            {
                remove(&q);
            }
            else
            {
                q=q->sig;
            } 
        }
        
        p=p->sig;
    }
}

int isSameSize(FileList *p, FileList *q)
{
    return p->size==q->size;
}

int readAFile(int fileDescriptor,char *data, int size)
{
    char *buff;
    int i;
    int m;

    char *aux = data;
    i = 0;
    while(i < size) 
    {
        m = read(fileDescriptor, aux + i, size - i);
        if(m == -1) {
            if(errno == EINTR)
                continue;
            else
                return -1;
        }
        if(m == 0)
            break;
        i += m;
    }
    return i;
}

int compare(FileList *p, FileList *q)
{
    if(isSameSize(p,q))
    {
        int size=p->size>p->sstat.st_blksize?p->sstat.st_blksize:p->size;
        if(p->data==NULL)
        {
            p->data=(char*)malloc(1024*4);
            int readFileP=read(p->fileDescriptor,p->data,size);
        }
        if(q->data==NULL)
        {
            q->data=(char*)malloc(1024*4);
            int readFileQ=read(q->fileDescriptor,q->data,size);
        }
        return strcmp(p->data,q->data)==0;
    }
    return 0;
}

void deleteEqualsFiles(FileList **list)
{
    FileList *p = *list;
    while(p&&p->sig)
    {
        FileList *q = p;
        while(q&&q->sig)
        {
            if(compare(p,q->sig))
            {
                remove(&q);
            }
            else
            {
                q=q->sig;
            } 
        }
        
        p=p->sig;
    }
}

int main(int argc, char *argv[])
{
  int num;

  printf("Entrando en: %s\n", ".");
  num=search(".", 1);
  printf("%s . Total: %u archivos\n", argv[1], num);
  /* Empezaremos a leer en el directorio actual */
  show(fileList);
  deleteEqualsFiles(&fileList);
  show(fileList);
  return EXIT_SUCCESS;
}