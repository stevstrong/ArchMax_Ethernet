
#include "ArchMax_Ethernet.h"
#include <wirish_time.h>
#include "lwip/stats.h"

//-----------------------------------------------------------------------------
uint32_t LocalTime; // time reference incremented by 1 ms

//-----------------------------------------------------------------------------
void ArchMax_Ethernet_Init()
{
	PRINTF("<ArchMax_Ethernet_Init.");
  // Configure GPIOs and clocks
  ethernet_gpio_init();
  ethernet_init();
  
  // Configure MAC and DMA
  ETH_BSP_Config();

  // Initialize the LwIP stack
  LwIP_Init();

	PRINTF(".ArchMax_Ethernet_Init>\n");
}

//-----------------------------------------------------------------------------
void ArchMax_Ethernet_Loop()
{
	ETH_Tx_DbgPrint();
	ETH_Rx_DbgPrint();
    /* check if any packet received */
    if (ETH_CheckFrameReceived())
    { 
      /* process received ethernet packet */
      LwIP_Pkt_Handle();
    }
    /* handle periodic timers for LwIP */
	if ( LocalTime == millis() ) return;
	LocalTime = millis();
    LwIP_Periodic_Handle(LocalTime);
}

//-----------------------------------------------------------------------------
//  @brief  Inserts a delay time.
//  @param  nCount: number of 10ms periods to wait for.
//  @retval None
//-----------------------------------------------------------------------------
void Delay(uint32_t nCount)
{
  /* wait until the desired delay finish */
  while( (millis()-LocalTime)<nCount ) ;
}
