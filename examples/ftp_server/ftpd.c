/*
 * Copyright (c) 2002 Florian Schulze.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the authors nor the names of the contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * ftpd.c - This file is part of the FTP daemon for lwIP
 *
 */

#include "lwip/debug.h"

#include "lwip/stats.h"

#include "ftpd.h"

#include "lwip/tcp.h"

#include <stdio.h>
#include <stdarg.h>
//#include <ctype.h>
#include <string.h>

#include <libmaple/rtc.h>


//-----------------------------------------------------------------------------
#define FTPD_DEBUG  1

//-----------------------------------------------------------------------------

#ifdef FTPD_DEBUG
//int dbg_printf(const char *fmt, ...);
#define dbg_printf PRINTF
#else
#ifdef _MSC_VER
#define dbg_printf(x) /* x */
#else
#define dbg_printf(f, ...) /* */
#endif
#endif

#ifndef ftpd_logd
#define ftpd_logd(fmt, ...) dbg_printf(fmt "\n", ## __VA_ARGS__)
#endif
#ifndef ftpd_logi
#define ftpd_logi(fmt, ...) dbg_printf(fmt "\n", ## __VA_ARGS__)
#endif
#ifndef ftpd_logw
#define ftpd_logw(fmt, ...) dbg_printf(fmt "\n", ## __VA_ARGS__)
#endif
#ifndef ftpd_loge
#define ftpd_loge(fmt, ...) dbg_printf(fmt "\n", ## __VA_ARGS__)
#endif

//#define EINVAL 1
//#define ENOMEM 2
//#define ENODEV 3

/*
	110 Restart marker reply.
	In this case, the text is exact and not left to the particular implementation;
	it must read:
		MARK yyyy = mmmm
	Where
		yyyy is User-process data stream marker, and
		mmmm is server's equivalent marker (note the spaces between markers and "=").
*/
#define msg110 "110 MARK %s = %s."
#define msg120 "120 Service ready in nnn minutes."
#define msg125 "125 Data connection already open; transfer starting."
#define msg150 "150 File status okay; about to open data connection."
//#define msg150recv "150 Opening BINARY mode data connection for %s (%u bytes)."
#define msg150recv "150 Opening BINARY mode data to send %s (%u bytes)."
#define msg150stor "150 Opening BINARY mode data connection for %s."
#define msg200 "200 Command okay."
#define msg202 "202 Command not implemented, superfluous at this site."
#define msg211 "211 System status, or system help reply."
#define msg212 "212 Directory status."
#define msg213 "213 File status."
/*
	214 Help message.
	On how to use the server or the meaning of a particular non-standard command.
	This reply is useful only to the human user.
*/
#define msg214 "214 %s."
#define msg214SYST "214 %s system type."
/*
	215 NAME system type.
	Where NAME is an official system name from the list in the assigned numbers document.
*/
/*
	220 Service ready for new user.
*/
#define msg220 "220 lwIP FTP Server ready."
/*
	221 Service closing control connection.
             Logged out if appropriate.
*/
#define msg221 "221 Goodbye."
#define msg225 "225 Data connection open; no transfer in progress."
/*
	Requested file action successful (for example, file transfer or file abort).
*/
#define msg226 "226 Closing data connection."
/*
	227 Entering Passive Mode (h1,h2,h3,h4,p1,p2).
*/
#define msg227 "227 Entering Passive Mode (%i,%i,%i,%i,%i,%i)."
#define msg230 "230 User logged in, proceed."
#define msg250 "250 Requested file action okay, completed."
#define msg257PWD "257 \"%s\" is current directory."
/*
	257 "PATHNAME" created.
*/
#define msg257 "257 \"%s\" created."
#define msg331 "331 User name okay, need password."
#define msg332 "332 Need account for login."
#define msg350 "350 Requested file action pending further information."
/*
	This may be a reply to any command if the service knows it must shut down.
*/
#define msg421 "421 Service not available, closing control connection."
#define msg425 "425 Can't open data connection."
#define msg426 "426 Connection closed; transfer aborted."
/*
	File unavailable (e.g., file busy).
*/
#define msg450 "450 Requested file action not taken."
#define msg451 "451 Requested action aborted: local error in processing."
/*
	Insufficient storage space in system.
*/
#define msg452 "452 Requested action not taken."
/*
	This may include errors such as command line too long.
*/
#define msg500 "500 Syntax error, command unrecognized."
#define msg501 "501 Syntax error in parameters or arguments."
#define msg502 "502 Command not implemented."
#define msg503 "503 Bad sequence of commands."
#define msg504 "504 Command not implemented for that parameter."
#define msg530 "530 Not logged in."
#define msg532 "532 Need account for storing files."
/*
	File unavailable (e.g., file not found, no access).
*/
#define msg550 "550 Requested action not taken."
#define msg551 "551 Requested action aborted: page type unknown."
/*
	Exceeded storage allocation (for current directory or dataset).
*/
#define msg552 "552 Requested file action aborted."
/*
	File name not allowed.
*/
#define msg553 "553 Requested action not taken."

