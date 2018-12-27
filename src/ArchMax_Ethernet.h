#ifndef _ARCH_MAX_ETHERNET_H
#define _ARCH_MAX_ETHERNET_H


#include <libmaple/ethernet.h>
#include "main.h"



extern "C" {
	
void ArchMax_Ethernet_Init();
void ArchMax_Ethernet_Loop();

}


#endif // _ARCH_MAX_ETHERNET_H
