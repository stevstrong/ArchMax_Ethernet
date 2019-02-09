// 
// ftpd_fs.h
// 

#ifndef FTPD_FS_H
#define FTPD_FS_H

#ifdef __cplusplus
extern "C" {
#endif


#define FS_FILE_FLAG_IS_DIR              0x10
#define FS_FILE_IS_DIR(x) ( (x)->flags&FS_FILE_FLAG_IS_DIR )

#define FS_STAT_NAME_SIZE   128

typedef struct fs_stat {
	char name[FS_STAT_NAME_SIZE];
	uint32_t size;
	time_t mtime;
	uint32_t flags;
} fs_stat_t;


#define FS_FILE_EXTENSION_T_DEFINED
typedef const struct fsdata_file fs_file_extension;


#include "lwip/apps/fs.h"


//-----------------------------------------------------------------------------
#ifdef USE_SD
  // FatFile struct pointer
  #define fs_dir_t FatFile
#else
  // fs_file struct poiner 
  #define fs_dir_t struct fs_file
#endif

extern void ftpd_fs_init(void);
//-----------------------------------------------------------------------------
extern bool fs_eof(fs_dir_t *);
extern bool fs_exists(const char *filename);
extern int fs_write(char *filename, u8_t *, int);
//extern void fs_close_file(struct fs_file*);
fs_dir_t * fs_getdirent(fs_dir_t*);
extern err_t fs_opendir(char*);
extern err_t fs_closedir(char*);
extern err_t fs_chdir(char *src_dir, const char *dest_dir);
extern err_t fs_getstat(fs_dir_t *, fs_stat_t *);
extern err_t fs_rename(const char *filename, const char*newname);
extern err_t fs_remove(const char *filename);
extern err_t fs_rmdir(const char *filename);
extern err_t fs_mkdir(const char *filename);
//extern err_t fs_getmtime(const char *filename, tm_t *);//struct fs_file*);   // get it from fs_stat
//extern err_t fs_getsize(const char*filename, uint32_t *);//struct fs_file*); // get it from fs_stat


#ifdef __cplusplus
}
#endif

#endif // FTPD_FS_H


//Added by Sloeber 
#pragma once

