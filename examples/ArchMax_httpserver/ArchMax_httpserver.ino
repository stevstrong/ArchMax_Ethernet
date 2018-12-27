//-----------------------------------------------------------------------------
// TCP Server for Arch Max v1.1
//-----------------------------------------------------------------------------

#include <ArchMax_Ethernet.h>
#include "httpd.h"

//-----------------------------------------------------------------------------
const uint8_t my_mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

// Enter an IP address for your controller below.
// The IP address will be dependent on your local network:
//IPAddress ip(192, 168, 1, 177);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
//EthernetServer server(80);

#define LED1 LED_BUILTIN
#define LED2 PC6
#define LED3 PC7
//-----------------------------------------------------------------------------
void setup()
{
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  for (int i=1; i<4; i++) { LEDOff(i); }
  for (int i=1; i<4; i++) { LEDOn(i); delay(200); }
  for (int i=1; i<4; i++) { LEDOff(i); delay(200); }

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("*************************************************");
  Serial.println("Ethernet server with D83848 PHY for Arch Max v1.1");
  Serial.println("*************************************************");
  // start the Ethernet connection and the server:
  Serial.println("Initializing Ethernet");

  ArchMax_Ethernet_Init();

  Serial.print("Initializing the HTTP server ... ");
  httpd_init();
  Serial.println("done.");
}

//-----------------------------------------------------------------------------
void loop()
{
  ArchMax_Ethernet_Loop();
}


extern "C" {
//-----------------------------------------------------------------------------
uint16_t ADC_GetConversionValue(uint8_t adc_in_nr)
{
	return analogRead(PA0+adc_in_nr);
}
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

