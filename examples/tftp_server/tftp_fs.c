/*
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Dirk Ziegelmeier <dziegel@gmx.de>
 *
 */

#include "lwip/apps/tftp_server.h"
#include "lwip/apps/fs.h"
#include "tftp_fs.h"

//-----------------------------------------------------------------------------
// struct fs_file *fs_open(const char *name);
// void fs_close(struct fs_file *file);
// int fs_read(struct fs_file *file, char *buffer, int count);
// int fs_bytes_left(struct fs_file *file);
//-----------------------------------------------------------------------------
struct fs_file file;
/**
 * Open file for read/write.
 * @param fname Filename
 * @param mode Mode string from TFTP RFC 1350 (netascii, octet, mail)
 * @param write Flag indicating read (0) or write (!= 0) access
 * @returns File handle supplied to other functions
 */
static void* tftp_open(const char* fname, const char* mode, u8_t is_write)
{
  err_t ret = fs_open(&file, fname); //, (is_write) ? "wb":"rb");
  return (ret==ERR_OK) ? &file : NULL;
}

/**
 * Close file handle
 * @param handle File handle returned by open()
 */
static void tftp_close(void* handle)
{
  fs_close((struct fs_file *)handle);
}

/**
 * Read from file 
 * @param handle File handle returned by open()
 * @param buf Target buffer to copy read data to
 * @param bytes Number of bytes to copy to buf
 * @returns &gt;= 0: Success; &lt; 0: Error
 */
static int tftp_read(void* handle, void* buf, int bytes)
{
  return fs_read((struct fs_file *)handle, (char *)buf, bytes);
}

/**
 * Write to file
 * @param handle File handle returned by open()
 * @param pbuf PBUF adjusted such that payload pointer points to the beginning of write data.
 *             In other words,TFTP headers are stripped off.
 * @returns &gt;= 0: Success; &lt; 0: Error
 */
static int tftp_write(void* handle, struct pbuf* p)
{
#if 0 // not implemented yet
  while (p != NULL) {
    if (fs_write(p->payload, 1, p->len, (FILE*)handle) != (size_t)p->len) {
      return -1;
    }
    p = p->next;
  }
#else // not implemented yet
  (void)(handle);
  (void)(p);
#endif // not implemented yet
  return 0;
}
//-----------------------------------------------------------------------------
static const struct tftp_context tftp_ctx = {
  tftp_open,
  tftp_close,
  tftp_read,
  tftp_write
};

//-----------------------------------------------------------------------------
void tftp_fs_init(void)
{
  tftp_init(&tftp_ctx);
}
