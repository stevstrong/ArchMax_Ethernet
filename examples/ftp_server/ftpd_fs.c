//
// ftpd_fs.c
//

#include "lwip/debug.h"

#include "ftpd_fs.h"

//#include <libmaple/rtc.h>

#ifndef USE_SD
// #include "fsdata.h"
#endif
//#include <stdio.h>
//#include <stdarg.h>
#include <string.h>

#ifndef USE_SD
//-----------------------------------------------------------------------------
extern const struct fsdata_file * fs_files[];
extern const uint32_t FS_NUMFILES;
#endif

//-----------------------------------------------------------------------------
void ftpd_fs_init(void)
{
	PRINTF("<ftpd_fs_init>\n");
#ifdef USE_SD
#else
	PRINTF("> fs_files:\n");
	for (u16_t i = 0; i<FS_NUMFILES; i++) {
		PRINTF("    %u: [%p] %s, %u bytes\n", i, fs_files[i], fs_files[i]->name, fs_files[i]->len);
	}
#endif
}
//-----------------------------------------------------------------------------
bool fs_eof(fs_dir_t * f)
{
#ifdef USE_SD
#else
	return (f->index >= f->len);
#endif
}

//-----------------------------------------------------------------------------
bool fs_exists(const char *filename)
{
#ifdef USE_SD
#else
	for (u16_t i = 0; i<FS_NUMFILES; i++) {
		const char * name = fs_files[i]->name;
		if ( strcmp(filename, name)==0 ) return 1;
	}
	return 0;
#endif
}

//-----------------------------------------------------------------------------
int fs_write(char *filename, u8_t * data, int n)
{
#ifdef USE_SD
#else
	return n;
#endif
}

//-----------------------------------------------------------------------------
fs_dir_t * fs_getdirent(fs_dir_t * entry)
{
	// valid entry?
	if ( entry==NULL) {
		//PRINTF("> ERR: fs_getdirent: entry=NULL!\n");
		return NULL;
	}
#if 0
	PRINTF("> fs_getdirent(%p)\n", entry);
//	PRINTF("entry: %p\n", entry);
	PRINTF("  dataPtr: %p\n", entry->data);
	PRINTF("  len: %u\n", entry->len);
	PRINTF("  index: %u\n", entry->index);
	PRINTF("  pextension: %p\n", entry->pextension);
	if ( entry->pextension ) {
		PRINTF("    name: %s\n", entry->pextension->name);
		PRINTF("    data: %p\n", entry->pextension->data);
		PRINTF("    len: %u\n", entry->pextension->len);
		PRINTF("    flags: %04X\n", entry->pextension->flags);
	}
#endif

#ifdef USE_SD
#else
	// no previous entry?
	if ( entry->pextension==NULL) {
		// return first file
		PRINTF("> fs_getdirent: file index 0\n");
		if ( fs_open(entry, fs_files[0]->name)==ERR_OK )
			return entry;
		else
			return NULL;
	}

	// get index of the file and return the next file
	u16_t i;
	for (i=0; i<FS_NUMFILES; i++) {
		if ( entry->pextension==fs_files[i] )
			break;
	}
	PRINTF("> fs_getdirent: current file index %u\n", i);
	if ( i<(FS_NUMFILES-1) ) {
		//PRINTF("> fs_getdirent: trying to open '%s'\n", fs_files[i+1]->name);
		if ( fs_open(entry, fs_files[i+1]->name)==ERR_OK )
			return entry;
		else
			return NULL;
	} else
		return NULL;
#endif
}

//-----------------------------------------------------------------------------
err_t fs_opendir(char* dir)
{
	PRINTF("<fs_opendir(%s)>\n", dir);
#ifdef USE_SD
#else
	return ERR_OK;
#endif
}

//-----------------------------------------------------------------------------
err_t fs_closedir(char* dir)
{
	PRINTF("<fs_closedir(%s)>\n", dir);
#ifdef USE_SD
#else
	return ERR_OK;
#endif
}

//-----------------------------------------------------------------------------
err_t fs_chdir(char *src_dir, const char *dest_dir)
{
#ifdef USE_SD
#else
	return ERR_OK;
#endif
}

//-----------------------------------------------------------------------------
//typedef struct fs_stat {
//	char name[128];
//	uint32_t size;
//	time_t mtime;
//	uint32_t flags;
//} fs_stat_t;
//-----------------------------------------------------------------------------
// struct fs_file {                // == fs_dir_t
//  const char *data;
//  int len;
//  int index;
//  fs_file_extension *pextension; // points to "struct fsdata_file"
//  u8_t flags;
//}
//-----------------------------------------------------------------------------
const uint32_t dummy_now = 1547907475;
//-----------------------------------------------------------------------------
err_t fs_getstat(fs_dir_t * dir, fs_stat_t * stat)
{
#ifdef USE_SD
#else
	strncpy(stat->name, dir->pextension->name, FS_STAT_NAME_SIZE);
	stat->size = dir->len;
	stat->mtime = dummy_now;
	stat->flags &= (~FS_FILE_FLAG_IS_DIR);

	return ERR_OK;
#endif
}

//-----------------------------------------------------------------------------
err_t fs_rename(const char *filename, const char *newname)
{
#ifdef USE_SD
#else
	return ERR_OK;
#endif
}

//-----------------------------------------------------------------------------
err_t fs_remove(const char *filename)
{
#ifdef USE_SD
#else
	return ERR_OK;
#endif
}

//-----------------------------------------------------------------------------
err_t fs_rmdir(const char *filename)
{
#ifdef USE_SD
#else
	return ERR_OK;
#endif
}

//-----------------------------------------------------------------------------
err_t fs_mkdir(const char *filename)
{
#ifdef USE_SD
#else
	return ERR_OK;
#endif
}