static const char *month_table[12] = {
"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

//-----------------------------------------------------------------------------
//	SFIFO 1.3
//-----------------------------------------------------------------------------
//#define SFIFO_BUFFERS       4
#define	SFIFO_BUFFER_SIZE	(4*1024) //(2*1460) //2048
//uint8_t fifo_buffers[SFIFO_BUFFERS][SFIFO_BUFFER_SIZE];
//uint8_t fifo_buf_state[SFIFO_BUFFERS];

typedef struct fifo {
	char *buffer __attribute__((aligned(4)));
	uint16_t size;		// Number of bytes
	uint16_t pos;		// rd/wr position
} fifo_t;

#define fifo_used(x)	((x).pos)
#define fifo_space(x)	((x).size - (x).pos)// - 1)
#define fifo_dptr(x)	((x).buffer + (x).pos)

#define FSD_FIFO_DPTR  fifo_dptr(fsm->data_fifo)
#define FSM_FIFO_DPTR  fifo_dptr(fsm->msg_fifo)
#define FSD_FIFO_USED  fifo_used(fsm->data_fifo)
#define FSM_FIFO_USED  fifo_used(fsm->msg_fifo)
#define FSD_FIFO_SPACE fifo_space(fsm->data_fifo)
#define FSM_FIFO_SPACE fifo_space(fsm->msg_fifo)
//
//#define SFIFO_SIZEMASK(x)	((x)->size - 1)
//

enum ftpd_state_e {
	FTPD_USER,
	FTPD_PASS,
	FTPD_IDLE,
	FTPD_NLST,
	FTPD_LIST,
	FTPD_RETR,
	FTPD_RNFR,
	FTPD_STOR,
	FTPD_QUIT
};

// forward declarations
typedef struct ftpd_datastate ftpd_datastate_t;
typedef struct ftpd_msgstate ftpd_msgstate_t;

#include "ftpd_fs.h"

typedef struct ftpd_conn {
	enum ftpd_state_e state;
	u8_t passive;
	u16_t dataport;
	struct ip4_addr dataip;
	fs_stat_t *fs_stat;    // file status info - name, size, mdate, flags
	fs_dir_t *dir_entry;   // file or dir entry
	u32_t data_connected;
	struct tcp_pcb *msgpcb;
	struct tcp_pcb *datapcb;
	struct tcp_pcb *datalistenpcb;
	char crt_dir[512] __attribute__((aligned(4)));
	fifo_t msg_fifo;
	fifo_t data_fifo;
} ftpd_conn_t;

static ftpd_conn_t ftpd_fsm;

static fs_dir_t fs_file;
static fs_stat_t f_stat;

//static const char username[] = "stm32";
//static const char pasword[] = "duino";

static char msg_buf[512] __attribute__((aligned(4)));
static char data_buf[SFIFO_BUFFER_SIZE] __attribute__((aligned(4)));

//-----------------------------------------------------------------------------
static void ftpd_dataclose(ftpd_conn_t *fsm);


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void DBG_PrintFSM(ftpd_conn_t *fsm)
{
	return;
//typedef struct ftpd_conn {
	PRINTF("<- ftpd_fsm -\n");
//	enum ftpd_state_e state;
	PRINTF("state: %u\n", fsm->state);
//	u8_t passive;
	PRINTF("passive: %u\n", fsm->passive);
//	u16_t dataport;
	PRINTF("dataport: %u\n", fsm->dataport);
//	struct ip4_addr dataip;
	delay(10);
	PRINTF("dataip: %u\n", (uint32_t)fsm->dataip.addr);
//	fs_stat_t *fs_stat;    // file status info - name, size, mdate, flags
	PRINTF("fs_stat: %08X\n", (uint32_t)fsm->fs_stat);
	PRINTF("  name: %s\n", fsm->fs_stat->name);
	PRINTF("  size: %u\n", fsm->fs_stat->size);
	PRINTF("  mtime: %lli\n", fsm->fs_stat->mtime);
	PRINTF("  flags: %04X\n", fsm->fs_stat->flags);
	return;
//	fs_dir_t *dir_entry;   // file or dir entry
	PRINTF("dir_entry: %08X\n", (uint32_t)fsm->dir_entry);
	PRINTF("  dataPtr: %08X\n", (uint32_t)fsm->dir_entry->data);
	PRINTF("  len: %u\n", fsm->dir_entry->len);
	PRINTF("  index: %u\n", fsm->dir_entry->index);
	PRINTF("  pextension: %08X\n", (uint32_t)fsm->dir_entry->pextension);
	PRINTF("  flags: %04X\n", fsm->fs_stat->flags);
//	u32_t data_connected;
	PRINTF("data_connected: %u\n", fsm->data_connected);
//	struct tcp_pcb *msgpcb;
	PRINTF("msgpcb: %08X\n", (uint32_t)fsm->msgpcb);
//	struct tcp_pcb *datapcb;
	PRINTF("datapcb: %08X\n", (uint32_t)fsm->datapcb);
//	struct tcp_pcb *datalistenpcb;
	PRINTF("datalistenpcb: %08X\n", (uint32_t)fsm->datalistenpcb);
//	char crt_dir[512];
	PRINTF("crt_dir: %s\n", fsm->crt_dir);
//	fifo_t msg_fifo;
	PRINTF("msg_fifo:\n");
	PRINTF("  buffer: %s\n", fsm->msg_fifo.buffer);
	PRINTF("  size: %u\n", fsm->msg_fifo.size);
	PRINTF("  pos: %u\n", fsm->msg_fifo.pos);
//	fifo_t data_fifo;
	PRINTF("data_fifo:\n");
	PRINTF("  buffer: %s\n", fsm->data_fifo.buffer);
	PRINTF("  size: %u\n", fsm->data_fifo.size);
	PRINTF("  pos: %u\n", fsm->data_fifo.pos);
//} ftpd_conn_t;
	PRINTF(" - ftpd_fsm ->\n");
}

//-----------------------------------------------------------------------------
static void send_msgdata(ftpd_conn_t *fsm)
{
	u16_t len = FSM_FIFO_USED;
	PRINTF("<send_msgdata(%u)>\n", len);
	if ( len == 0) return;

	DBG_PrintFSM(fsm);

	if ( !fsm->msgpcb ) {
		PRINTF("> ERR: send_msgdata: msgpcb=NULL!\n");
		return;
	}

	uint16_t tcp_len = tcp_sndbuf(fsm->msgpcb);
	// We cannot send more data than space available in the send buffer.
	if ( len > tcp_len ) {
		len = tcp_len;
		PRINTF("> INFO: send_msgdata: len(%u) > tcp_sndbuf(%u)\n", len, tcp_len);
	}

	err_t err = tcp_write(fsm->msgpcb, fsm->msg_fifo.buffer, len, 1);
	if (err != ERR_OK) {
		ftpd_loge("send_msgdata: tcp_write error!");
		return;
	}

	len = FSM_FIFO_USED - len;
	if ( len ) { // if there is more data to be send, move it to begin
		PRINTF("> INFO: send_data: moved %u residual data\n", len);
		memcpy(fsm->msg_fifo.buffer, FSM_FIFO_DPTR, len);
	}
	FSM_FIFO_USED = len; // update fifo index
	PRINTF(" send_msgdata: FIFO_len=%u\n", len);
}

//-----------------------------------------------------------------------------
static void send_msg(ftpd_conn_t *fsm, char *msg, ...)
{
	// check available space in fifo
	int len = FSM_FIFO_SPACE - 2; // - \r\n
	if ( len < 256 ) { // expected size of msg = 256 (half buffer)
		PRINTF("> ERR: send_msg: message won't fit into fifo: %i!\n", len);
		return;
	}

	char *buffer = FSM_FIFO_DPTR;

	va_list args;
	va_start(args, msg);
	len = vsnprintf(buffer, len, msg, args);
	va_end(args);

	if (len < 0) {
		PRINTF("> ERR: send_msg: snprintf returned: %i!\n", len);
		return;
	}
	buffer += len;
	// add \r\n to end of msg
	*buffer++ = '\r';
	*buffer++ = '\n';
	*buffer = 0;
	
	len +=2;
	ftpd_logi("< sending %u bytes: \"%s\"", len, FSM_FIFO_DPTR);
	FSM_FIFO_USED += len;
	
	send_msgdata(fsm);
}

//-----------------------------------------------------------------------------
static void send_data(ftpd_conn_t *fsm)
{
	if ( !fsm->datapcb ) {
		PRINTF("> ERR: send_data: datapcb=NULL!\n");
		return;
	}
	uint16_t len = FSD_FIFO_USED;
	uint16_t tcp_len = tcp_sndbuf(fsm->datapcb);
	PRINTF("<send_data(%p, len=%u, tcp_len=%u)>\n", fsm, len, tcp_len);
	if ( len == 0 )
		return; // nothing to send
	
	// We cannot send more data than space available in the send buffer.
	if ( len > tcp_len )
		len = tcp_len;

	err_t err = tcp_write(fsm->datapcb, fsm->data_fifo.buffer, len, 1); // copy
	if (err != ERR_OK) {
		ftpd_loge("> ERR: send_data: tcp_write error!");
		return;
	}
	
	uint16_t dif = FSD_FIFO_USED - len; // data left
	if (dif) {
		FSD_FIFO_USED = len;
		PRINTF("> INFO: send_data moved %u left bytes\n", dif);
		memcpy(fsm->data_fifo.buffer, FSD_FIFO_DPTR, dif);
	}
	FSD_FIFO_USED = dif; // update index
}

//-----------------------------------------------------------------------------
//static void send_file(ftpd_conn_t *fsm, struct tcp_pcb *pcb)
static void send_file(ftpd_conn_t *fsm)
{
	PRINTF("<send_file(%p)>\n", fsm);
	if (!fsm->data_connected) {
		PRINTF("> ERR: send_file: data_connected=0!\n");
		return;
	}
//	if ( !fsm->datapcb ) {
//		PRINTF("> ERR: send_file: datapcb=NULL!\n");
//		return;
//	}

	if (fsm->dir_entry->data) {
		// file still active
		int len = FSD_FIFO_SPACE;
		if (len) {
			len = fs_read(fsm->dir_entry, FSD_FIFO_DPTR, len);
			//if (len==0 || fs_eof(fsm->dir_entry) ) { // nothing read or EOF?
			if ( len<=0 ) { // nothing read or EOF?
				len = 0;
				PRINTF("> INFO: send_file: EOF\n");
				fs_close(fsm->dir_entry);
				//fsm->dir_entry = NULL;
				fsm->dir_entry->data = NULL;
				//return;
			}
			//fifo_write(&fsd->fifo, buffer, len);
			FSD_FIFO_USED += len;
		}
		send_data(fsm);
		//free(buffer);
	} else {
		// finish file send
		PRINTF("> finishing send_file\n");
		if ( FSD_FIFO_USED ) { // send residual data
			send_data(fsm);
			return;
		}

		//if (fsm->dir_entry) // superfluous ?
		//	fs_close(fsm->dir_entry), fsm->dir_entry = NULL;
		ftpd_dataclose(fsm);
		//ftpd_conn_t *fsm = fsm->msgfs;
		fsm->datapcb = NULL;
		fsm->state = FTPD_IDLE;
		send_msg(fsm, msg226);
		return;
	}
}

//-----------------------------------------------------------------------------
static void send_next_directory(ftpd_conn_t *fsm, int shortlist)
{
	PRINTF("<send_next_directory(%p, %u)>\n", fsm, shortlist); delay(10);
	// loop till fifo full, then send fifo data
	while (1) {
		//if (fsd->dir_ent<0)
		//	fsd->dir_ent = 0;

		fs_dir_t *tmp_file;
		tmp_file = fs_getdirent(fsm->dir_entry);
		if ( tmp_file ) {
			// read file stats (name, size, type, time, date)
			//fs_stat_t st;
			fs_getstat(tmp_file, fsm->fs_stat);
			PRINTF("> found dir entry: %s\n", fsm->fs_stat->name);
			u32_t len = strlen(fsm->fs_stat->name);

			if (shortlist) { // NLST - short list, only names
				// check if file name fits into buffer
				if ( FSD_FIFO_SPACE < (len+3) ) { // file name + \r\n\000
					PRINTF("> INFO: send_next_directory: low FIFO space: %u, need: %u\n", FSD_FIFO_SPACE, len+3);
					send_data(fsm); // not enough space, free some by sending data
					break;
				}
				// write data to fifo
				len = sprintf(FSD_FIFO_DPTR, "%s\r\n", fsm->fs_stat->name);

			} else { // LIST - detailed dir entry info
				// check if there is enough space in fifo buffer
				if ( FSD_FIFO_SPACE < (u16_t)(70+len)) { // expected length of a sprintf containing file name
					PRINTF("> INFO: send_next_directory: low FIFO space: %u, need: %u\n", FSD_FIFO_SPACE, len+3);
					send_data(fsm); // wont't fit, free up some space in buffer by sending data
					break;
				}

				//time_t current_time = now(); // epoch time from RTC
				int current_year = yearNow();

				tm_t *s_time = getTimePtr(fsm->fs_stat->mtime);
				
				// write data to fifo
				char * buf = FSD_FIFO_DPTR;
				if (s_time->year == current_year) {
					len = sprintf(buf, "-rw-rw-rw-   1 user     ftp  %11d %3s %02i %02i:%02i %s\r\n",
							fsm->fs_stat->size, month_table[s_time->month], s_time->day, s_time->hour, s_time->minute, fsm->fs_stat->name);
				} else {
					len = sprintf(buf, "-rw-rw-rw-   1 user     ftp  %11d %3s %02i %5i %s\r\n",
							fsm->fs_stat->size, month_table[s_time->month], s_time->day, s_time->year + 1900, fsm->fs_stat->name);
				}
				if (FS_FILE_IS_DIR(tmp_file))
					*buf = 'd';
			}
			// update fifo index and dir entry
			FSD_FIFO_USED += len;
			//fsd->dir_ent ++;
		} else { // no more dir entries to send
			//struct tcp_pcb *msgpcb;
			fs_close(fsm->dir_entry);
			//fsm->dir_entry = NULL;

			if (FSD_FIFO_USED > 0) { // send residual data
				send_data(fsm);
				//free(buffer);
				break;
			}
			//ftpd_msgstate_t *fsm = fsd->msgfs;
			//msgpcb = fsm->msgpcb;

			ftpd_dataclose(fsm);
			fsm->datapcb = NULL;
			fsm->state = FTPD_IDLE;
			send_msg(fsm, msg226);
			//free(buffer);
			break;
		}
	}
}

//-----------------------------------------------------------------------------
static err_t ftpd_datasent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
	ftpd_conn_t *fsm = arg;
	PRINTF("<ftpd_datasent(%p, %p, %u); state=%u,%s>\n", arg, pcb, len,
		fsm->state, (fsm->state==FTPD_LIST)?"FTPD_LIST":((fsm->state==FTPD_NLST)?"FTPD_NLST":((fsm->state==FTPD_RETR)?"FTPD_RETR":"NO_ACT")));
	switch (fsm->state) {
	case FTPD_LIST:
		send_next_directory(fsm, 0);
		break;
	case FTPD_NLST:
		send_next_directory(fsm, 1);
		break;
	case FTPD_RETR:
		send_file(fsm);
		break;
	default:
		break;
	}

	return ERR_OK;
}

