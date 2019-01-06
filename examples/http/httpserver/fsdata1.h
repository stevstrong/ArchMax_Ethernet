#ifndef __FSDATA_H__
#define __FSDATA_H__

#include "lwip/opt.h"
#include "lwip/def.h"
//#include "fs.h"
#include "lwip/apps/fs.h"


// struct fsdata_file {
//   const char *name;
//   const unsigned char *data;
//   unsigned int len;
//   unsigned int flags;
// };

static const char name_favicon[] = "/favicon.ico";
static const unsigned char data_favicon[] = {
#include "fs/favicon.h"
};

static const char name_logo_jpg[] = "/files/logo.jpg";
static const unsigned char data_logo_jpg[] = {
#include "fs/logo_jpg.h"
};

static const char name_ST_gif[] = "/files/ST.gif";
static const unsigned char data_ST_gif[] = {
#include "fs/ST_gif.h"
};

static const char name_stm32_jpg[] = "/files/stm32.jpg";
static const unsigned char data_stm32_jpg[] = {
#include "fs/stm32_jpg.h" 
};

//static const char name_404_html[] = "/404.html";
//static const unsigned char data_404_html[] = {
//#include "fs/404_html.h"
//};

static const char name_index_html[] = "/index.html";
static const unsigned char data_index_html[] = {
#include "fs/index_htm.h"
};

static const char name_ADC_shtml[] = "/ADC.shtml";
static const unsigned char data_ADC_shtml[] = {
#include "fs/ADC_shtml.h"
};

static const char name_LED_html[] = "/LED.html";
static const unsigned char data_LED_html[] = {
#include "fs/LED_html.h"
};


#define file_NULL (struct fsdata_file *) NULL

const struct fsdata_file file_favicon = {
name_favicon,
data_favicon,
sizeof(data_favicon),
0,
};

const struct fsdata_file file_logo_jpg = {
name_logo_jpg,
data_logo_jpg,
sizeof(data_logo_jpg),
0,
};

const struct fsdata_file file_ST_gif = {
name_ST_gif,
data_ST_gif,
sizeof(data_ST_gif),
0,
};

const struct fsdata_file file_stm32_jpg = {
name_stm32_jpg,
data_stm32_jpg,
sizeof(data_stm32_jpg),
0,
};

//const struct fsdata_file file_404_html = {
//name_404_html,
//data_404_html,
//sizeof(data_404_html),
//0,
//};

const struct fsdata_file file_index_html = {
name_index_html,
data_index_html,
sizeof(data_index_html),
0,
};

const struct fsdata_file file_ADC_shtml = {
name_ADC_shtml,
data_ADC_shtml,
sizeof(data_ADC_shtml),
0,
};

const struct fsdata_file file_LED_html = {
name_LED_html,
data_LED_html,
sizeof(data_LED_html),
0,
};

const struct fsdata_file * fs_files[] = {
	&file_index_html,
	&file_LED_html,
	&file_ADC_shtml,
//	&file_404_html,
	&file_ST_gif,
	&file_stm32_jpg,
	&file_logo_jpg,
	&file_favicon
};

//#define FS_NUMFILES 7
#define FS_NUMFILES ( sizeof(fs_files)/sizeof(struct fsdata_file *) )


#endif /* __FSDATA_H__ */
