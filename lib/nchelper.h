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
#ifndef NCHELPER_H
#define NCHELPER_H

#include <string.h>
#include <stdlib.h>
#include "netcdf.h"
#include "util.h"

#define NC_NAME NC_MAX_NAME+1
#ifdef __cplusplus
extern "C"{
#endif
struct _var_Info;
struct _nc_info;
typedef struct _dimInfo
{
    char name[NC_NAME];
    size_t size;
    int id;
    int start;
    int count;
}NC_DimInfo;
typedef struct _var_Info* NC_Variable;
typedef struct _nc_info* NC_Handler;

extern int NC_EXT_VARIABLE_SIZE[13];

NC_Handler NC_Open4Read(const char *file);
NC_Handler NC_Open4Write(const char *file);
NC_Handler NC_Open4Copy(NC_Handler pNcInfo,const char *file);
int getUnlimitedDimension(NC_Handler pNcInfo);
size_t getDimensionSize(NC_Handler pNcInfo,int did);
int getDimensionCount(NC_Handler pNcInfo);
int getVariableCount(NC_Handler pNcInfo);
int getGlobalAttrCount(NC_Handler pNcInfo);
const char* getDimensionName(NC_Handler pNcInfo, int id);
const char* getGlobalAttrName(NC_Handler pNcInfo, int id);
const char* getVariableName(NC_Handler pNcInfo, int id);
inline nc_type getVariableType(NC_Variable var);
inline size_t getVariableOverallSize(NC_Variable var);
NC_Variable getEmptyVariable();
void releaseVariable(NC_Variable pVar);
//int NC_DefineVariable(NC_Handler pNcInfo,NC_Variable *pVar);
int NC_MetaData_Inq(NC_Handler handle);
void NC_Close(NC_Handler pNcInfo);
void NC_SetDimName(NC_DimInfo *pDimInfo,const char *pszName);
char NC_ReadDimInfo(NC_DimInfo *pDimInfo,NC_Handler pNcInfo);
NC_DimInfo* NC_GetDimByName(struct _var_Info* var,const char const *dimName);
NC_Variable NC_DefineVariable(const char *pszName,NC_Handler pNcInfo,NC_DimInfo *pDimInfo,size_t dim_len);
void NC_DestroyVariable(NC_Variable pVar);
char NC_RemoveVariableAttributes(NC_Handler pNcInfo,NC_Variable pVar,char **attrs,int len);
char NC_AddVariableTextAttribute(NC_Handler pNcInfo,NC_Variable pVar,const char *name,const char *value);
int copyVariablesByUlimitDim(NC_Handler onc,NC_Handler nc,int start,int offset);
char writeFloatAll(NC_Handler handler,NC_Variable pvar,double* pdata);
char NC_ReadAllFloat(NC_Handler pNcInfo,NC_Variable pVarInfo,double *pfVars);
char NC_ReadFloatArray(NC_Handler pNcInfo,NC_Variable pVarInfo,double *pfVars);
NC_DimInfo* NC_Inq_VarDimensions(NC_Handler handle,NC_Variable pVar,int *len);
NC_Variable NC_Inq_Var(NC_Handler handle, const char *var);
char NC_ReadAllValues(NC_Handler pNcInfo,NC_Variable pVarInfo,void *pfVars);
char NC_ReadValues(NC_Handler pNcInfo,NC_Variable pVarInfo,void *pfVars);
#ifdef __cplusplus
}
#endif
#endif
