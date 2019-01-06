//#include "eth_cfg.h"

#include "main.h"
#include <libmaple/flash.h>
#include <usb_serial.h>

extern "C" {

#ifdef ARCH_DEBUG
char dbg_str[_MAX_SS] __attribute__ ((aligned));
void dbg_print(char* s) { Serial.print(s); }

#if 0
#include <IPAddress.h>

//-----------------------------------------------------------------------------
void DBG_PrintPbuf(struct pbuf *p)
{
/** Main packet buffer struct */
//struct pbuf {
	PRINTF("<- pbuf -\n");
  /** next pbuf in singly linked pbuf chain */
  //struct pbuf *next;
	PRINTF("next: %08X\n", (uint32_t)p->next);
  /** pointer to the actual data in the buffer */
  //void *payload;
	PRINTF("payload: %08X\n", (uint32_t)p->payload);
  /**
   * total length of this buffer and all next buffers in chain belonging to the same packet.
   * For non-queue packet chains this is the invariant:
   * p->tot_len == p->len + (p->next? p->next->tot_len: 0)
   */
  //u16_t tot_len;
	PRINTF("tot_len: %u\n", p->tot_len);

  /** length of this buffer */
  //u16_t len;
	PRINTF("len: %u\n", p->len);

  /** pbuf_type as u8_t instead of enum to save space */
  //u8_t /*pbuf_type*/ type;
	PRINTF("type: %u\n", p->type);

  /** misc flags */
  //u8_t flags;
	PRINTF("flags: %02X\n", p->flags);
  /**
   * the reference count always equals the number of pointers that refer to this pbuf.
   * This can be pointers from an application, the stack itself, or pbuf->next pointers from a chain.
   */
  //u16_t ref;
	PRINTF("ref: %04X\n", p->ref);
//};
	PRINTF("- pbuf ->\n");
}

//-----------------------------------------------------------------------------
void DBG_PrintMACRegs(void)
{
	PRINTF("<- MAC regs -\n");
	PRINTF("MACCR: %08X\n",     ETH->MACCR);
	PRINTF("MACFFR: %08X\n",    ETH->MACFFR);
	PRINTF("MACHTHR: %08X\n",   ETH->MACHTHR);
	PRINTF("MACHTLR: %08X\n",   ETH->MACHTLR);
//	PRINTF("MACMIIAR: %08X\n",  ETH->MACMIIAR);
//	PRINTF("MACMIIDR: %08X\n",  ETH->MACMIIDR);
	PRINTF("MACFCR: %08X\n",    ETH->MACFCR);
	PRINTF("MACVLANTR: %08X\n", ETH->MACVLANTR);
//	PRINTF("MACRWUFFR: %08X\n", ETH->MACRWUFFR);
//	PRINTF("MACPMTCSR: %08X\n", ETH->MACPMTCSR);
	PRINTF("MACDBGR: %08X\n",   ETH->MACDBGR);
	PRINTF("MACSR: %08X\n",     ETH->MACSR);
	PRINTF("MACIMR: %08X\n",    ETH->MACIMR);
	PRINTF("MACA0HR: %08X\n",   ETH->MACA0HR);
	PRINTF("MACA0LR: %08X\n",   ETH->MACA0LR);
	PRINTF("- MAC regs ->\n");
}

//-----------------------------------------------------------------------------
void DBG_PrintDMARegs(void)
{
	PRINTF("<- DMA regs -\n");
	PRINTF("DMABMR: %08X\n",    ETH->DMABMR);
//	PRINTF("DMATPDR: %08X\n",   ETH->DMATPDR);
//	PRINTF("DMARPDR: %08X\n",   ETH->DMARPDR);
	PRINTF("DMARDLAR: %08X\n",  ETH->DMARDLAR);
	PRINTF("DMATDLAR: %08X\n",  ETH->DMATDLAR);
	PRINTF("DMASR: %08X\n",     ETH->DMASR);
	PRINTF("DMAOMR: %08X\n",    ETH->DMAOMR);
	PRINTF("DMAIER: %08X\n",    ETH->DMAIER);
	PRINTF("DMAMFBOCR: %08X\n", ETH->DMAMFBOCR);
	PRINTF("DMARSWTR: %08X\n",  ETH->DMARSWTR);
	PRINTF("DMACHTDR: %08X\n",  ETH->DMACHTDR);
	PRINTF("DMACHRDR: %08X\n",  ETH->DMACHRDR);
	PRINTF("DMACHTBAR: %08X\n", ETH->DMACHTBAR);
	PRINTF("DMACHRBAR: %08X\n", ETH->DMACHRBAR);
	PRINTF("- DMA regs ->\n");
}
#endif
#if 0
//-----------------------------------------------------------------------------
void DBG_PrintDMADesc(void * _desc)
{
	ETH_DMARxDescTypeDef * desc = (ETH_DMARxDescTypeDef *)_desc;
	//typedef struct  
	//{
		PRINTF("<- dma_tx_desc -\n");
	//  __IO uint32_t   Status;           /*!< Status */
		PRINTF("  Status: %08X\n", desc->Status);
	//  uint32_t   ControlBufferSize;     /*!< Control and Buffer1, Buffer2 lengths */
		PRINTF("  ControlBufferSize: %lu\n", desc->ControlBufferSize);
	//  uint32_t   Buffer1Addr;           /*!< Buffer1 address pointer */
		PRINTF("  Buffer1Addr: %08X\n", (uint32_t)desc->Buffer1Addr);
	//  uint32_t   Buffer2NextDescAddr;   /*!< Buffer2 or next descriptor address pointer */
		PRINTF("  NextDescAddr: %08X\n", (uint32_t)desc->NextDescAddr);
	//  /*!< Enhanced ETHERNET DMA PTP Descriptors */
	//  uint32_t   ExtendedStatus;        /*!< Extended status for PTP receive descriptor */
	//  uint32_t   Reserved1;             /*!< Reserved */
	//  uint32_t   TimeStampLow;          /*!< Time Stamp Low value for transmit and receive */
	//  uint32_t   TimeStampHigh;         /*!< Time Stamp High value for transmit and receive */
	//} ETH_DMADescTypeDef;
		PRINTF("- dma_tx_desc ->\n");
}


extern ETH_DMATxDescTypeDef  DMATxDscrTab[ETH_RXBUFNR];
//-----------------------------------------------------------------------------
void DBG_PrintHandle(ETH_HandleTypeDef * handle, uint8_t rxtx)
{
	PRINTF("<- eth_handle -\n");
//  uint32_t                   LinkStatus;    /*!< Ethernet link status        */
//	PRINTF("LinkStatus: %08X\n", handle->LinkStatus);
//  ETH_DMADescTypeDef         *TxDesc;       /*!< Tx descriptor to Set        */
	if (rxtx&1) // Tx descriptors
	{
		for (uint8_t desc_nr = 0; desc_nr<ETH_TXBUFNR; desc_nr++)
		{
			ETH_DMATxDescTypeDef * desc = &DMATxDscrTab[desc_nr];
			PRINTF("TxDesc_%u: %08X\n", desc_nr, (uint32_t)desc);
			DBG_PrintDMADesc(desc);
		}
	}
	PRINTF("TxDesc: %08X\n", (uint32_t)handle->TxDesc);
	DBG_PrintDMADesc(handle->TxDesc);
//  ETH_DMADescTypeDef         *RxDesc;       /*!< Rx descriptor to Get        */
	PRINTF("RxDesc: %08X\n", (uint32_t)handle->RxDesc);
	DBG_PrintDMADesc(handle->RxDesc);
//  ETH_DMARxFrameInfos        RxFrameInfos;  /*!< last Rx frame infos         */
//  ETH_DMADescTypeDef         *FSRxDesc;     /*!< First Segment Rx Desc */
	PRINTF("FSRxDesc: %08X\n", (uint32_t)handle->FSRxDesc);
	DBG_PrintDMADesc(handle->FSRxDesc);
//  ETH_DMADescTypeDef         *LSRxDesc;     /*!< Last Segment Rx Desc */
	PRINTF("LSRxDesc: %08X\n", (uint32_t)handle->LSRxDesc);
	DBG_PrintDMADesc(handle->LSRxDesc);
//  uint32_t                   rxSegCount; /*!< Segment count */
	PRINTF("rxSegCount: %u\n", handle->rxSegCount);
//  uint32_t                   rxLength;   /*!< Frame length */
	PRINTF("rxLength: %u\n", handle->rxLength);
//  uint32_t                   rxBuffer;   /*!< Frame buffer */
	PRINTF("rxBuffer: %08X\n", (uint32_t)handle->rxBuffer);
//  __IO HAL_ETH_StateTypeDef  State;         /*!< ETH communication state     */
	PRINTF("State: %02X\n", handle->State);
//  HAL_LockTypeDef            Lock;          /*!< ETH Lock                    */
//	PRINTF("Lock: %u\n", handle->Lock);
	PRINTF("- eth_handle ->\n");
}
#endif
#if 0
//-----------------------------------------------------------------------------
void DBG_PrintNetif(struct netif* netIf)
{
	PRINTF("\n<--- netif ---\n");
/** Generic data structure used for all lwIP network interfaces.
 *  The following fields should be filled in by the initialization
 *  function for the device driver: hwaddr_len, hwaddr[], mtu, flags */
//struct netif {
  /** pointer to next in linked list */
  PRINTF("next: %08X\n", (uint32_t)netIf->next);

#if LWIP_IPV4
  /** IP address configuration in network byte order */
  PRINTF("ip_addr: %s\n", ip4addr_ntoa((const ip4_addr_t*)&netIf->ip_addr)); //  ip_addr_t ip_addr;
  PRINTF("netmask: %s\n", ip4addr_ntoa((const ip4_addr_t*)&netIf->netmask)); //  ip_addr_t netmask;
  PRINTF("gateway: %s\n", ip4addr_ntoa((const ip4_addr_t*)&netIf->gw)); //  ip_addr_t gw;
#endif /* LWIP_IPV4 */
#if LWIP_IPV6
  /** Array of IPv6 addresses for this netif. */
  ip_addr_t ip6_addr[LWIP_IPV6_NUM_ADDRESSES];
  /** The state of each IPv6 address (Tentative, Preferred, etc).
   * @see ip6_addr.h */
  u8_t ip6_addr_state[LWIP_IPV6_NUM_ADDRESSES];
#endif /* LWIP_IPV6 */
  /** This function is called by the network device driver
   *  to pass a packet up the TCP/IP stack. */
  PRINTF("input: %08X\n", (uint32_t)netIf->input); //  netif_input_fn input;
#if LWIP_IPV4
  /** This function is called by the IP module when it wants
   *  to send a packet on the interface. This function typically
   *  first resolves the hardware address, then sends the packet.
   *  For ethernet physical layer, this is usually etharp_output() */
  PRINTF("output: %08X\n", (uint32_t)netIf->output); //  netif_output_fn output;
#endif /* LWIP_IPV4 */
  /** This function is called by ethernet_output() when it wants
   *  to send a packet on the interface. This function outputs
   *  the pbuf as-is on the link medium. */
  PRINTF("linkoutput: %08X\n", (uint32_t)netIf->linkoutput); //  netif_linkoutput_fn linkoutput;
#if LWIP_IPV6
  /** This function is called by the IPv6 module when it wants
   *  to send a packet on the interface. This function typically
   *  first resolves the hardware address, then sends the packet.
   *  For ethernet physical layer, this is usually ethip6_output() */
  netif_output_ip6_fn output_ip6;
#endif /* LWIP_IPV6 */
#if LWIP_NETIF_STATUS_CALLBACK
  /** This function is called when the netif state is set to up or down
   */
  PRINTF("status_callback: %08X\n", (uint32_t)netIf->status_callback); //  netif_status_callback_fn status_callback;
#endif /* LWIP_NETIF_STATUS_CALLBACK */
#if LWIP_NETIF_LINK_CALLBACK
  /** This function is called when the netif link is set to up or down
   */
  PRINTF("link_callback: %08X\n", (uint32_t)netIf->link_callback); //  netif_status_callback_fn link_callback;
#endif /* LWIP_NETIF_LINK_CALLBACK */
#if LWIP_NETIF_REMOVE_CALLBACK
  /** This function is called when the netif has been removed */
  PRINTF("remove_callback: %08X\n", (uint32_t)netIf->remove_callback); //  netif_status_callback_fn remove_callback;
#endif /* LWIP_NETIF_REMOVE_CALLBACK */
  /** This field can be set by the device driver and could point
   *  to state information for the device. */
  PRINTF("state: %d\n", *(uint8_t*)(netIf->state)); //  void *state;
#ifdef netif_get_client_data
  PRINTF("client_data: %08X\n", (uint32_t)netIf->client_data); //  void* client_data[LWIP_NETIF_CLIENT_DATA_INDEX_MAX + LWIP_NUM_NETIF_CLIENT_DATA];
#endif
#if LWIP_IPV6_AUTOCONFIG
  /** is this netif enabled for IPv6 autoconfiguration */
  u8_t ip6_autoconfig_enabled;
#endif /* LWIP_IPV6_AUTOCONFIG */
#if LWIP_IPV6_SEND_ROUTER_SOLICIT
  /** Number of Router Solicitation messages that remain to be sent. */
  PRINTF("rs_count: %d\n", netIf->rs_count); //  u8_t rs_count;
#endif /* LWIP_IPV6_SEND_ROUTER_SOLICIT */
#if LWIP_NETIF_HOSTNAME
  /* the hostname for this netif, NULL is a valid value */
  PRINTF("hostname: %s\n", netIf->hostname); //  const char*  hostname;
#endif /* LWIP_NETIF_HOSTNAME */
#if LWIP_CHECKSUM_CTRL_PER_NETIF
  PRINTF("chksum_flags: %04X\n", netIf->chksum_flags); //  u16_t chksum_flags;
#endif /* LWIP_CHECKSUM_CTRL_PER_NETIF*/
  /** maximum transfer unit (in bytes) */
  PRINTF("mtu: %d\n", netIf->mtu); //  u16_t mtu;
  /** number of bytes used in hwaddr */
  PRINTF("hwaddr_len: %d\n", netIf->hwaddr_len); //  u8_t hwaddr_len;
  /** link level hardware address of this interface */
  uint8_t *a = netIf->hwaddr; //  u8_t hwaddr[NETIF_MAX_HWADDR_LEN];
	PRINTF("hwddress: %02X:%02X:%02X:%02X:%02X:%02X\n", a[0], a[1], a[2], a[3], a[4], a[5]);
  /** flags (@see @ref netif_flags) */
  PRINTF("flags: %02X\n", netIf->flags); //  u8_t flags;
  /** descriptive abbreviation */
  PRINTF("name: %c%c\n", netIf->name[0], netIf->name[1]); //  char name[2];
  /** number of this interface */
  PRINTF("num: %d\n", netIf->num); //  u8_t num;
#if MIB2_STATS
  /** link type (from "snmp_ifType" enum from snmp_mib2.h) */
  PRINTF("link_type: %d\n", netIf->link_type); //  u8_t link_type;
  /** (estimate) link speed */
  PRINTF("link_speed: %lu\n", netIf->link_speed); //  u32_t link_speed;
  /** timestamp at last change made (up/down) */
  PRINTF("ts: %lu\n", netIf->ts); //  u32_t ts;
  /** counters */
  PRINTF("mib2_counters: %08X\n", (uint32_t)netIf->mib2_counters); //  struct stats_mib2_netif_ctrs mib2_counters;
#endif /* MIB2_STATS */
#if LWIP_IPV4 && LWIP_IGMP
  /** This function could be called to add or delete an entry in the multicast
      filter table of the ethernet MAC.*/
  PRINTF("igmp_mac_filter: %08X\n", (uint32_t)netIf->igmp_mac_filter); //  netif_igmp_mac_filter_fn igmp_mac_filter;
#endif /* LWIP_IPV4 && LWIP_IGMP */
#if LWIP_IPV6 && LWIP_IPV6_MLD
  /** This function could be called to add or delete an entry in the IPv6 multicast
      filter table of the ethernet MAC. */
  PRINTF("mld_mac_filter: %08X\n", (uint32_t)netIf->mld_mac_filter); //  netif_mld_mac_filter_fn mld_mac_filter;
#endif /* LWIP_IPV6 && LWIP_IPV6_MLD */
#if LWIP_NETIF_HWADDRHINT
  PRINTF("addr_hint: %08X\n", (uint32_t)netIf->addr_hint); //  u8_t *addr_hint;
#endif /* LWIP_NETIF_HWADDRHINT */
#if ENABLE_LOOPBACK
  /* List of packets to be queued for ourselves. */
  PRINTF("loop_first: %08X\n", (uint32_t)netIf->loop_first); //  struct pbuf *loop_first;
  PRINTF("loop_last: %08X\n", (uint32_t)netIf->loop_last); //  struct pbuf *loop_last;
#if LWIP_LOOPBACK_MAX_PBUFS
  PRINTF("ts: %d\n", netIf->loop_cnt_current); //  u16_t loop_cnt_current;
#endif /* LWIP_LOOPBACK_MAX_PBUFS */
#endif /* ENABLE_LOOPBACK */
//};
	PRINTF("--- netif end --->\n");
}
#endif


#endif // ARCH_DEBUG

} // extern "C"