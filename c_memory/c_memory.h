/*
 * LEGO® MINDSTORMS EV3
 *
 * Copyright (C) 2010-2013 The LEGO Group
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef C_MEMORY_H_
#define C_MEMORY_H_

#include  "lms2012.h"

enum
{
  OPEN_FOR_WRITE    = 1,
  OPEN_FOR_APPEND   = 2,
  OPEN_FOR_READ     = 3,
  OPEN_FOR_LOG      = 4
};

RESULT    ConstructFilename(PRGID PrgId,char *pFilename,char *pName,const char *pDefaultExt);
DSPSTAT   cMemoryOpenFile(PRGID PrgId,DATA8 Access,char *pFileName,HANDLER *pHandle,DATA32 *pSize);
RESULT    cMemoryAlloc(PRGID PrgId,DATA8 Type,GBINDEX Size,void **ppMemory,HANDLER *pHandle);
DSPSTAT   cMemoryReadFile(PRGID PrgId,HANDLER Handle,DATA32 Size,DATA8 Del,DATA8 *pDestination);
void      cMemoryDeleteSubFolders(char *pFolderName);
DSPSTAT   cMemoryWriteFile(PRGID PrgId,HANDLER Handle,DATA32 Size,DATA8 Del,DATA8 *pSource);
DSPSTAT   cMemoryGetFileHandle(PRGID PrgId,char *pFileName,HANDLER *pHandle,DATA8 *pOpenForWrite);
void      cMemoryFilename(PRGID PrgId,char *pName,char *pExt,DATA8 Length,char *pResult);
void      cMemoryFileMd5Sum(void);

RESULT    cMemoryInit(void);

RESULT    cMemoryOpen(PRGID PrgId,GBINDEX Size,void **pMemory);

RESULT    cMemoryClose(PRGID PrgId);

RESULT    cMemoryExit(void);

RESULT    cMemoryRealloc(void *pOldMemory,void **ppMemory,DATA32 Size);

RESULT    cMemoryFree(void *pMemory);

RESULT    cMemoryGetPointer(PRGID PrgId,HANDLER Handle,void **pMemory);

RESULT    cMemoryArraryPointer(PRGID PrgId,HANDLER Handle,void **pMemory);

DATA8     cMemoryGetCacheFiles(void);

RESULT    cMemoryGetCacheName(DATA8 Item,DATA8 MaxLength,char *pFileName,char *pName,DATA8 *pType);

DATA8     cMemoryFindSubFolders(char *pFolderName);

DATA8     cMemoryGetSubFolderName(DATA8 Item,DATA8 MaxLength,char *pFolderName,char *pSubFolderName);

DATA8     cMemoryFindFiles(char *pFolderName);

void      cMemoryGetResourcePath(PRGID PrgId,char *pString,DATA8 MaxLength);

RESULT    cMemoryGetIcon(DATA8 *pFolderName,DATA8 Item,DATA32 *pImagePointer);

RESULT    cMemoryGetImage(DATA8 *pText,DATA16 Size,UBYTE *pBmp);

DSPSTAT   cMemoryCloseFile(PRGID PrgId,HANDLER Handle);

RESULT    cMemoryCheckOpenWrite(char *pFileName);

RESULT    cMemoryCheckFilename(char *pFilename,char *pPath,char *pName,char *pExt);

RESULT    cMemoryGetMediaName(char *pChar,char *pName);



void      cMemoryFile(void);

void      cMemoryArray(void);

void      cMemoryArrayWrite(void);

void      cMemoryArrayRead(void);

void      cMemoryArrayAppend(void);

void*     cMemoryResize(PRGID PrgId,HANDLER Handle,DATA32 Elements);

void      cMemoryFileName(void);



RESULT    cMemoryOpenFolder(PRGID PrgId,DATA8 Type,DATA8 *pFolderName,HANDLER *pHandle);

RESULT    cMemoryGetFolderItems(PRGID PrgId,HANDLER Handle,DATA16 *pItems);

RESULT    cMemoryGetItemName(PRGID PrgId,HANDLER Handle,DATA16 Item,DATA8 Length,DATA8 *pName,DATA8 *pType,DATA8 *pPriority);

RESULT    cMemoryGetItemIcon(PRGID PrgId,HANDLER Handle,DATA16 Item,HANDLER *pHandle,DATA32 *pImagePointer);

RESULT    cMemoryGetItemText(PRGID PrgId,HANDLER Handle,DATA16 Item,DATA8 Length,DATA8 *pText);

RESULT    cMemorySetItemText(PRGID PrgId,HANDLER Handle,DATA16 Item,DATA8 *pText);

RESULT    cMemoryGetItem(PRGID PrgId,HANDLER Handle,DATA16 Item,DATA8 Length,DATA8 *pName,DATA8 *pType);

void      cMemoryCloseFolder(PRGID PrgId,HANDLER *pHandle);

void      cMemoryGetUsage(DATA32 *pTotal,DATA32 *pFree,DATA8 Force);

void      cMemoryUsage(void);

#define   POOL_TYPE_MEMORY    0
#define   POOL_TYPE_FILE      1

typedef   struct
{
  void    *pPool;
  GBINDEX Size;
  DATA8   Type;
}
POOL;


typedef   struct
{
  DATA32  Elements;
  DATA32  UsedElements;
  DATA8   ElementSize;
  DATA8   Type;
  DATA8   Free1;
  DATA8   Free2;
  DATA8   pArray[]; // Must be aligned
}
DESCR;

typedef   struct
{
  int     hFile;
  DATA8   Access;
  char    Filename[vmFILENAMESIZE];
}
FDESCR;


typedef struct
{
  //*****************************************************************************
  // Memory Global variables
  //*****************************************************************************

  DATA32  SyncTime;
  DATA32  SyncTick;

  DATA8   PathList[MAX_PROGRAMS][vmPATHSIZE];
  POOL    pPoolList[MAX_PROGRAMS][MAX_HANDLES];

  DATA8   Cache[CACHE_DEEPT + 1][vmFILENAMESIZE];

} MEMORY_GLOBALS;

extern MEMORY_GLOBALS MemoryInstance;

#endif /* C_MEMORY_H_ */
