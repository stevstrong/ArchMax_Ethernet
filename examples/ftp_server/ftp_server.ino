//-----------------------------------------------------------------------------
// TFTP Server for Arch Max v1.1
//-----------------------------------------------------------------------------

#include <ArchMax_Ethernet.h>
#include "ftpd.h"
#include <SdFat.h>

//-----------------------------------------------------------------------------
//const uint8_t my_mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

// Enter an IP address for your controller below.
// The IP address will be dependent on your local network:
//IPAddress ip(192, 168, 1, 177);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
//EthernetServer server(80);

SdFat sd;

//-----------------------------------------------------------------------------
#define LED1 LED_BUILTIN
#define LED2 PC6
#define LED3 PC7

//-----------------------------------------------------------------------------
void setup()
{
  pinMode(LED1, OUTPUT);
  digitalWrite(LED1, HIGH); // turn off
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  for (int i=1; i<4; i++) { LEDOff(i); }
  for (int i=1; i<4; i++) { LEDOn(i); delay(200); }
  for (int i=1; i<4; i++) { LEDOff(i); delay(200); }

  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) ; // wait for serial port to connect. Needed for native USB port only

  Serial.println("********************************************************");
  Serial.println("LwIP 2.x Ethernet server for D83848 PHY on Arch Max v1.1");
  Serial.println("********************************************************");
  // start the Ethernet connection and the server:
  Serial.println("Initializing Ethernet");

  ArchMax_Ethernet_Init();

}

uint32_t gotIP;
//-----------------------------------------------------------------------------
void loop()
{
  ArchMax_Ethernet_Loop();

  uint32_t IPaddress = LwIP_DHCP_GetAddress();
	if ( (IPaddress) && (!gotIP) )
	{
		gotIP = 1;
		PRINTF("My Address: %hhu.%hhu.%hhu.%hhu\n", IPaddress&0xFF, (IPaddress>>8)&0xFF, (IPaddress>>16)&0xFF, (IPaddress>>24));
		Serial.print("Initializing the tftp server ... ");
		ftpd_init();
		Serial.println("done.");

	}
	if ( (gotIP) && (!IPaddress) )
		Serial.print("ERROR: IPaddress cleared!"), delay(500);
	
}


extern "C" {
//-----------------------------------------------------------------------------
void LEDOn(uint8_t nr)
{
	if (nr==1) digitalWrite(LED1, LOW);
	else if (nr==2) digitalWrite(LED2, LOW);
	else if (nr==3) digitalWrite(LED3, LOW);
}
//-----------------------------------------------------------------------------
void LEDOff(uint8_t nr)
{
	if (nr==1) digitalWrite(LED1, HIGH);
	else if (nr==2) digitalWrite(LED2, HIGH);
	else if (nr==3) digitalWrite(LED3, HIGH);
}
//-----------------------------------------------------------------------------
void ToggleLED(void)
{
	digitalWrite(LED1, !digitalRead(LED1));
}

}