//-----------------------------------------------------------------------------
static err_t ftpd_datarecv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
	PRINTF("<ftpd_datarecv(%p, %p, %p, %u)>\n", arg, pcb, p, err);
	if ( err )
		PRINTF("> ftpd_datarecv ERROR\n");

	ftpd_conn_t *fsm = arg;
	if ( p != NULL )
	{
		struct pbuf *q;
		u16_t tot_len = 0;

		for (q = p; q != NULL; q = q->next) {
			int len = fs_write(fsm->fs_stat->name, q->payload, q->len);
			tot_len += len;
			if (len != q->len)
				break;
		}

		// Inform TCP that we have taken the data
		tcp_recved(pcb, tot_len);
		pbuf_free(p);
	}
	else // ( p == NULL )
	{

		fs_close(fsm->dir_entry);
		//fsm->dir_entry = NULL;
		ftpd_dataclose(fsm);
		if (pcb == fsm->datapcb ) {//&& fsd == old_datafs) {
			fsm->datapcb = NULL;
			fsm->state = FTPD_IDLE;
		} else {
			ftpd_logw("data connection closed in ftpd_datarecv but it is different "
				"from the one in fsm: pcb=%p, fsm->datapcb=%p", pcb, fsm->datapcb);
		}
		send_msg(fsm, msg226);
	}
	return ERR_OK;
}

