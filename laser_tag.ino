#define DEBUG

#include <IRremote.h> //http://www.arcfn.com/2009/08/multi-protocol-infrared-remote-library.html

#define LED_YEL 18   // Analog 4
#define LED_GRN 19   // Analog 5
#define LED_RED 7
#define LED_BLU 8
#define LED_IR 3

#define SENSOR_IR 2
#define SENSOR_TRIGGER 6

#define WIFI_TX 4
#define WIFI_RX 5

#define COIN_VIB 9

// SPI bus values -- arduino already has 11-13 reserved/defined, so just documentation here
#define SOUND_SELECT 10 //SLAVE_SELECT
//#define MISO 11
//#define MOSI 12
//#define SCK 13

#define LCD_TX A0
#define LCD_RX A1

/**
 * Locally-defined functions:
 * void send_shot()
 * void send_sysmsg()
 * void setTeamLEDs()
 * void setLEDsOff()
 */

IRrecv irrecv(SENSOR_IR);
IRsend irsend;

decode_results results;

void setup() {
  // Specify pin modes
  pinMode(LED_YEL, OUTPUT);
  pinMode(LED_GRN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLU, OUTPUT);
  pinMode(SENSOR_TRIGGER, INPUT);
// set rest of pin modes here

  // Turn outputs off
  setLEDsOff();
  digitalWrite(COIN_VIB, LOW);
  // Enable pull-up resistors
  digitalWrite(SENSOR_TRIGGER, HIGH);
  
  // Initialize libraries
  irrecv.enableIRIn(); // Start the receiver
  
  Serial.begin(9600); 
  Serial.println("Serial started");
  
  mt_setup();
  mt_setTeamID(3);    // TO be changed to whatever server sends: valid teams=1-4
  mt_setPlayerID(10); // Ditto.
  
  // set LED team colors
  setTeamLEDs();
}


unsigned long time = micros();

void loop() {
  if(digitalRead(SENSOR_TRIGGER) == LOW)
  {
      send_shot();
  }

  if (irrecv.decode(&results))
  {
    mt_parseIRMessage(results.value);
    irrecv.resume(); // Receive the next value
  }
}

void send_shot()
{
  irsend.sendMT(mt_buildShot(), 16);
  delay(50);
  irrecv.enableIRIn(); // Re-enable IR receiving after sending
}

void send_sysmsg()
{
  irsend.sendMT(0x0900,16);
  delay(50);
  irrecv.enableIRIn();
}

/**
 * Turns on team id LEDs and turns off all LEDs but the team id LED.
 */
void setTeamLEDs() {
  setLEDsOff();

  switch( mt_getTeamID() ) {
    case 1:  digitalWrite(LED_RED, HIGH);
        break;
    case 2:  digitalWrite(LED_BLU, HIGH);
        break;
    case 3:  digitalWrite(LED_YEL, HIGH);
        break;
    case 4:  digitalWrite(LED_GRN, HIGH);
        break;
  } 
}

void setLEDsOff() {
  digitalWrite(LED_YEL, LOW);
  digitalWrite(LED_GRN, LOW);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_BLU, LOW);
}
