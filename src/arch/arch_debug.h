/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
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
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef __ARCH_DEBUG_H__
#define __ARCH_DEBUG_H__

#include <wirish_time.h>
#include <stdio.h>


#define ARCH_DEBUG 1

/* Uncomment SERIAL_DEBUG to enables retarget of printf to  serial port (COM1 on STM32 evalboard) 
   for debug purpose */   
//#define SERIAL_DEBUG

/* Uncomment the line below to expanse the "assert_param" macro in the 
   Standard Peripheral Library drivers code */
/* #define USE_FULL_ASSERT    1 */

//-----------------------------------------------------------------------------
#ifdef ARCH_DEBUG

//#warning DEBUG version
  #define _MAX_SS 512
  extern char dbg_str[]; // declared in debug.cpp
  extern void dbg_print(char*); // declared in debug.cpp

  #define PRINTF(...) { sprintf(dbg_str, "[%06d] ", millis()); sprintf(&dbg_str[9], __VA_ARGS__); dbg_print(dbg_str); }

#if 0
//#include "utility/stm32f4xx_hal_eth.h"
  extern void DBG_PrintDMADesc(void * desc);
  extern void DBG_PrintHandle(ETH_HandleTypeDef * handle, uint8_t rxtx);
  //extern void DBG_PrintHandle(ETH_HandleTypeDef * handle, uint32_t size, uint8_t rxtx);
  extern void DBG_PrintNetif(struct netif*);
  extern void DBG_PrintMACRegs(void);
  extern void DBG_PrintDMARegs(void);
  extern void DBG_PrintPbuf(struct pbuf *p);
#endif

#else //  ARCH_DEBUG

//#warning RELEASE version
  #define PRINTF(...)
  #define DBG_PrintDMADesc()
  #define DBG_PrintHandle()
  #define DBP_PrintfNetif()

#endif // ARCH_DEBUG

/* Exported macro ------------------------------------------------------------*/
#ifdef  USE_FULL_ASSERT

/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr: If expr is false, it calls assert_failed function
  *   which reports the name of the source file and the source
  *   line number of the call that failed. 
  *   If expr is true, it returns no value.
  * @retval None
  */
  #define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))
/* Exported functions ------------------------------------------------------- */
  void assert_failed(uint8_t* file, uint32_t line);
#else
  #define assert_param(expr) ((void)0)
#endif /* USE_FULL_ASSERT */





#endif /* __ARCH_DEBUG_H__ */
