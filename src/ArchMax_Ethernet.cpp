
#include "ArchMax_Ethernet.h"
#include <wirish_time.h>

//-----------------------------------------------------------------------------
uint32_t LocalTime; /* this variable is used to create a time reference incremented by 10ms */


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