//-----------------------------------------------------------------------------
static void ftpd_dataerr(void *arg, err_t err)
{
	PRINTF("<ftpd_dataerr(%p, %u)>\n", arg, err);
	ftpd_conn_t *fsm = arg;

	ftpd_loge("ftpd_dataerr: %s (%i)", lwip_strerr(err), err);
	if (fsm == NULL)
		return;
	//fsm->datafs = NULL;
	fsm->state = FTPD_IDLE;
//	free(fsd);
}

//-----------------------------------------------------------------------------
static err_t ftpd_data_conn_accepted(void *arg, struct tcp_pcb *pcb, err_t err)
{
	PRINTF("<ftpd_data_conn_accepted(%p, %p, %u)>\n", arg, pcb, err);
	ftpd_conn_t *fsm = arg;

	fsm->datapcb = pcb;
	fsm->data_connected = 1;

	tcp_err(pcb, ftpd_dataerr);
	// Tell TCP that we wish to be informed of incoming data
	tcp_recv(pcb, ftpd_datarecv);

	// Tell TCP that we wish be to informed of data sent and error status
	tcp_sent(pcb, ftpd_datasent);

	// initiate data transmission
	return ftpd_datasent(fsm, pcb, 0); // third parameter (0) is not relevant, not used
}

//-----------------------------------------------------------------------------
static void ftpd_dataclose(ftpd_conn_t *fsm)
{
	PRINTF("<ftpd_dataclose(%p)>\n", fsm);
	FSM_FIFO_USED = 0;
	struct tcp_pcb *pcb;
	pcb = fsm->datalistenpcb;
	if ( !pcb ) {
		PRINTF("> ERR: ftpd_dataclose: datalistenpcb=NULL!\n");
	} else {
		PRINTF("> closing datalistenpcb %p!\n", fsm->datalistenpcb);
		tcp_arg(pcb, NULL);
		tcp_accept(pcb, NULL);
		tcp_close(pcb);
		fsm->datalistenpcb = NULL;
	}

	pcb = fsm->datapcb;
	if ( !pcb ) {
		PRINTF("> ERR: ftpd_dataclose: datapcb=NULL!\n");
	} else {
		PRINTF("> closing datapcb %p!\n", fsm->datapcb);
		tcp_arg(pcb, NULL);
		tcp_sent(pcb, NULL);
		tcp_recv(pcb, NULL);
		tcp_close(pcb);
		fsm->datapcb = NULL;
	}
}
//-----------------------------------------------------------------------------
static err_t open_dataconnection(ftpd_conn_t *fsm)
{
	PRINTF("<open_dataconnection(%p), pasv=%u>\n", fsm, fsm->passive);
	// open a new data connection in active mode
	if (fsm->passive)
		//return ftpd_msgpoll(fsm, fsm->datapcb); // in case the data connection was extablished before the command
		return ftpd_datasent(fsm, fsm->datapcb, 0); // in case the data connection was extablished before the command

	fsm->datapcb = tcp_new();

	if (fsm->datapcb == NULL) {
		PRINTF("> ERROR: open_dataconnection: tcp_new failed!\n");
		send_msg(fsm, msg451);
		return 1;
	}
	//fsm->datafs->msgfs = fsm;
	fsm->data_fifo.buffer = data_buf;
	fsm->data_fifo.size = sizeof(data_buf);
	fsm->data_fifo.pos = 0;

	// Tell TCP that this is the structure we wish to be passed for our callbacks.
	tcp_arg(fsm->datapcb, &fsm);
	ip4_addr_t dataip;
	IP_SET_TYPE_VAL(dataip, IPADDR_TYPE_V4);
	ip4_addr_copy(*ip_2_ip4(&dataip), fsm->dataip);
	tcp_connect(fsm->datapcb, &dataip, fsm->dataport, ftpd_data_conn_accepted);

	return ERR_OK;
}

