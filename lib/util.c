/***************************************************************************
 *   Copyright (C) 2008 by Guan Wang   *
 *   gwang@glc.org   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "util.h"

#define MAXERRMSG 256
#ifdef _WIN32
#define NEWLINE '\r'
#else
#define NEWLINE '\n'
#endif
#define NULCHAR '\0'
#define hash(H,C) while(*(C)!=NEWLINE&&*(C)!=NULCHAR)(H)=((256*(H)+(*(C)++))%(NBUCKETS));
struct _hash_node
{
    char key[MAX_HASH_KEY+1];
    void *data;
    size_t datasize;
    struct _hash_node *next;
};
struct _hash_table
{
    struct _hash_node* bucks[NBUCKETS];
    size_t size;
};

static void _printErr(const char *fmt,va_list ap,int errNoFlag)
{
    char buf[MAXERRMSG];
    int errno_save=errno;
    vsnprintf(buf,MAXERRMSG,fmt,ap);
    if(errNoFlag)
        snprintf(buf+strlen(buf),MAXERRMSG-strlen(buf),": %s",strerror(errno_save));
    if(strlen(buf)<MAXERRMSG-1)
        strcat(buf,"\n");
    fflush(stdout);
    fputs(buf,stderr);
    fflush(stderr);
}
void printError(int errNoFlag,const char *fmt,...)
{
    va_list ap;
    va_start(ap,fmt);
    _printErr(fmt,ap,errNoFlag);
    va_end(ap);
}
struct _hash_table* createHashTab()
{
    struct _hash_table *ptab=(struct _hash_table*)malloc(sizeof(struct _hash_table));
    memset(ptab,0,sizeof(struct _hash_table));
    return ptab;
}

int hashTabPut(struct _hash_table *table,const char * const key,size_t keysize,void *data,size_t datasize)
{
    if(!table)return -1;
    if(keysize>MAX_HASH_KEY)return -2;
    size_t h=0;
    struct _hash_node *node=NULL,*prev=NULL,*curr=NULL;;
    char *pc=(char*)key;
    hash(h,pc)
    node=(struct _hash_node*)malloc(sizeof(struct _hash_node));
    node->next=NULL;
    strncpy(node->key,key,keysize);
    node->key[keysize]=NULCHAR;
    node->data=malloc(datasize);
    memcpy(node->data,data,datasize);
    node->datasize=datasize;
    prev=curr=table->bucks[h];
    while(curr)
    {
	if(strncmp(curr->key,key,keysize)==0)
	{
	    struct _hash_node *pt=curr->next;
	    if(prev!=curr)
	        prev->next=node;
	    else
		table->bucks[h]=node;
	    node->next=pt;
	    free(curr);
	    break;
	}
	else if(!curr->next)
	{
	    curr->next=node;
	    break;               
	}
	else
	{
	    prev=curr;
	    curr=curr->next;
	}
    }
    if(!curr&&!prev)
	table->bucks[h]=node;
    table->size++;
    return 0;
}
void* hashTabLookup(struct _hash_table *table,const char * const key,size_t keysize,size_t *pdatasize,HASH_LOOKUP_MOD mod)
{
    void *data=NULL;
    if(table&&pdatasize&&keysize<=MAX_HASH_KEY)
    {
	size_t h=0;
	char *pc=(char*)key;
	hash(h,pc)
	struct _hash_node *node,*prev;
	prev=node=table->bucks[h];
	while(node&&strncmp(node->key,key,keysize)!=0)
	{
	    prev=node;
	    node=node->next;
	}
	if(node!=NULL)
	{
	    data=malloc(node->datasize);
	    memcpy(data,node->data,node->datasize);
	    *pdatasize=node->datasize;
	    if(mod==REMOVE)
	    {
		if(prev==node)table->bucks[h]=node->next;
		else prev->next=node->next;
		free(node);
		table->size--;
	    }
	}
    }
    return data;
}
size_t gethashTabSize(struct _hash_table *table)
{
    return table->size;
}
void hashTabDestroy(struct _hash_table *table)
{
    if(table)
    {
	struct _hash_node *pnext,*pcurrent;
	int i=0;
	for(;i<NBUCKETS;++i)
	{
	    pcurrent=table->bucks[i];
	    while(pcurrent)
	    {
		pnext=pcurrent->next;
		free(pcurrent->data);
		free(pcurrent);
		pcurrent=pnext;
	    }
	}
	free(table);
    }
}
struct _hash_table* getProperties(const char *path)
{
    struct _hash_table *hash=NULL;
    if(path)
    {
	//int des=open(path,O_RDONLY|O_EXLOCK);
	//if(des>=0)
	{
	    //FILE *pfile=fdopen(des,"r");
	    FILE *pfile=fopen(path,"r");
	    if(pfile)
	    {
		flockfile(pfile);
		char buf[READBUFFSIZ];
		memset(buf,0,READBUFFSIZ);
		int i1,i2;
		hash=createHashTab();
		while(fgets(buf,READBUFFSIZ-1,pfile))
		{
		    i1=0;
		    i2=0;
		    while(i2<READBUFFSIZ-1&&*(buf+i2)!=NEWLINE&&*(buf+i2)!=NULCHAR)
		    {
			if(*(buf+i2)=='='){i1=i2;*(buf+i2)=NULCHAR;}
			++i2;
		    }
		    if(*(buf+i2)==NEWLINE)*(buf+i2)=NULCHAR;
		    //we only handel chars less than READBUFFSIZ each line
		    if(i1>0&&i2<READBUFFSIZ-2&&0==hashTabPut(hash,buf,i1+1,buf+i1+1,i2-i1))
			continue;
		    else
			break;
		}
		if(hash->size==0)
		{
		    hashTabDestroy(hash);
		    hash=NULL;
		}
		funlockfile(pfile);
		fclose(pfile);
	    }
	    //close(des);
	}
    }
    return hash;
}
//static char *irfi_path=NULL;
//static int irfi_len=0;
static int _iterateRegFileInDir(const char* dirName,DIR_SEARCH_MOD mode,void (*fun)(const char*))
{
    if(dirName&&fun)
    {
        struct stat statbuf;
        struct stat statb;
        struct dirent *dirp=NULL;
        DIR *dp=NULL;
        int ret=0;
        if((ret=lstat(dirName,&statbuf))<0)
        {
            printError(ret,"Failed to lstat %s",dirName);
        }
        else
        {
            if(S_ISDIR(statbuf.st_mode)==0)
            {
                printError(1,"%s is not a directory",dirName);
            }
            else
            {
                if((dp=opendir(dirName))==NULL)
                {
                    printError(2,"Can not open %s",dirName);
                }
                else
                {
                    char *ptr=(char*)(dirName+strlen(dirName));
		    *ptr++='/';
                    *ptr=0;
                    while((dirp=readdir(dp))!=NULL)
                    {
                        if(strcmp(dirp->d_name,".")==0||strcmp(dirp->d_name,"..")==0){
			    continue;
 			}
			strcpy(ptr,dirp->d_name);
                        if((ret=lstat(dirName,&statb))<0)
                        {
			    printError(ret,"Failed to lstat %s",dirName);
                        }
                        else
			{
			    if((statb.st_mode&S_IFMT)==S_IFREG)
                            {
			        fun(dirName);
                            }
                            else if((statb.st_mode&S_IFMT)==S_IFDIR&&mode==RESURSIVE)
			    {
				if((ret=_iterateRegFileInDir(dirName,mode,fun))<0)
				    break;
			    }
			}
                    }
 		    ptr[-1]=0;
                    if((ret=closedir(dp))<0)
                        printError(ret,"Failed to close directory %s",dirName);
		    return ret;
                }
            }
        }
    }
    return -1;
}
int iterateRegFileInDir(const char* dirName,DIR_SEARCH_MOD mode,void (*fun)(const char*))
{
    int irfi_len=0;
    char *irfi_path=unix_path_alloc(&irfi_len);
    strncpy(irfi_path,dirName,irfi_len);
    int ret=_iterateRegFileInDir(dirName,mode,fun);
    if(irfi_len>0)
        free(irfi_path);
    irfi_len=0;
    return ret;
}
char* unix_path_alloc(int *sizep)
{
    char *ptr;
    int size;
    if(posix_version==0)
        posix_version=sysconf(_SC_VERSION);
    if(pathmax==0)/*first time through*/
    {
        errno=0;
        if((pathmax=pathconf("/",_PC_PATH_MAX))<0)
        {
            if(0==errno)
                pathmax=PATH_MAX_GUESS;
            else
                printError(errno,"pathconf error for _PC_PATH_MAX");
        }
        else
            pathmax++;/*add one since it's relative to root*/
    }
    if(posix_version<SUSV3)
        size=pathmax+1;
    else
        size=pathmax;
    if((ptr=malloc(size))==NULL)
        printError(errno,"malloc error for pathname");
    if(sizep!=NULL)
        *sizep=size;
    return ptr;
}

