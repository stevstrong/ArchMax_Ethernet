//-----------------------------------------------------------------------------
// TCP Server for Arch Max v1.1
//-----------------------------------------------------------------------------

#include <ArchMax_Ethernet.h>
#include "tcp_echoserver.h"

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
//-----------------------------------------------------------------------------
void setup()
{
  pinMode(LED1, OUTPUT);
  ToggleLED();
  delay(100);
  ToggleLED();
  delay(100);
  ToggleLED();
  delay(100);
  ToggleLED();

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("*************************************************");
  Serial.println("TCP echo server with D83848 PHY for Arch Max v1.1");
  Serial.println("*************************************************");
  // start the Ethernet connection and the server:
  Serial.println("Initializing Ethernet");

  ArchMax_Ethernet_Init();

  Serial.print("Initializing tcp_echoserver...");
  tcp_echoserver_init();
  Serial.println("done.");

}

//-----------------------------------------------------------------------------
void loop()
{
  ArchMax_Ethernet_Loop();
}


extern "C" {
//-----------------------------------------------------------------------------
void LEDOn(void)
{
	digitalWrite(LED1, LOW);
}
//-----------------------------------------------------------------------------
void LEDOff(void)
{
	digitalWrite(LED1, HIGH);
}
//-----------------------------------------------------------------------------
void ToggleLED(void)
{
	digitalWrite(LED1, !digitalRead(LED1));
}
}