//-----------------------------------------------------------------------------
static void cmd_user(const char *arg, ftpd_conn_t *fsm)
{
	send_msg(fsm, msg331);
	fsm->state = FTPD_PASS;
}

//-----------------------------------------------------------------------------
static void cmd_pass(const char *arg, ftpd_conn_t *fsm)
{
	send_msg(fsm, msg230);
	fsm->state = FTPD_IDLE;
}

//-----------------------------------------------------------------------------
static void cmd_port(const char *arg, ftpd_conn_t *fsm)
{
	// active mode, open port
	uint8_t pHi, pLo, ip[4];
	// specifier for uint8_t is hhu, see http://www.cplusplus.com/reference/cstdio/scanf/
	int nr = sscanf(arg, "%hhu,%hhu,%hhu,%hhu,%hhu,%hhu", &(ip[0]), &(ip[1]), &(ip[2]), &(ip[3]), &pHi, &pLo);
	if (nr != 6) {
		send_msg(fsm, msg501);
	} else {
		IP4_ADDR(&fsm->dataip, ip[0], ip[1], ip[2], ip[3]);
		fsm->dataport = (u16_t)(pHi << 8) | pLo;
		send_msg(fsm, msg200);
	}
}

//-----------------------------------------------------------------------------
static void cmd_quit(const char *arg, ftpd_conn_t *fsm)
{
	send_msg(fsm, msg221);
	fsm->state = FTPD_QUIT;
}

//-----------------------------------------------------------------------------
static void cmd_cwd(const char *arg, ftpd_conn_t *fsm)
{
	// set working directory
	char *msg = (!fs_chdir(fsm->crt_dir, arg)) ? msg250 : msg550;
	send_msg(fsm, msg);
}

//-----------------------------------------------------------------------------
static void cmd_cdup(const char *arg, ftpd_conn_t *fsm)
{
	// change directory up
	char *msg = (!fs_chdir(fsm->crt_dir, "..")) ? msg250 : msg550;
	send_msg(fsm, msg);
}

//-----------------------------------------------------------------------------
static void cmd_pwd(const char *arg, ftpd_conn_t *fsm)
{
	// get working dir
	send_msg(fsm, msg257PWD, fsm->crt_dir);
}

//-----------------------------------------------------------------------------
static void cmd_list_common(const char *arg, ftpd_conn_t *fsm, uint8_t shortlist)
{
	// list directory
	if ( ERR_OK != fs_opendir(fsm->crt_dir) ) {
		send_msg(fsm, msg451);
		return;
	}

	if (open_dataconnection(fsm) != ERR_OK) {
		fs_closedir(fsm->crt_dir);
		return;
	}

	fsm->state = (shortlist) ? FTPD_NLST : FTPD_LIST;

	send_msg(fsm, msg150);
	// in case the data connection was extablished before the command
	//ftpd_msgpoll(fsm, fsm->datapcb);
	ftpd_datasent(fsm, fsm->datapcb, 0);
}

//-----------------------------------------------------------------------------
static void cmd_nlst(const char *arg, ftpd_conn_t *fsm)
{
	cmd_list_common(arg, fsm, 1);
}

//-----------------------------------------------------------------------------
static void cmd_list(const char *arg, ftpd_conn_t *fsm)
{
	cmd_list_common(arg, fsm, 0);
}

//-----------------------------------------------------------------------------
static void cmd_retr(const char *f_name, ftpd_conn_t *fsm)
{
	PRINTF("<cmd_retr(%s)>\n", f_name);
	if ( fsm->dir_entry==NULL ) {
		PRINTF("> ERR: cmd_retr: file=NULL!\n");
		return;
	}

	err_t ret = fs_open(fsm->dir_entry, f_name);//, "rb");
	if (ret) {
		PRINTF("> ERR: cmd_retr: could not open file!\n");
		send_msg(fsm, msg550);
		return;
	}
	fs_getstat(fsm->dir_entry, fsm->fs_stat);
	send_msg(fsm, msg150recv, f_name, fsm->fs_stat->size);

	fsm->state = FTPD_RETR;

	if (open_dataconnection(fsm) != ERR_OK) {
		fs_close(fsm->dir_entry);
		return;
	}
}