int createGrid(const char *pname,const char *playername,int epsg,GridCell *pcells,size_t count)
{
    if(!pname||!playername||!pcells)return 1;
    OGRDataSourceH hDs=NULL;
    OGRSpatialReferenceH hSRS=NULL;
    OGRSFDriverH hDriver=NULL;
    OGRFieldDefnH hFld=NULL;
    OGRLayerH hLayer=NULL;
    OGRFeatureDefnH hFtrDef=NULL;
    OGRFeatureH hFeature=NULL;
    OGRGeometryH hGeom=NULL;
    OGRGeometryH hRing=NULL;
    OGRRegisterAll();
    if(NULL==(hSRS=OSRNewSpatialReference(NULL)))
    {
	printError(errno,"OGR can not create spatial reference");
	return -1;
    }
    OSRImportFromEPSG(hSRS,epsg);
    if((hDriver=OGRGetDriverByName("ESRI Shapefile"))==NULL)
    {
        printError(errno,"OGR can not create driver for %s\n",pname);
        return -2;
    }
    if((hDs=OGR_Dr_CreateDataSource(hDriver,pname,NULL))==NULL)
    {
        printError(errno,"OGR can not create data source for %s, file may already exist!\n",pname);
        return -3;
    }
    if((hLayer=OGR_DS_CreateLayer(hDs,playername,hSRS,wkbPolygon,NULL))==NULL)
    {
        printError(errno,"OGR can not create polygon layer for %s\n",pname);
        return -4;
    }
    if((hFtrDef=OGR_L_GetLayerDefn(hLayer))==NULL)
    {
        printError(errno,"OGR can not retrieve layer defination for %s\n",pname);
        return -5;
    }
    int i=0;
    for(;i<count;++i)
    {
	hFeature=OGR_F_Create(hFtrDef);
        hGeom=OGR_G_CreateGeometry(wkbPolygon);
        hRing=OGR_G_CreateGeometry(wkbLinearRing);
	if((pcells+i)->length<=4)
	{
	    OGR_G_AddPoint_2D(hRing,(pcells+i)->coord1.x,(pcells+i)->coord1.y);
	    OGR_G_AddPoint_2D(hRing,(pcells+i)->coord2.x,(pcells+i)->coord2.y);
	    OGR_G_AddPoint_2D(hRing,(pcells+i)->coord3.x,(pcells+i)->coord3.y);
	    if((pcells+i)->length==4)
	        OGR_G_AddPoint_2D(hRing,(pcells+i)->coord4.x,(pcells+i)->coord4.y);
	    OGR_G_AddPoint_2D(hRing,(pcells+i)->coord1.x,(pcells+i)->coord1.y);
	}
	else
	{
	    Coordinate *ptr=(pcells+i)->pcoords;
	    if(ptr)
	    {
	        int j=(pcells+i)->length;
	    //OGR_G_AddPoint_2D(hRing,ptr->x,ptr->y);
	        while(j--)
	        {
		    OGR_G_AddPoint_2D(hRing,ptr->x,ptr->y);
		    ++ptr;
	        }
	        OGR_G_AddPoint_2D(hRing,(pcells+i)->pcoords->x,(pcells+i)->pcoords->y);
	    }
	    else
	    {
		printError(errno,"Coordinates are not filled for %s\n",pname);
                return -6;
	    }
	}
	//OGR_G_AddPoint_2D(hRing,(pcells+i)->coord1.x,(pcells+i)->coord1.y);
	OGR_G_AddGeometry(hGeom,hRing);
        OGR_F_SetGeometry(hFeature,hGeom);
        OGR_L_CreateFeature(hLayer,hFeature);
        OGR_G_DestroyGeometry(hRing);
        OGR_G_DestroyGeometry(hGeom);
        OGR_F_Destroy(hFeature);
    }
    OSRDestroySpatialReference( hSRS );
    OGR_DS_Destroy(hDs);
    return 0;
}
