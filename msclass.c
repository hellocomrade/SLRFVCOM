#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
int count;
void printClass(FILE *file,const char *classname,const char *expr,int r,int g,int b)
{//count++;
    //or historical reasons, width has two meanings within the context of filling polygons and stroke widths for the outline. If a polygon is filled, then the width defines the width of the symbol inside the filled polygon. In this event, the outline width is disregarded, and it is always set to 1. To acheive the effect of both a fill and an outline width, you need to use two styles in your class

    fprintf(file,"\nCLASS\n\tNAME \"%s\"\n\tEXPRESSION (%s)\n\tSTYLE\n\t\tCOLOR %d %d %d\n\tEND\n\tSTYLE\n\t\tOUTLINECOLOR %d %d %d\n\t\tWIDTH 0\n\t\tANTIALIAS TRUE\n\tEND\nEND\n",classname,expr,r,g,b,r,g,b);
}
int main(int argc,char **argv)
{
    if(argc<5)
    {
	fprintf(stderr,"program_name minval maxval numcolor varname\n");
	return -1;
    }
    double min=atof(*(argv+1));
    double max=atof(*(argv+2));
    int num=atoi(*(argv+3))/4;
    char *vname=*(argv+4);
    double dv=max-min;
    if(min>=max||num<1||!vname)
    {
	fprintf(stderr,"params are not valid,dude!\n");
	return -2;
    }
    int r=255,g=255,b=255,i=0;
    double j,k;
    char classBuf[64];
    memset(classBuf,0,64);
    char exprBuf[64];
    memset(exprBuf,0,64);
    //bule2cyan
    double step=0.25*dv/num;
    r=0;
    for(;i<num;++i)
    {
	j=min+step*i;
	k=min+step*(i+1);
	g=4.0*(j-min)/dv*255;
	sprintf(classBuf,"%.2f-%.2f",j,k);
        sprintf(exprBuf,"[%s]>=%.2f AND [%s]<%.2f",vname,j,vname,k);
	printClass(stdout,classBuf,exprBuf,r,g,b);
	
    }
    //cyan2green
    i=0;
    g=255;
    for(;i<num;++i)
    {
	j=(max+3*min)/4.0+step*i;
	k=(max+3*min)/4.0+step*(i+1);
	b=255*(1+4*(min+0.25*dv-j)/dv);
	sprintf(classBuf,"%.2f-%.2f",j,k);
        sprintf(exprBuf,"[%s]>=%.2f AND [%s]<%.2f",vname,j,vname,k);
        printClass(stdout,classBuf,exprBuf,r,g,b);
    }
    i=0,b=0;
    for(;i<num;++i)
    {
	j=(min+max)/2.0+step*i;
	k=(min+max)/2.0+step*(i+1);
	r=255*4*(j-min-0.5*dv)/dv;
	sprintf(classBuf,"%.2f-%.2f",j,k);
        sprintf(exprBuf,"[%s]>=%.2f AND [%s]<%.2f",vname,j,vname,k);
        printClass(stdout,classBuf,exprBuf,r,g,b);
    }
    i=0,r=255;
    for(;i<num;++i)
    {
	j=(3*max+min)/4.0+step*i;
	k=(3*max+min)/4.0+step*(i+1);
	g=255*(1+4*(min+0.75*dv-j)/dv);
	sprintf(classBuf,"%.2f-%.2f",j,k);
        sprintf(exprBuf,"[%s]>=%.2f AND [%s]<%.2f",vname,j,vname,k);
        printClass(stdout,classBuf,exprBuf,r,g,b);
    }
//printf("total:%d\n",count);
    memset(classBuf,0,strlen(exprBuf));
    sprintf(classBuf,"%.2f-INF",max);
    memset(exprBuf,0,strlen(exprBuf));
    sprintf(exprBuf,"[%s]>=%.2f",vname,max);
    printClass(stdout,classBuf,exprBuf,0,0,255);
    return 0;
}