//-----------------------------------------------------------------------------
static void cmd_stor(const char *arg, ftpd_conn_t *fsm)
{
	err_t ret = fs_open(fsm->dir_entry, arg);//, "rb");
	if (ret) {
		PRINTF("> ERR: cmd_stor: could not open file!\n");
		send_msg(fsm, msg550);
		return;
	}

	send_msg(fsm, msg150stor, arg);

	fsm->state = FTPD_STOR;

	if (open_dataconnection(fsm) != 0) {
		fs_close(fsm->dir_entry);
		return;
	}
}

//-----------------------------------------------------------------------------
static void cmd_noop(const char *arg, ftpd_conn_t *fsm)
{
	send_msg(fsm, msg200);
}

//-----------------------------------------------------------------------------
static void cmd_syst(const char *arg, ftpd_conn_t *fsm)
{
	send_msg(fsm, msg214SYST, "UNIX");
}

//-----------------------------------------------------------------------------
static void cmd_pasv(const char *arg, ftpd_conn_t *fsm)
{
	fsm->datalistenpcb = tcp_new();

	if (fsm->datalistenpcb == NULL) {
		PRINTF("> ERR: cmd_pasv: datalistenpcb = NULL !\n");
		send_msg(fsm, msg451);
		return;
	}
	
	u16_t cnt = 0xFFFF; // loop counter
	while (1) {
		fsm->dataport ++;
		if ( (fsm->dataport<4096) || (fsm->dataport>=0x7fff) )
			fsm->dataport = 4096;

		err_t err = tcp_bind(fsm->datalistenpcb, (ip_addr_t*)&fsm->msgpcb->local_ip, fsm->dataport);
		if (err == ERR_OK)
			break;

		if ( (err != ERR_USE) || ((--cnt)==0) ) {
			ftpd_loge("> ERR: cmd_pasv: tcp_bind failed !");
			ftpd_dataclose(fsm); //	fsm->datalistenpcb = NULL;
			return;
		}
	}

	fsm->datalistenpcb = tcp_listen(fsm->datalistenpcb);
	if (!fsm->datalistenpcb) {
		ftpd_loge("> ERR: cmd_pasv: tcp_listen failed !");
		ftpd_dataclose(fsm); //	fsm->datalistenpcb = NULL;
		return;
	}

	fsm->passive = 1;
	fsm->data_connected = 0;
	//fsm->datafs->msgfs = fsm;
	//fsm->msgpcb = pcb;
	fsm->data_fifo.buffer = data_buf;
	fsm->data_fifo.size = sizeof(data_buf);
	fsm->data_fifo.pos = 0;

	// Tell TCP that this is the structure we wish to be passed for our callbacks.
	tcp_arg(fsm->datalistenpcb, fsm);
	tcp_accept(fsm->datalistenpcb, ftpd_data_conn_accepted);

	uint32_t addr = (fsm->msgpcb->local_ip).addr;
	send_msg(fsm, msg227, (addr)&0xFF, (addr>>8)&0xFF, (addr>>16)&0xFF, addr>>24, fsm->dataport>>8, (fsm->dataport)&0xff);
}

//-----------------------------------------------------------------------------
static void cmd_abrt(const char *arg, ftpd_conn_t *fsm)
{
	if (fsm->datapcb != NULL) {
		tcp_arg(fsm->datapcb, NULL);
		tcp_sent(fsm->datapcb, NULL);
		tcp_recv(fsm->datapcb, NULL);
		tcp_abort(fsm->datapcb);
		fsm->datapcb = NULL;
		FSM_FIFO_USED = 0;
		//fifo_close(&fsm->datafs->fifo);
		//free(fsm->datafs);
	}
	fsm->state = FTPD_IDLE;
}

//-----------------------------------------------------------------------------
static void cmd_type(const char *arg, ftpd_conn_t *fsm)
{
	if(strcmp(arg, "I") != 0) {
		send_msg(fsm, msg502);
		return;
	}
	
	send_msg(fsm, msg200);
}

//-----------------------------------------------------------------------------
static void cmd_mode(const char *arg, ftpd_conn_t *fsm)
{
	send_msg(fsm, msg502);
}

//-----------------------------------------------------------------------------
static void cmd_rnfr(const char *arg, ftpd_conn_t *fsm)
{
	if ( (arg == NULL) || (*arg == '\0') ) {
		PRINTF("> ERR: cmd_rnfr: arg invalid!");
		send_msg(fsm, msg501);
		return;
	}

	// check if file exists
	if ( !fs_exists(arg) ) {
		PRINTF("> ERR: cmd_rnfr: file %s does not exist!", arg);
		send_msg(fsm, msg450);
		return;
	}
	if (!(fsm->fs_stat)) {
		PRINTF("> ERR: cmd_rnfr: fs_stat = NULL!\n");
		//send_msg(pcb, fsm, msg550);
		if ( fs_getstat(fsm->dir_entry, fsm->fs_stat) != 0) {
			send_msg(fsm, msg550);
			return;
		}
	}

	strcpy(fsm->fs_stat->name, arg);

	fsm->state = FTPD_RNFR;
	send_msg(fsm, msg350);
}

//-----------------------------------------------------------------------------
static void cmd_rnto(const char *arg, ftpd_conn_t *fsm)
{
	if (fsm->state != FTPD_RNFR) {
		PRINTF("> ERR: cmd_rnto: invalid state %u\n", fsm->state);
		send_msg(fsm, msg503);
		return;
	}
	fsm->state = FTPD_IDLE;
	if ( (arg == NULL) || (*arg == '\0') ) {
		PRINTF("> ERR: cmd_rnto: arg invalid!");
		send_msg(fsm, msg501);
		return;
	}
	if (!(fsm->fs_stat)) {
		PRINTF("> ERR: cmd_rnto: fs_stat = NULL!\n");
		//send_msg(pcb, fsm, msg550);
		if ( fs_getstat(fsm->dir_entry, fsm->fs_stat) != 0) {
			send_msg(fsm, msg550);
			return;
		}
	}

	if ( fs_rename(fsm->fs_stat->name, arg)!=ERR_OK ) {
		send_msg(fsm, msg450);
	} else {
		send_msg(fsm, msg250);
	}
}

