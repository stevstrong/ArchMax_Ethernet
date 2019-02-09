#ifndef FTPFS_H
#define FTPFS_H

//#include "ftpfs.h"
#include <RTClock.h>
#include <SdFat.h>

RTClock rtc;
tm_t mt;


#if USE_SD_FILESYSTEM

// declare variables
//SdFatSdio sd;
SdFat sd;
File file;

//-----------------------------------------------------------------------------
static void FS_GetDateTime(uint16_t* date, uint16_t* time)
{
  // User gets date and time from GPS or real-time clock here
  rtc.getTime(mt);
  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE((mt.year+1970), mt.month, mt.day);
  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(mt.hour, mt.minute, mt.second);
}
//-----------------------------------------------------------------------------
static uint32_t FS_GetFileSize(char* pathfilename)
{
	file = sd.open(pathfilename, FILE_READ);
	if ( !file )
		return 0;

	uint32_t size = file.fileSize();
	file.close();
	return size;
}
//-----------------------------------------------------------------------------
static uint32_t FS_OpenFile(const char * fname, uint8_t oflags)
{
	file.close();

	PRINTF("opening \"%s\" ", fname);
	// listing of root entries
	if ( (file=sd.open(fname, oflags)) ) {
		PRINTF("done.\n");
		return file.fileSize();
	} else {
		PRINTF("failed!\n");
		sd.errorPrint();
		return 0;
	}
}
//-----------------------------------------------------------------------------
static uint16_t FS_ChangeDir(char * path)
{
	PRINTF("change dir to \"%s\" ", path);
	sd.vwd()->rewind();
	if ( !sd.chdir(path) )
	{
		PRINTF("%s", "failed!\n");
		return 0;
	}
	PRINTF("%s", "done.\n");
	return 1;
}
//-----------------------------------------------------------------------------
static void FS_ListRoot(void)
{
	sd.vwd()->rewind();
	sd.chdir("/");
	while (file.openNext(file.cwd(), O_READ))
	{
		file.printFileSize(&Serial);
		Serial.write(' ');
		file.printModifyDateTime(&Serial);
		Serial.write(' ');
		file.printName(&Serial);
		if (file.isDir()) {
			Serial.write('/'); // Indicate a directory.
		}
		Serial.println();
		file.close();
	}
}
//-----------------------------------------------------------------------------
// Writes formatted lines containing list of files of chosen directory.
// The directory must be opened beforehand. 
//-----------------------------------------------------------------------------
static void FS_ListDir(char * path, char * buf, uint16 * buf_len)
{
	if (*path<=' ') { *path++='/'; *path = 0; }
	*buf_len = 0;
	uint32 t = micros();
	//size_t res = FS_OpenFile(path, O_READ);
	//if ( res )
	{
		PRINTF("content of \"%s\":\n", path);
		sd.vwd()->rewind();
		char tmp_str[120];
		int16 len = 0;
		while (file.openNext(file.cwd(), O_READ))
		{
			if (file.isDir())
			{
				len += sprintf(buf+len, "Type=dir;");
			} else {
				len += sprintf(buf+len, "Type=file;Size=%lu;", file.fileSize());
			}
			// get modified timestamp
			file.getModifyDateTime(tmp_str);
			len += sprintf(buf+len, "Modify=%s;", tmp_str);
			len += sprintf(buf+len, "Perm=r;");
			file.getName(tmp_str, sizeof(tmp_str));
			len += sprintf(buf+len, " %s\r\n", tmp_str);
			file.close();
		}
		*buf_len = len;
	}
	file.close();
	t = micros()-t;
	PRINTF("printing done in %u µs.\n", t);
}

#endif // USE_FILESYSTEM



#endif // FTPFS_H
//Added by Sloeber 
#pragma once

