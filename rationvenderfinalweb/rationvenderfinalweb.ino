#include <SPI.h>
#include <MFRC522.h>
#include <Ethernet.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
byte ip[] = { 192, 168, 1, 6 };// ip ethernet
byte serverName[] = { 192, 168, 1, 4 };//ip system
EthernetClient client;

/* -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno           Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          7             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      8            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 */

#define RST_PIN   7     // Configurable, see typical pin layout above
#define SS_PIN    8    // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

/* Set your new UID here! */
//#define NEW_UID {0xDE, 0xAD, 0xBE, 0xEF}

MFRC522::MIFARE_Key key;

String id="";
char *uid[]={"20291177171"};
int uidindex=1;
int flaguid=0;


//----------------------------------------------------------------------------------------\\

#include <Servo.h>

Servo myservo;  // create servo object to control a servo
int val;    // variable to read the value from the analog pin
int servopin=3;

//-----------------------------------------------------------------------------------------\\

// Hx711.DOUT - pin #A3
// Hx711.SCK - pin #A2

#include <Hx711.h>
Hx711 scale(A3, A2);
float gm=0.0;
int califactor=-5;

//-----------------------------------------------------------------------------------------\\

byte sensorInterrupt = 0; 
byte flowPin       = 2;
int relayPin       = 4;

// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 12;

volatile byte pulseCount;  

int sensedelay=500;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

unsigned long oldTime;
float temp;

//-------------------------------------------------------------------------------------\\

void setup()
{ 
  while (!Serial);     // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  myservo.attach(servopin);  // attaches the servo on pin 9 to the servo object
  myservo.write(0);
  SPI.begin();         // Init SPI bus
  mfrc522.PCD_Init();         // Init MFRC522 card
  pinMode(flowPin, INPUT);
  pinMode(relayPin, OUTPUT);
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  Serial.begin(9600);

  Ethernet.begin(mac, ip);
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  //digitalWrite(flowPin, LOW);
  //digitalWrite(relayPin, LOW);
}

//-----------------------------------------------------------------------------------------\\

void loop()
{
   if (client.connect(serverName, 80))
   {
     Serial.println("connected...");
     Serial.println("ARDUINO: forming HTTP request message");
     client.print("GET /rfidration_kerosene/add.php?temp=");
     if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial() ) 
     {
       for (byte i = 0; i < mfrc522.uid.size; i++) 
       {
        client.print(mfrc522.uid.uidByte[i], HEX);
        Serial.println(mfrc522.uid.uidByte[i], HEX);
       }
     }  
     client.println();
     client.println(" HTTP/1.1");
     client.println("Host: localhost");
     client.println();
     Serial.println("ARDUINO: HTTP message sent");

     String c = client.readString();
     if (c != "")
     {
       String st=c.substring(1,c.length()-2);
       if(c.charAt(0)=='L')
       {
         temp = st.toFloat();
         Serial.println(temp);
         delay(1000);
         client.stop();
         liquid();
         temp = 0;
       }
       if(c.charAt(0)=='S')
       {
         temp = st.toFloat();
         Serial.println(temp);
         delay(1000);
         client.stop();
         solid();
         temp = 0;
       }
     }
     else
     {
        client.stop();
     }
   }
   else
   {
      Serial.println("connection failure");
   }
}

void solid()
{ 
  gm=0.0;
  float val=0.0;
  val=temp*1000;
  if(val==0)
  {
        Serial.println("Done");
        Serial.println();
        return;
  }  
  myservo.write(180);                  // sets the servo position according to the scaled value

  while((gm+50)<val)
  {
    gm=(califactor*scale.getGram());
    Serial.print(gm,1);
    Serial.println(" gm");
  }
  myservo.write(0);
  Serial.println("Done");  
  delay(1000);
  Serial.print("Last time "); 
  gm=(califactor*scale.getGram());
  Serial.print(gm,1);  
  Serial.println("gm Vendered "); 
  Serial.println();   
}  

void liquid()
{
  float value = temp * 1000;
  digitalWrite(relayPin, LOW);
  {
    pulseCount        = 0;
    flowRate          = 0.0;
    flowMilliLitres   = 0;
    totalMilliLitres  = 0;
    oldTime           = 0;
    if (value > 0)
    {
      Serial.println("Started");
      digitalWrite(relayPin, HIGH);
      while (value > totalMilliLitres)
      {
        if ((millis() - oldTime) > sensedelay)   // Only process counters once per second
        {
          detachInterrupt(sensorInterrupt);
          flowRate = ((1000.0 / sensedelay) * pulseCount) / calibrationFactor;
          oldTime = millis();
          flowMilliLitres = (flowRate / 60) * 1000;
          totalMilliLitres += flowMilliLitres;
          Serial.print("Quantity: ");
          Serial.print(totalMilliLitres);
          Serial.println(" mL");
          pulseCount = 0;
          attachInterrupt(sensorInterrupt, pulseCounter, RISING);
        }
      }
      Serial.println("Done\n");
    }
    value = 0;
    digitalWrite(relayPin, LOW);
  }
}

void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}