//-----------------------------------------------------------------------------
static void cmd_mkd(const char *arg, ftpd_conn_t *fsm)
{
	if ( (arg == NULL) || (*arg == '\0') ) {
		PRINTF("> ERR: cmd_mkd: arg invalid!");
		send_msg(fsm, msg501);
		return;
	}
	if ( fs_mkdir(arg)!=ERR_OK ) {
		send_msg(fsm, msg550);
	} else {
		send_msg(fsm, msg257, arg);
	}
}

//-----------------------------------------------------------------------------
static void cmd_rmd(const char *arg, ftpd_conn_t *fsm)
{
	if ( (arg == NULL) || (*arg == '\0') ) {
		PRINTF("> ERR: cmd_rmd: arg invalid!");
		send_msg(fsm, msg501);
		return;
	}

	if ( fs_getstat(fsm->dir_entry, fsm->fs_stat) != 0) {
		send_msg(fsm, msg550);
		return;
	}
	if ( !FS_FILE_IS_DIR(fsm->fs_stat) ) {
		send_msg(fsm, msg550);
		return;
	}

	if ( fs_rmdir(arg)!=ERR_OK ) {
		send_msg(fsm, msg550);
	} else {
		send_msg(fsm, msg250);
	}
}

//-----------------------------------------------------------------------------
static void cmd_dele(const char *arg, ftpd_conn_t *fsm)
{
	if ( (arg == NULL) || (*arg == '\0') ) {
		PRINTF("> ERR: cmd_dele: arg invalid!");
		send_msg(fsm, msg501);
		return;
	}
	if ( fs_remove(arg) != ERR_OK) {
		send_msg(fsm, msg550);
	} else {
		send_msg(fsm, msg250);
	}
}

//-----------------------------------------------------------------------------
static void cmd_mdtm(const char *arg, ftpd_conn_t *fsm)
{
	// return the modification time of a file 
	if ( (arg == NULL) || (*arg == '\0') ) {
		PRINTF("> ERR: cmd_mdtm: arg invalid!");
		send_msg(fsm, msg501);
		return;
	}

	if ( fs_getstat(fsm->dir_entry, fsm->fs_stat) != 0) {
		PRINTF("> ERR: cmd_mdtm: fs_getstat");
		send_msg(fsm, msg550);
		return;
	}

	tm_t s_time;
	breakTime(fsm->fs_stat->mtime, &s_time);
	// format for time is YYYYMMDDHHMMSS.sss but the last part is optional
	send_msg(fsm, "213 %04i%02i%02i%02i%02i%02i",
		s_time.year + 1900, s_time.month+1, s_time.day,
		s_time.hour, s_time.minute, s_time.second);
}

//-----------------------------------------------------------------------------
static void cmd_size(const char *arg, ftpd_conn_t *fsm)
{
	if ( (arg == NULL) || (*arg == '\0') ) {
		PRINTF("> ERR: cmd_size: arg invalid!");
		send_msg(fsm, msg501);
		return;
	}

	if ( fs_getstat(fsm->dir_entry, fsm->fs_stat) != 0) {
		PRINTF("> ERR: cmd_mdtm: fs_getstat");
		send_msg(fsm, msg550);
		return;
	}

	send_msg(fsm, "213 %u", fsm->fs_stat->size);
}

//-----------------------------------------------------------------------------
typedef struct ftpd_command {
	char *cmd;
	void (*func) (const char *arg, ftpd_conn_t * fsm);
} ftpd_command_t;

//-----------------------------------------------------------------------------
static ftpd_command_t ftpd_commands[] = {
	{"USER", cmd_user},
	{"PASS", cmd_pass},
	{"PORT", cmd_port},
	{"QUIT", cmd_quit},
	{"CWD",  cmd_cwd},
	{"CDUP", cmd_cdup},
	{"PWD",  cmd_pwd},
	{"XPWD", cmd_pwd},
	{"NLST", cmd_nlst},
	{"LIST", cmd_list},
	{"RETR", cmd_retr},
	{"STOR", cmd_stor},
	{"NOOP", cmd_noop},
	{"SYST", cmd_syst},
	{"ABOR", cmd_abrt},
	{"TYPE", cmd_type},
	{"MODE", cmd_mode},
	{"RNFR", cmd_rnfr},
	{"RNTO", cmd_rnto},
	{"MKD",  cmd_mkd},
	{"XMKD", cmd_mkd},
	{"RMD",  cmd_rmd},
	{"XRMD", cmd_rmd},
	{"DELE", cmd_dele},
	{"PASV", cmd_pasv},
	{"MDTM", cmd_mdtm},
	{"SIZE", cmd_size},
	{NULL, NULL}
};

//-----------------------------------------------------------------------------
static void ftpd_msgclose(ftpd_conn_t *fsm, struct tcp_pcb *pcb)
{
	PRINTF("<ftpd_msgclose(%p, %p)>\n", fsm, pcb);
	if ( pcb) {
		tcp_arg(pcb, NULL);
		tcp_sent(pcb, NULL);
		tcp_recv(pcb, NULL);
		tcp_close(pcb);
	}
	if ( pcb != fsm->msgpcb ) {
		pcb = fsm->msgpcb;
		PRINTF("<ftpd_msgclose(%p, %p)>\n", fsm, pcb);
		if ( pcb) {
			tcp_arg(pcb, NULL);
			tcp_sent(pcb, NULL);
			tcp_recv(pcb, NULL);
			tcp_close(pcb);
		}
	}
	fsm->msgpcb = NULL;
	if (fsm->datapcb) {
		ftpd_dataclose(fsm); // fsm->datapcb = NULL;
	}
	FSM_FIFO_USED = 0;
	fs_close(fsm->dir_entry);
}

