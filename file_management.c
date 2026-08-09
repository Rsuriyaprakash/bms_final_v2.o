/** @file file_management.c @brief Generic binary persistence implementation. */
#include "file_management.h"
#include "logging.h"
#include "config.h"
#include "file_names.h"
#include "utility.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define BMS_MKDIR(path) _mkdir(path)
#else
#define BMS_MKDIR(path) mkdir((path), 0777)
#endif

BmsStatus_t FileManagementEnsureDataDirectory(void)
{
    struct stat info;
    if (stat(BMS_DATA_DIRECTORY, &info) == 0)
    { return BMS_STATUS_OK; }
    return (BMS_MKDIR(BMS_DATA_DIRECTORY) == 0) ? BMS_STATUS_OK : BMS_STATUS_FILE_ERROR;
}
BmsStatus_t FileManagementInitialize(void){return FileManagementEnsureDataDirectory();}

BmsStatus_t FileManagementWriteRecords(const char *filePath,const void *records,
 uint32_t recordCount,size_t recordSize)
{
    FILE *file; BmsFileHeader_t header; size_t written;
    if((filePath==NULL)||(recordSize==0U)||((recordCount>0U)&&(records==NULL)))
    return BMS_STATUS_INVALID_ARGUMENT;
    file=fopen(filePath,"wb");
    if(file==NULL)
    {
        BMS_LOG_ERROR("FILE", "Open for write failed: path=%s", filePath);
        return BMS_STATUS_FILE_ERROR;
    }
    header.magic=BMS_FILE_MAGIC;header.formatVersion=BMS_FILE_FORMAT_VERSION;
    header.recordSize=(uint16_t)recordSize;header.recordCount=recordCount;
    header.checksum=(recordCount==0U)?0U:UtilityCalculateChecksum(records,(size_t)recordCount*recordSize);
    if(fwrite(&header,sizeof(header),1U,file)!=1U){fclose(file);return BMS_STATUS_WRITE_ERROR;}
    written=(recordCount==0U)?0U:fwrite(records,recordSize,recordCount,file);
    if((recordCount>0U)&&(written!=recordCount)){fclose(file);return BMS_STATUS_WRITE_ERROR;}
    if (fclose(file) == 0)
    {
        BMS_LOG_DEBUG("FILE",
                      "Records written: path=%s count=%u size=%u",
                      filePath,
                      (unsigned int)recordCount,
                      (unsigned int)recordSize);
        return BMS_STATUS_OK;
    }
    BMS_LOG_ERROR("FILE", "File close failed after write: path=%s", filePath);
    return BMS_STATUS_FILE_ERROR;
}
BmsStatus_t FileManagementReadRecords(const char *filePath,void *records,
 uint32_t capacity,uint32_t *recordCount,size_t recordSize)
{
    FILE *file;BmsFileHeader_t header;size_t read;
    if((filePath==NULL)||(records==NULL)||(recordCount==NULL)||(recordSize==0U))
    return BMS_STATUS_INVALID_ARGUMENT;
    file=fopen(filePath,"rb");
    if(file==NULL)
    {
        BMS_LOG_DEBUG("FILE", "File not found: path=%s", filePath);
        return BMS_STATUS_FILE_NOT_FOUND;
    }
    if(fread(&header,sizeof(header),1U,file)!=1U){fclose(file);return BMS_STATUS_READ_ERROR;}
    if((header.magic!=BMS_FILE_MAGIC)||(header.recordSize!=(uint16_t)recordSize)||
       (header.recordCount>capacity)){fclose(file);return BMS_STATUS_FILE_CORRUPT;}
    read=(header.recordCount==0U)?0U:fread(records,recordSize,header.recordCount,file);
    fclose(file);if(read!=header.recordCount)return BMS_STATUS_READ_ERROR;
    if((header.recordCount>0U)&&(UtilityCalculateChecksum(records,(size_t)header.recordCount*recordSize)!=header.checksum))
    return BMS_STATUS_FILE_CORRUPT;
    *recordCount=header.recordCount;
    BMS_LOG_DEBUG("FILE",
                  "Records read: path=%s count=%u",
                  filePath,
                  (unsigned int)header.recordCount);
    return BMS_STATUS_OK;
}
BmsStatus_t FileManagementAppendRecord(const char *filePath,const void *record,size_t recordSize)
{
    BmsFileHeader_t header;void *buffer;uint32_t count;
    BmsStatus_t status=FileManagementGetRecordCount(filePath,recordSize,&count);
    if(status==BMS_STATUS_FILE_NOT_FOUND)return FileManagementWriteRecords(filePath,record,1U,recordSize);
    if(status!=BMS_STATUS_OK)return status;
    buffer=calloc((size_t)count+1U,recordSize);if(buffer==NULL)return BMS_STATUS_MEMORY_ERROR;
    status=FileManagementReadRecords(filePath,buffer,count,&header.recordCount,recordSize);
    if(status==BMS_STATUS_OK){(void)memcpy((uint8_t*)buffer+(size_t)count*recordSize,record,recordSize);status=FileManagementWriteRecords(filePath,buffer,count+1U,recordSize);}
    free(buffer);return status;
}
BmsStatus_t FileManagementVisitRecords(const char *filePath,size_t recordSize,
 BmsFileRecordVisitor_t visitor,void *context)
{
    FILE *file;BmsFileHeader_t header;void *record;uint32_t i;BmsStatus_t status;
    if((filePath==NULL)||(recordSize==0U)||(visitor==NULL))return BMS_STATUS_INVALID_ARGUMENT;
    file=fopen(filePath,"rb");if(file==NULL)return BMS_STATUS_FILE_NOT_FOUND;
    if(fread(&header,sizeof(header),1U,file)!=1U){fclose(file);return BMS_STATUS_READ_ERROR;}
    record=malloc(recordSize);if(record==NULL){fclose(file);return BMS_STATUS_MEMORY_ERROR;}
    status=BMS_STATUS_OK;for(i=0U;i<header.recordCount;++i){if(fread(record,recordSize,1U,file)!=1U){status=BMS_STATUS_READ_ERROR;break;}status=visitor(record,i,context);if(status!=BMS_STATUS_OK)break;}
    free(record);fclose(file);return status;
}
BmsStatus_t FileManagementGetRecordCount(const char *filePath,size_t recordSize,uint32_t *recordCount)
{
    FILE *file;BmsFileHeader_t header;
    if((filePath==NULL)||(recordCount==NULL)||(recordSize==0U))return BMS_STATUS_INVALID_ARGUMENT;
    file=fopen(filePath,"rb");if(file==NULL)return BMS_STATUS_FILE_NOT_FOUND;
    if(fread(&header,sizeof(header),1U,file)!=1U){fclose(file);return BMS_STATUS_READ_ERROR;}fclose(file);
    if((header.magic!=BMS_FILE_MAGIC)||(header.recordSize!=(uint16_t)recordSize))return BMS_STATUS_FILE_CORRUPT;
    *recordCount=header.recordCount;return BMS_STATUS_OK;
}
BmsStatus_t FileManagementDeleteFile(const char *filePath){if(filePath==NULL)return BMS_STATUS_INVALID_ARGUMENT;return(remove(filePath)==0)?BMS_STATUS_OK:BMS_STATUS_FILE_ERROR;}
BmsStatus_t FileManagementFileExists(const char *filePath,bool *exists){FILE *file;if((filePath==NULL)||(exists==NULL))return BMS_STATUS_INVALID_ARGUMENT;file=fopen(filePath,"rb");*exists=(file!=NULL);if(file!=NULL)fclose(file);return BMS_STATUS_OK;}
