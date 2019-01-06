/**
  ******************************************************************************
  * @file    netconf.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    31-July-2013
  * @brief   Network connection configuration
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/tcp.h"

#include "lwip/init.h"
#include "lwip/netif.h"
#include "netif/ethernet.h"
#include "lwip\timeouts.h"

#include "lwip/udp.h"
#include "lwip/etharp.h"
#include "ethernetif.h"
#include "main.h"
#include "netconf.h"
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
struct netif gnetif;
uint32_t TCPTimer;
uint32_t ARPTimer;

#ifdef USE_DHCP
#include "lwip/dhcp.h"
uint32_t DHCPfineTimer;
uint32_t DHCPcoarseTimer;
uint8_t DHCP_state;
#endif

/* Private functions ---------------------------------------------------------*/
void LwIP_DHCP_Process_Handle(void);
/**
* @brief  Initializes the lwIP stack
* @param  None
* @retval None
*/
void LwIP_Init(void)
{
  struct ip4_addr ipaddr;
  struct ip4_addr netmask;
  struct ip4_addr gw;

  lwip_init(); // new in v2
  
#ifdef USE_DHCP
  ipaddr.addr = 0;
  netmask.addr = 0;
  gw.addr = 0;
#else
  IP4_ADDR(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
  IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1 , NETMASK_ADDR2, NETMASK_ADDR3);
  IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
#endif  

  /* - netif_add(struct netif *netif, struct ip_addr *ipaddr,
  struct ip_addr *netmask, struct ip_addr *gw,
  void *state, err_t (* init)(struct netif *netif),
  err_t (* input)(struct pbuf *p, struct netif *netif))

  Adds your network interface to the netif_list. Allocate a struct
  netif and pass a pointer to this structure as the first argument.
  Give pointers to cleared ip_addr structures when using DHCP,
  or fill them with sane numbers otherwise. The state pointer may be NULL.

  The init function pointer must point to a initialization function for
  your ethernet netif interface. The following code illustrates it's use.*/
  netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);

  /*  Registers the default network interface.*/
  netif_set_default(&gnetif);

  extern uint32_t EthStatus;
  if (EthStatus == (ETH_INIT_FLAG | ETH_LINK_FLAG))
  { 
    /* Set Ethernet link flag */
    gnetif.flags |= NETIF_FLAG_LINK_UP;

    /* When the netif is fully configured this function must be called.*/
    netif_set_up(&gnetif);
#ifdef USE_DHCP
    DHCP_state = DHCP_START;
	PRINTF("> DHCP set to start\n");
//	dhcp_start(&gnetif);
#endif /* USE_DHCP */
  }
  else
  {
    /*  When the netif link is down this function must be called.*/
    netif_set_down(&gnetif);
#ifdef USE_DHCP
    DHCP_state = DHCP_LINK_DOWN;
	PRINTF("> DHCP link down!\n");
#endif /* USE_DHCP */
  }
  /* Set the link callback function, this function is called on change of link status*/
  netif_set_link_callback(&gnetif, ETH_link_callback);
}

/**
* @brief  Called when a frame is received
* @param  None
* @retval None
*/
void LwIP_Pkt_Handle(void)
{
  /* Read a received packet from the Ethernet buffers and send it to the lwIP for handling */
  ethernetif_input(&gnetif);
}

/**
* @brief  LwIP periodic tasks
* @param  localtime the current LocalTime value
* @retval None
*/
void LwIP_Periodic_Handle(__IO uint32_t localtime)
{
    /* Cyclic lwIP timers check */
    sys_check_timeouts(); // v2

#ifdef USE_DHCP
  /* Fine DHCP periodic process every 500ms */
  if ( (localtime - DHCPfineTimer) >= DHCP_FINE_TIMER_MSECS)
  {
    DHCPfineTimer = localtime;
     if ((DHCP_state != DHCP_ADDRESS_ASSIGNED) && 
        (DHCP_state != DHCP_TIMEOUT) &&
          (DHCP_state != DHCP_LINK_DOWN))
    { 
      /* toggle LED1 to indicate DHCP on-going process */
	  ToggleLED(); // STM_EVAL_LEDToggle(LED1);

      /* process DHCP state machine */
      LwIP_DHCP_Process_Handle();    
    }
  }
#else
	(void)(localtime);
#endif
}

#ifdef USE_DHCP
uint32_t IPaddress;
/**
* @brief  LwIP_DHCP_Process_Handle
* @param  None
* @retval None
*/
void LwIP_DHCP_Process_Handle()
{
  struct ip4_addr ipaddr;
  struct ip4_addr netmask;
  struct ip4_addr gw;

  //PRINTF("<LwIP_DHCP_Process_Handle(state=%u)>\n", DHCP_state);
  switch (DHCP_state)
  {
  case DHCP_START:
    {
      DHCP_state = DHCP_WAIT_ADDRESS;
	  PRINTF(". DHCP_WAIT_ADDRESS .\n");
      dhcp_start(&gnetif);
      /* IP address should be set to 0 
         every time we want to assign a new DHCP address */
      IPaddress = 0;
    }
    break;

  case DHCP_WAIT_ADDRESS:
    {
      /* Read the new IP address */
      IPaddress = gnetif.ip_addr.addr;

      if (IPaddress!=0) 
      {
        DHCP_state = DHCP_ADDRESS_ASSIGNED;	

	    PRINTF(". DHCP_ADDRESS_ASSIGNED .\n");
        /* Stop DHCP */
        //dhcp_stop(&gnetif); // do not stop, this will clear the IP address

		LEDOn(1); // STM_EVAL_LEDOn(LED1);
      }
      else
      {
		struct dhcp * dhcp;
		dhcp = netif_get_client_data(&gnetif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);
        /* DHCP timeout */
        if (dhcp->tries > MAX_DHCP_TRIES)
        {
          DHCP_state = DHCP_TIMEOUT;

	      PRINTF(". DHCP_TIMEOUT .\n");
          /* Stop DHCP */
          dhcp_stop(&gnetif);

          /* Static address used */
          IP4_ADDR(&ipaddr, IP_ADDR0 ,IP_ADDR1 , IP_ADDR2 , IP_ADDR3 );
          IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
          IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
          netif_set_addr(&gnetif, &ipaddr , &netmask, &gw);

		  LEDOff(1); // STM_EVAL_LEDOn(LED1);
        }
      }
    }
    break;
  default: break;
  }
}

uint32_t LwIP_DHCP_GetAddress(void)
{
  return IPaddress;
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