//-----------------------------------------------------------------------------
static void ftpd_msgerr(void *arg, err_t err)
{
	PRINTF("<ftpd_msgerr(%p, %i)>\n", arg, err);
	ftpd_conn_t *fsm = arg;

	ftpd_loge("ftpd_msgerr: %s (%i)", lwip_strerr(err), err);
	if (fsm == NULL)
		return;
	ftpd_msgclose(fsm, fsm->msgpcb);
}

//-----------------------------------------------------------------------------
static err_t ftpd_msgsent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
	PRINTF("<ftpd_msgsent(%p, %p, %u)>\n", arg, pcb, len);
	ftpd_conn_t *fsm = arg;

	if ((FSM_FIFO_USED == 0) && (fsm->state == FTPD_QUIT)) {
		ftpd_msgclose(fsm, pcb);
		return ERR_OK;
	}
	
	if (pcb->state <= ESTABLISHED)
		send_msgdata(fsm);
	return ERR_OK;
}

//-----------------------------------------------------------------------------
static err_t ftpd_msgrecv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
	PRINTF("<ftpd_msgrecv(%p, %p, %p, %u)>\n", arg, pcb, p, err);

	ftpd_conn_t *fsm = arg;

	if ( (err != ERR_OK) || (p == NULL) ) {
		//PRINTF("> INFO: ftpd_msgrecv: p = %p\n", p);
	    ftpd_msgclose(fsm, pcb);
		return ERR_OK;
	}

	char *text = data_buf; //large enough, don't need: malloc(p->tot_len + 1);
	char *pt = text;

	struct pbuf *q;
	u16_t total = 0;
	for (q = p; q != NULL; q = q->next) {
		total += q->len;
		if ( total>=sizeof(data_buf) ) {
			PRINTF("> ERR: ftpd_msgrecv: q=%p, total data len=%u too large!\n", q, total);
			break;
		}
		memcpy(pt, q->payload, q->len);
		pt += q->len;
	}
	*pt = '\0';

	pt = &text[strlen(text) - 1];
	while (((*pt == '\r') || (*pt == '\n')) && pt >= text)
		*pt-- = '\0';

	ftpd_logi("> %s", text);

	char cmd[5];
	strncpy(cmd, text, 4);
	for (pt = cmd; isalpha(*pt) && pt < &cmd[4]; pt++)
		*pt = toupper(*pt);
	*pt = '\0';

	ftpd_command_t *ftpd_cmd;
	for (ftpd_cmd = ftpd_commands; ftpd_cmd->cmd != NULL; ftpd_cmd++) {
		if (!strcmp(ftpd_cmd->cmd, cmd))
			break;
	}
	// pt will point to arguments
	if (strlen(text) < (strlen(cmd) + 1))
		pt = "";
	else
		pt = &text[strlen(cmd) + 1];

	if (ftpd_cmd->func)
		ftpd_cmd->func(pt, fsm);
	else
		send_msg(fsm, msg502);

	// Inform TCP that we have taken the data
	tcp_recved(pcb, p->tot_len);
	pbuf_free(p);

	return ERR_OK;
}
//-----------------------------------------------------------------------------
static err_t ftpd_msgpoll(void *arg, struct tcp_pcb *pcb)
{
	PRINTF("<ftpd_msgpoll(%p, %p)>\n", arg, pcb);
	return ftpd_datasent(arg, pcb, 0);
}

//-----------------------------------------------------------------------------
static err_t ftpd_msgaccept(void *arg, struct tcp_pcb *pcb, err_t err)
{
	PRINTF("<ftpd_msgaccept(%p, %p, %u)>\n", arg, pcb, err);
	// Allocate memory for the structure that holds the state of the connection.
	//ftpd_msgstate_t *fsm = malloc(sizeof(struct ftpd_msgstate));
	ftpd_conn_t *fsm = &ftpd_fsm;

	//if (fsm == NULL) {
	//	ftpd_loge("ftpd_msgaccept: Out of memory");
	//	return ERR_MEM;
	//}
	memset(fsm, 0, sizeof(ftpd_conn_t));

	// Initialize FIFO
	fsm->msg_fifo.buffer = msg_buf;
	fsm->msg_fifo.size = sizeof(msg_buf);
	fsm->msg_fifo.pos = 0;
	
	fsm->crt_dir[0] = '/';
	fsm->crt_dir[1] = 0;

	fsm->state = FTPD_IDLE;

//	fsm->fs_file = fs_open();
//	if (fsm->fs_file == NULL) {
//		//fifo_close(&fsm->fifo);
//		//free(fsm);
//		return ERR_CLSD;
//	}
	fsm->dir_entry = &fs_file;
	memset(fsm->dir_entry, 0, sizeof(fs_dir_t));
	fsm->fs_stat = &f_stat;
	fsm->msgpcb = pcb;

	// Tell TCP that this is the structure we wish to be passed for our callbacks
	tcp_arg(pcb, fsm);

	// Tell TCP that we wish to be informed of incoming data by a call to the recv() function.
	tcp_recv(pcb, ftpd_msgrecv);

	/* Tell TCP that we wish be to informed of data that has been
	   successfully sent by a call to the ftpd_sent() function. */
	tcp_sent(pcb, ftpd_msgsent);

	tcp_err(pcb, ftpd_msgerr);

	tcp_poll(pcb, ftpd_msgpoll, 1);

	send_msg(fsm, msg220);

	return ERR_OK;
}
//-----------------------------------------------------------------------------
void ftpd_init(void)
{
	PRINTF("<ftpd_init>\n");
	struct tcp_pcb *pcb;

	ftpd_fs_init();

	pcb = tcp_new();
	tcp_bind(pcb, IP_ADDR_ANY, 21);
	pcb = tcp_listen(pcb);
	tcp_accept(pcb, ftpd_msgaccept);
}
