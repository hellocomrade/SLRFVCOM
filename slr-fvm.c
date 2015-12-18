#include <stdio.h>
#include <string.h>
#include <netcdf.h>
#include <nchelper.h>
#include <util.h>
#define BOOL unsigned char
#define DATA_PATH "Data_Dir"
#define OUTPUT_DIR "Output_Dir"
#define CREATE_GRID "Create_Grid"
#define SOURCE_EPSG "Source_EPSG"
#define WATER_LEVEL_CSV "Create_WaterLevel_CSV"
#define WATER_CURRENT_SHP "Create_WaterCurrent_SHP"
//#define TRUE 0
//#define FALSE 1
typedef struct _varContext
{
    size_t size;
    nc_type type;
    NC_Variable hvar;
    void *pdata;
}VarContext;

BOOL readAllVariable(NC_Handler nc,const char* const vName,VarContext *pvc)
{
    if(pvc&&vName)
    {
	pvc->hvar=NC_Inq_Var(nc,vName);
	if(!pvc->hvar)return FALSE;
	pvc->type=getVariableType(pvc->hvar);
        if(pvc->type>=sizeof(NC_EXT_VARIABLE_SIZE))return FALSE;
	pvc->size=getVariableOverallSize(pvc->hvar);
	pvc->pdata=malloc(NC_EXT_VARIABLE_SIZE[pvc->type]*(pvc->size));
	if(NC_ReadAllValues(nc,pvc->hvar,pvc->pdata)==0)return TRUE;
    }
    return FALSE;
}
int getVariableContext(NC_Handler nc,VarContext *pvc,const char * const name,size_t size)
{
    if(pvc&&nc&&name)
    {
        pvc->hvar=NC_Inq_Var(nc,name);
        if(!pvc->hvar)return 10;
        pvc->type=getVariableType(pvc->hvar);
        if(pvc->type>=sizeof(NC_EXT_VARIABLE_SIZE))return 11;
        pvc->size=size;
        pvc->pdata=malloc(NC_EXT_VARIABLE_SIZE[pvc->type]*(pvc->size));
	return 0;
    }
    return -1;
}
void destroyVariableContext(VarContext *pvc)
{
    if(pvc)
    {
	if(pvc->pdata)
	    free(pvc->pdata);
	if(pvc->hvar)
	    releaseVariable(pvc->hvar);
    }
}
size_t getDimensionLength(NC_Handler handler,VarContext vctx,const char *dname)
{
    size_t result=0;
    int dLen=0,i=0;
    NC_DimInfo* pdims=NC_Inq_VarDimensions(handler,vctx.hvar,&dLen);
    if(!pdims||!dLen)return 0;
    for(;i<dLen;++i)
        if(!strcmp((pdims+i)->name,dname))
        {
            result=(pdims+i)->size;
        }
    free(pdims);
    return result;    
}
int getGrid(const char *fname,const char*lname,int epsg,VarContext x,VarContext y,VarContext nv,size_t nele)
{
    size_t i=0;
    GridCell *pgc=(GridCell*)malloc(sizeof(GridCell)*nele);
    GridCell *ptor=pgc;
    if(!pgc)return -1;
    for(;i<nele;++i,++ptor)
    {
	ptor->length=3;
	ptor->pcoords=NULL;
	ptor->coord1.x=(double)(*(((double*)(x.pdata))+*(((int*)(nv.pdata))+i)-1));
	ptor->coord1.y=(double)(*(((double*)(y.pdata))+*(((int*)(nv.pdata))+i)-1));
	ptor->coord2.x=(double)(*(((double*)(x.pdata))+*(((int*)(nv.pdata))+nele+i)-1));
        ptor->coord2.y=(double)(*(((double*)(y.pdata))+*(((int*)(nv.pdata))+nele+i)-1));
	ptor->coord3.x=(double)(*(((double*)(x.pdata))+*(((int*)(nv.pdata))+nele*2+i)-1));
        ptor->coord3.y=(double)(*(((double*)(y.pdata))+*(((int*)(nv.pdata))+nele*2+i)-1));
    }
    int result=createGrid(fname,lname,epsg,pgc,nele);
    free(pgc);
    return result;
}
int createDepthCSV(const char *fname,VarContext x,VarContext y,VarContext nv,VarContext zeta,VarContext h,size_t nele)
{
    size_t i=0;
    double fx,fy,fz;
    FILE *csv=fopen(fname,"w");
    fputs("lon,lat,depth\n",csv);
    for(;i<nele;++i)
    {
        fx=(*(((double*)(x.pdata))+*(((int*)(nv.pdata))+i)-1)+*(((double*)(x.pdata))+*(((int*)(nv.pdata))+nele+i)-1)+*(((double*)(x.pdata))+*(((int*)(nv.pdata))+nele*2+i)-1))/3.0;
	fy=(*(((double*)(y.pdata))+*(((int*)(nv.pdata))+i)-1)+*(((double*)(y.pdata))+*(((int*)(nv.pdata))+nele+i)-1)+*(((double*)(y.pdata))+*(((int*)(nv.pdata))+nele*2+i)-1))/3.0;
	fz=(*(((double*)(zeta.pdata))+*(((int*)(nv.pdata))+i)-1)+*(((double*)(zeta.pdata))+*(((int*)(nv.pdata))+nele+i)-1)+*(((double*)(zeta.pdata))+*(((int*)(nv.pdata))+nele*2+i)-1))/3.0+(*(((double*)(h.pdata))+*(((int*)(nv.pdata))+i)-1)+*(((double*)(h.pdata))+*(((int*)(nv.pdata))+nele+i)-1)+*(((double*)(h.pdata))+*(((int*)(nv.pdata))+nele*2+i)-1))/3.0;
	fprintf(csv,"%f,%f,%f\n",fx,fy,fz);
    }
    fclose(csv);
    return 0;
}
int createField(const char *pszFldName,OGRFieldType type,size_t width,size_t precision,OGRLayerH hLayer)
{
    char result=-1;
    OGRFieldDefnH hFld=NULL;
    if(!pszFldName||!hLayer)
        return result;
    if((hFld=OGR_Fld_Create(pszFldName,type))!=NULL)
    {
        if(width>0)
            OGR_Fld_SetWidth(hFld,width);
        if(precision>0)
            OGR_Fld_SetPrecision(hFld,precision);
        OGR_L_CreateField(hLayer,hFld,FALSE);
        OGR_Fld_Destroy(hFld);
        result=0;
    }
    return result;
}
int createPolygonLayer(const char * const fname,const char * const time,VarContext x,VarContext y,VarContext u,VarContext v,VarContext nv,VarContext h,VarContext zeta,VarContext siglay,int siglaysize,int nele,int node,int epsg)
{
    if(!fname||!time||nele<=0||epsg<=0||siglaysize<=0||siglaysize>9)return -1;
    OGRDataSourceH hDs=NULL;
    OGRSpatialReferenceH hSRS=NULL;
    OGRSFDriverH hDriver=NULL;
    OGRLayerH hLayer=NULL;
    OGRFeatureDefnH hFtrDef=NULL;
    OGRFeatureH hFeature=NULL;
    OGRGeometryH hGeom=NULL;
    OGRGeometryH hRing=NULL;
    char u3[3];
    char u4[4];
    char d3[3];
    char d5[5];
    char d7[6];
    OGRRegisterAll();
    if(NULL==(hSRS=OSRNewSpatialReference(NULL)))
    {
        return -2;
    }
    OSRImportFromEPSG(hSRS,epsg);
    if((hDriver=OGRGetDriverByName("ESRI Shapefile"))==NULL)
    {
        return -3;
    }
    if((hDs=OGR_Dr_CreateDataSource(hDriver,fname,NULL))==NULL)
    {
        return -4;
    }
    if((hLayer=OGR_DS_CreateLayer(hDs,"slr-fvcom",hSRS,wkbPolygon,NULL))==NULL)
    {
        return -5;
    }
    int i=0;
    int cnt=6;
    for(;i<siglaysize;++i)
    {
        sprintf(u3,"u%d",i);
	if(createField(u3,OFTReal,32,8,hLayer)<0)
        {
            return -6;
        }
        sprintf(u3,"v%d",i);
        if(createField(u3,OFTReal,32,8,hLayer)<0)
        {
            return -6;
        }
        sprintf(u4,"uv%d",i);
        if(createField(u4,OFTReal,32,8,hLayer)<0)
        {
            return -6;
        }
        sprintf(d5,"dir%d",i);
        if(createField(d5,OFTInteger,3,-1,hLayer)<0)
        {
            return -6;
        }
        sprintf(d7,"msdir%d",i);
        if(createField(d7,OFTInteger,4,-1,hLayer)<0)
        {
            return -6;
        }
        sprintf(d3,"d%d",i);
        if(createField(d3,OFTReal,32,8,hLayer)<0)
        {
            return -6;
        }
    }
    if(createField("depth",OFTReal,32,8,hLayer)<0)
    {
        return -6;
    }
    if(createField("wlev",OFTReal,32,8,hLayer)<0)
    {
        return -6;
    }
    if(createField("bathy",OFTReal,32,8,hLayer)<0)
    {
        return -6;
    }
    if(createField("datetime",OFTString,21,-1,hLayer)<0)
    {
        return -6;
    }
    if((hFtrDef=OGR_L_GetLayerDefn(hLayer))==NULL)
    {
        return -7;
    }
    int j=0,d=0;
    i=0;
    double depth;
    for(;i<nele;++i)
    {
        hFeature=OGR_F_Create(hFtrDef);
        hGeom=OGR_G_CreateGeometry(wkbPolygon);
        hRing=OGR_G_CreateGeometry(wkbLinearRing);
	OGR_G_AddPoint_2D(hRing,*(((double*)(x.pdata))+*(((int*)(nv.pdata))+i)-1),*(((double*)(y.pdata))+*(((int*)(nv.pdata))+i)-1));
	OGR_G_AddPoint_2D(hRing,*(((double*)(x.pdata))+*(((int*)(nv.pdata))+nele+i)-1),*(((double*)(y.pdata))+*(((int*)(nv.pdata))+nele+i)-1));
	OGR_G_AddPoint_2D(hRing,*(((double*)(x.pdata))+*(((int*)(nv.pdata))+nele*2+i)-1),*(((double*)(y.pdata))+*(((int*)(nv.pdata))+nele*2+i)-1));
	OGR_G_AddPoint_2D(hRing,*(((double*)(x.pdata))+*(((int*)(nv.pdata))+i)-1),*(((double*)(y.pdata))+*(((int*)(nv.pdata))+i)-1));
	OGR_G_AddGeometry(hGeom,hRing);
        OGR_F_SetGeometry(hFeature,hGeom);
        j=0;
        for(;j<siglaysize;++j)
        {
            OGR_F_SetFieldDouble(hFeature,j*cnt,*(((double*)(u.pdata))+j*nele+i));
            OGR_F_SetFieldDouble(hFeature,j*cnt+1,*(((double*)(v.pdata))+j*nele+i));
            OGR_F_SetFieldDouble(hFeature,j*cnt+2,sqrt(pow(*(((double*)(u.pdata))+j*nele+i),2)+pow(*(((double*)(v.pdata))+j*nele+i),2)));
            d=(int)(atan2(*(((double*)(u.pdata))+j*nele+i)*(1.0),*(((double*)(v.pdata))+j*nele+i)*(1.0))*180.0/PI+0.5);
            if(d<0)d+=360;
            OGR_F_SetFieldInteger(hFeature,j*cnt+3,d);
            OGR_F_SetFieldInteger(hFeature,j*cnt+4,d*(-1));
            depth=(*(((double*)(h.pdata))+*(((int*)(nv.pdata))+i)-1)*(*(((double*)siglay.pdata)+j*node+*(((int*)(nv.pdata))+i)-1))+*(((double*)(h.pdata))+*(((int*)(nv.pdata))+nele+i)-1)*(*(((double*)siglay.pdata)+j*node+*(((int*)(nv.pdata))+nele+i)-1))+*(((double*)(h.pdata))+*(((int*)(nv.pdata))+nele*2+i)-1)*(*(((double*)siglay.pdata)+j*node+*(((int*)(nv.pdata))+nele*2+i)-1)))/3.0+(*(((double*)(zeta.pdata))+*(((int*)(nv.pdata))+i)-1)*(*(((double*)siglay.pdata)+j*node+*(((int*)(nv.pdata))+i)-1))+*(((double*)(zeta.pdata))+*(((int*)(nv.pdata))+nele+i)-1)*(*(((double*)siglay.pdata)+j*node+*(((int*)(nv.pdata))+nele+i)-1))+*(((double*)(zeta.pdata))+*(((int*)(nv.pdata))+nele*2+i)-1)*(*(((double*)siglay.pdata)+j*node+*(((int*)(nv.pdata))+nele*2+i)-1)))/3.0;
            OGR_F_SetFieldDouble(hFeature,j*cnt+5,depth);
        }
	depth=(*(((double*)(h.pdata))+*(((int*)(nv.pdata))+i)-1)+*(((double*)(h.pdata))+*(((int*)(nv.pdata))+nele+i)-1)+*(((double*)(h.pdata))+*(((int*)(nv.pdata))+nele*2+i)-1))/3.0+(*(((double*)(zeta.pdata))+*(((int*)(nv.pdata))+i)-1)+*(((double*)(zeta.pdata))+*(((int*)(nv.pdata))+nele+i)-1)+*(((double*)(zeta.pdata))+*(((int*)(nv.pdata))+nele*2+i)-1))/3.0;
	OGR_F_SetFieldDouble(hFeature,j*cnt,depth*(-1.0));
	OGR_F_SetFieldDouble(hFeature,j*cnt+1,(*(((double*)(zeta.pdata))+*(((int*)(nv.pdata))+i)-1)+*(((double*)(zeta.pdata))+*(((int*)(nv.pdata))+nele+i)-1)+*(((double*)(zeta.pdata))+*(((int*)(nv.pdata))+nele*2+i)-1))/3.0);
	OGR_F_SetFieldDouble(hFeature,j*cnt+2,(*(((double*)(h.pdata))+*(((int*)(nv.pdata))+i)-1)+*(((double*)(h.pdata))+*(((int*)(nv.pdata))+nele+i)-1)+*(((double*)(h.pdata))+*(((int*)(nv.pdata))+nele*2+i)-1))/3.0);
        OGR_F_SetFieldString(hFeature,j*cnt+3,time);
        OGR_L_CreateFeature(hLayer,hFeature);
	OGR_G_DestroyGeometry(hRing);
        OGR_G_DestroyGeometry(hGeom);
        OGR_F_Destroy(hFeature);
    }
    OSRDestroySpatialReference( hSRS );
    OGR_DS_Destroy(hDs);
    return 0;
}
int createWaterCurrentPointLayer(const char * const fname,const char * const time,VarContext x,VarContext y,VarContext u,VarContext v,VarContext nv,VarContext h,VarContext zeta,VarContext siglay,int siglaysize,int nele,int node,int epsg)
{
    if(!fname||!time||nele<=0||epsg<=0||siglaysize<=0||siglaysize>9)return -1;
    OGRDataSourceH hDs=NULL;
    OGRSpatialReferenceH hSRS=NULL;
    OGRSFDriverH hDriver=NULL;
    OGRLayerH hLayer=NULL;
    OGRFeatureDefnH hFtrDef=NULL;
    OGRFeatureH hFeature=NULL;
    OGRGeometryH hGeom=NULL;
    char u3[3];
    char u4[4];
    char d3[3];
    char d5[5];
    char d7[6];
    OGRRegisterAll();
    if(NULL==(hSRS=OSRNewSpatialReference(NULL)))
    {
        return -2;
    }
    OSRImportFromEPSG(hSRS,epsg);
    if((hDriver=OGRGetDriverByName("ESRI Shapefile"))==NULL)
    {
        return -3;
    }
    if((hDs=OGR_Dr_CreateDataSource(hDriver,fname,NULL))==NULL)
    {
        return -4;
    }
    if((hLayer=OGR_DS_CreateLayer(hDs,"slr-water-current",hSRS,wkbPoint,NULL))==NULL)
    {
        return -5;
    }
    int i=0;
    int cnt=6;
    for(;i<siglaysize;++i)
    {
	sprintf(u3,"u%d",i);
        if(createField(u3,OFTReal,32,8,hLayer)<0)
        {
            return -6;
        }
	sprintf(u3,"v%d",i);
        if(createField(u3,OFTReal,32,8,hLayer)<0)
        {
            return -6;
        }
	sprintf(u4,"uv%d",i);
        if(createField(u4,OFTReal,32,8,hLayer)<0)
        {
            return -6;
        }
	sprintf(d5,"dir%d",i);
        if(createField(d5,OFTInteger,3,-1,hLayer)<0)
        {
            return -6;
        }
	sprintf(d7,"msdir%d",i);
        if(createField(d7,OFTInteger,4,-1,hLayer)<0)
        {
            return -6;
        }
	sprintf(d3,"d%d",i);
	if(createField(d3,OFTReal,32,8,hLayer)<0)
        {
            return -6;
        }
    }
    if(createField("datetime",OFTString,21,-1,hLayer)<0)
    {
        return -6;
    }
    if((hFtrDef=OGR_L_GetLayerDefn(hLayer))==NULL)
    {
        return -7;
    }
    int j=0,d=0;
    i=0;
    double depth;
    for(;i<nele;++i)
    {
	hFeature=OGR_F_Create(hFtrDef);
        hGeom=OGR_G_CreateGeometry(wkbPoint);
	OGR_G_SetPoint(hGeom,0,(*(((double*)(x.pdata))+*(((int*)(nv.pdata))+i)-1)+*(((double*)(x.pdata))+*(((int*)(nv.pdata))+nele+i)-1)+*(((double*)(x.pdata))+*(((int*)(nv.pdata))+nele*2+i)-1))/3.0,(*(((double*)(y.pdata))+*(((int*)(nv.pdata))+i)-1)+*(((double*)(y.pdata))+*(((int*)(nv.pdata))+nele+i)-1)+*(((double*)(y.pdata))+*(((int*)(nv.pdata))+nele*2+i)-1))/3.0,0.0);
	OGR_F_SetGeometry(hFeature,hGeom);
	j=0;
	for(;j<siglaysize;++j)
 	{
	    OGR_F_SetFieldDouble(hFeature,j*cnt,*(((double*)(u.pdata))+j*nele+i));
	    OGR_F_SetFieldDouble(hFeature,j*cnt+1,*(((double*)(v.pdata))+j*nele+i));
	    OGR_F_SetFieldDouble(hFeature,j*cnt+2,sqrt(pow(*(((double*)(u.pdata))+j*nele+i),2)+pow(*(((double*)(v.pdata))+j*nele+i),2)));
	    d=(int)(atan2(*(((double*)(u.pdata))+j*nele+i),*(((double*)(v.pdata))+j*nele+i))*180.0/PI+0.5);
	    if(d<0)d+=360;
            OGR_F_SetFieldInteger(hFeature,j*cnt+3,d);
	    OGR_F_SetFieldInteger(hFeature,j*cnt+4,d*(-1));
	    depth=(*(((double*)(h.pdata))+*(((int*)(nv.pdata))+i)-1)*(*(((double*)siglay.pdata)+j*node+*(((int*)(nv.pdata))+i)-1))+*(((double*)(h.pdata))+*(((int*)(nv.pdata))+nele+i)-1)*(*(((double*)siglay.pdata)+j*node+*(((int*)(nv.pdata))+nele+i)-1))+*(((double*)(h.pdata))+*(((int*)(nv.pdata))+nele*2+i)-1)*(*(((double*)siglay.pdata)+j*node+*(((int*)(nv.pdata))+nele*2+i)-1)))/3.0+(*(((double*)(zeta.pdata))+*(((int*)(nv.pdata))+i)-1)*(*(((double*)siglay.pdata)+j*node+*(((int*)(nv.pdata))+i)-1))+*(((double*)(zeta.pdata))+*(((int*)(nv.pdata))+nele+i)-1)*(*(((double*)siglay.pdata)+j*node+*(((int*)(nv.pdata))+nele+i)-1))+*(((double*)(zeta.pdata))+*(((int*)(nv.pdata))+nele*2+i)-1)*(*(((double*)siglay.pdata)+j*node+*(((int*)(nv.pdata))+nele*2+i)-1)))/3.0;
	    OGR_F_SetFieldDouble(hFeature,j*cnt+5,depth);
	}
	OGR_F_SetFieldString(hFeature,j*cnt,time);
	OGR_L_CreateFeature(hLayer,hFeature);
        OGR_G_DestroyGeometry(hGeom);
        OGR_F_Destroy(hFeature);
    }
    OSRDestroySpatialReference( hSRS );
    OGR_DS_Destroy(hDs);
    return 0;
}
char *getFullPath(char *path,size_t psize,char *name,size_t nsize)
{
    char *p=NULL;
    if(path&&name)
    {
	char *pp=name;
	while(*pp++)
	    if(*pp=='/')
		return NULL;
	if(*(path+psize-1)!='/')
	    p=(char*)malloc(psize+nsize+2);
	else
	    p=(char*)malloc(psize+nsize+1);
	if(*(path+psize-1)!='/')
	    sprintf(p,"%s/%s",path,name);
	else
	    sprintf(p,"%s%s",path,name);
    }
    return p;
}
void help()
{
    fprintf(stderr,"program -c [configuration file path&name] -f [nc file name]\n");
}
int main(int argc, char **argv)
{
    char c;
    char *configPath=NULL;
    char *dataRepo=NULL;
    char *fileName=NULL;
    char *filePath=NULL;
    char *outputDir=NULL;
    char *gridFlag=NULL;
    char *epsg=NULL;
    char *wlFlag=NULL;
    char *wcFlag=NULL;
    HASHTAB config=NULL;
    size_t size;
    while((c=getopt(argc,argv,"c:f:"))!=-1)
    {
	switch(c)
	{
	case 'c':
	    configPath=optarg;
	    break;
	case 'f':
	    fileName=optarg;
	    break;
	default:
	    help();
	    return 0;
	}
    }
    if(!configPath||!fileName)
    {
	help();
	return 0;
    }
    config=getProperties(configPath);
    if(!config)
    {
	help();
        return 1;
    }
    dataRepo=(char*)hashTabLookup(config,DATA_PATH,strlen(DATA_PATH),&size,KEEP);
    if(!dataRepo||!size)
    {
	return 2;
    }
    if(*(dataRepo+size-1))
	*(dataRepo+size-1)=0;
    filePath=getFullPath(dataRepo,strlen(dataRepo),fileName,strlen(fileName));
    if(!filePath)return 3;
    outputDir=(char*)hashTabLookup(config,OUTPUT_DIR,strlen(OUTPUT_DIR),&size,KEEP);
    if(!outputDir||!size)
    {
	return 4;
    }
    if(!*(outputDir+size-1))
	*(outputDir+size-1)=0;
    size_t odlen=strlen(outputDir);
    NC_Handler handler=NC_Open4Read(filePath);
    if(handler)
    {
        VarContext time;
	///if(FALSE==readAllVariable(handler,"time",&time))return 4;
	if(FALSE==readAllVariable(handler,"Times",&time))return 4;
	VarContext lon;
	if(FALSE==readAllVariable(handler,"lon",&lon))return 5;
	VarContext lat;
	if(FALSE==readAllVariable(handler,"lat",&lat))return 6;
	VarContext nv;
	if(FALSE==readAllVariable(handler,"nv",&nv))return 7;
	VarContext h;
	if(FALSE==readAllVariable(handler,"h",&h))return 8;
	VarContext siglay;
        if(FALSE==readAllVariable(handler,"siglay",&siglay))return 9;
	size_t timeDSize=0,nodeDSize=0,elemDSize=0,siglayDSize=0;
	///timeDSize=getVariableOverallSize(time.hvar);
	size_t timeDSize1=0;
	timeDSize=getDimensionLength(handler,time,"time");
	timeDSize1=getDimensionLength(handler,time,"DateStrLen");
	nodeDSize=getDimensionLength(handler,lon,"node");
	elemDSize=getDimensionLength(handler,nv,"nele");
	siglayDSize=getDimensionLength(handler,siglay,"siglay");
	if(!timeDSize||!nodeDSize||!elemDSize||!siglayDSize)return 8;
	epsg=(char*)hashTabLookup(config,SOURCE_EPSG,strlen(SOURCE_EPSG),&size,KEEP);
        int iepsg=0;
        if(epsg&&strlen(epsg))
            iepsg=atoi(epsg);
        if(iepsg==0||iepsg==INT_MAX||iepsg==INT_MIN)
            iepsg=4326;
        if(epsg)free(epsg);
        gridFlag=(char*)hashTabLookup(config,CREATE_GRID,strlen(CREATE_GRID),&size,KEEP);
	if(gridFlag&&strlen(gridFlag)==1&&*gridFlag=='1')
	{
	    char *shpPath=getFullPath(outputDir,odlen,"fvcom-grid.shp",strlen("fvcom-grid.shp"));
	    if(!shpPath)
	    {
		return 3;
	    }
	    if(0!=getGrid(shpPath,"grid",iepsg,lon,lat,nv,elemDSize))
	        return 9;
	    free(shpPath);
	    free(gridFlag);
	}
	
	int i=0;
	char wlfname[28]={0};
	VarContext zeta;
	VarContext u,v;
	
	if(0!=(i=getVariableContext(handler,&zeta,"zeta",nodeDSize)))
	    return i;
	if(0!=(i=getVariableContext(handler,&u,"u",elemDSize*siglayDSize)))
	    return i;
	if(0!=(i=getVariableContext(handler,&v,"v",elemDSize*siglayDSize)))
	    return i;
	char *wlpath=getFullPath(outputDir,odlen,wlfname,28);
	if(!wlpath)
	{
	    return 3;
	}
	char timeBuf[21]={0};
	///time_t secs;
	///double *d=(double*)time.pdata;
	char *ptime=(char*)time.pdata;
	wlFlag=(char*)hashTabLookup(config,WATER_LEVEL_CSV,strlen(WATER_LEVEL_CSV),&size,KEEP);
	BOOL bwl=wlFlag!=NULL&&strlen(wlFlag)==1&&*wlFlag=='1'?TRUE:FALSE;
	wcFlag=(char*)hashTabLookup(config,WATER_CURRENT_SHP,strlen(WATER_CURRENT_SHP),&size,KEEP);
	BOOL bwc=wcFlag!=NULL&&strlen(wcFlag)==1&&*wcFlag=='1'?TRUE:FALSE;
	for(;i<timeDSize;++i)
	{
	    ///secs=MJD2EPOCH(*(d+i));
	    ///strftime(timeBuf,20,"%F %H:%M:%S",gmtime(&secs));
	    strncpy(timeBuf,ptime,19);
	    ptime+=timeDSize1;
	    if(bwl)
	    {
	        ///strftime(wlfname,27,"wl%FT%T.csv",gmtime(&secs));
		sprintf(wlfname,"wl%s.csv",timeBuf);
	        strncpy(wlpath+odlen+1,wlfname,27);
	    }
	    NC_DimInfo *pdim=NC_GetDimByName(zeta.hvar,"time");
	    if(!pdim)return 12;
	    pdim->start=i;
	    pdim->count=1;
	    pdim=NC_GetDimByName(zeta.hvar,"node");
            if(!pdim)return 12;
            pdim->start=0;
            pdim->count=pdim->size;
            if(NC_ReadValues(handler,zeta.hvar,zeta.pdata)!=0)return 13;
	    if(bwl)
	        createDepthCSV(wlpath,lon,lat,nv,zeta,h,elemDSize);
	    if(bwc)
	    {
	        ///strftime(wlfname,27,"wc%FT%T.shp",gmtime(&secs));
		sprintf(wlfname,"wc%s.shp",timeBuf);
	        strncpy(wlpath+odlen+1,wlfname,27);
	    }
	    pdim=NC_GetDimByName(u.hvar,"time");
	    if(!pdim)return 12;
	    pdim->start=i;
	    pdim->count=1;
	    pdim=NC_GetDimByName(u.hvar,"siglay");
	    if(!pdim)return 12;
	    pdim->start=0;
	    pdim->count=pdim->size;
	    pdim=NC_GetDimByName(u.hvar,"nele");
	    if(!pdim)return 12;
	    pdim->start=0;
            pdim->count=pdim->size;
	    pdim=NC_GetDimByName(v.hvar,"time");
            if(!pdim)return 12;
            pdim->start=i;
            pdim->count=1;
            pdim=NC_GetDimByName(v.hvar,"siglay");
            if(!pdim)return 12;
            pdim->start=0;
            pdim->count=pdim->size;
            pdim=NC_GetDimByName(v.hvar,"nele");
            if(!pdim)return 12;
            pdim->start=0;
            pdim->count=pdim->size;
	    if(NC_ReadValues(handler,u.hvar,u.pdata)!=0)return 13;
	    if(NC_ReadValues(handler,v.hvar,v.pdata)!=0)return 13;
	    if(bwc)
	        if(0!=createWaterCurrentPointLayer(wlpath,timeBuf,lon,lat,u,v,nv,h,zeta,siglay,siglayDSize,elemDSize,nodeDSize,iepsg))
		    return 13;
	    ///strftime(wlfname,27,"sl%FT%T.shp",gmtime(&secs));
	    sprintf(wlfname,"sl%s.shp",timeBuf);
            strncpy(wlpath+odlen+1,wlfname,27);
	    if(0!=createPolygonLayer(wlpath,timeBuf,lon,lat,u,v,nv,h,zeta,siglay,siglayDSize,elemDSize,nodeDSize,iepsg))
                return 14;
	}
	free(wlpath);
	if(wlFlag)free(wlFlag);
	if(wcFlag)free(wcFlag);
	destroyVariableContext(&u);
	destroyVariableContext(&v);
	destroyVariableContext(&zeta);
	destroyVariableContext(&siglay);
	destroyVariableContext(&h);
	destroyVariableContext(&nv);
	destroyVariableContext(&lat);
	destroyVariableContext(&lon);
	destroyVariableContext(&time);
	NC_Close(handler);
    }
    free(outputDir);
    free(dataRepo);
    free(filePath);
    hashTabDestroy(config);
    return 0;
}
