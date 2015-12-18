#include <stdio.h>
#include <string.h>
#include <util.h>

int main(void)
{
HASHTAB hash=createHashTab();
size_t i=3;
hashTabPut(hash,"key",3,&i,sizeof(size_t));
printf("%lu\n",*((size_t*)hashTabLookup(hash,"key",3,&i,KEEP)));
printf("%lu\n",i);
char *s="abc";
hashTabPut(hash,"abcd",4,s,strlen(s)+1);
printf("%s\n",(char*)hashTabLookup(hash,"abcd",4,&i,KEEP));
printf("%lu\n",i);
s="defgh";
hashTabPut(hash,"abcd",4,s,strlen(s)+1);
printf("%s\n",(char*)hashTabLookup(hash,"abcd",4,&i,KEEP));
printf("%lu\n",i);
printf("%lu\n",gethashTabSize(hash));
printf("%s\n",(char*)hashTabLookup(hash,"abcd",4,&i,REMOVE));
printf("%lu\n",gethashTabSize(hash));
hashTabDestroy(hash);
hash=getProperties("./myconfig");
printf("%s\n",(char*)hashTabLookup(hash,"key1",4,&i,KEEP));
printf("%s\n",(char*)hashTabLookup(hash,"key2",4,&i,KEEP));
hashTabDestroy(hash);
return 0;
}
