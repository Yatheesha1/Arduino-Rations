#include <SPI.h>
#include <MFRC522.h>

/* -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno           Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          8             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      7            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 */

#define RST_PIN   8     // Configurable, see typical pin layout above
#define SS_PIN    7    // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

MFRC522::MIFARE_Key key;

String id="";
char *uid[]={"8615180171","32242193128"};
int uidindex=2;
int flaguid=0;


//----------------------------------------------------------------------------------------\\

byte sensorInterrupt = 0; 
byte flowPin       = 2;
int relayPin       = 4;
int sensedelay=500;


// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 14;

volatile byte pulseCount;  

float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

unsigned long oldTime;

void setup()
{
  SPI.begin();         // Init SPI bus
  mfrc522.PCD_Init();         // Init MFRC522 card
  Serial.begin(9600); 
  pinMode(flowPin, INPUT);
  pinMode(relayPin, OUTPUT);
  //digitalWrite(sensorPin, HIGH);
  attachInterrupt(sensorInterrupt, pulseCounter, RISING);
  digitalWrite(flowPin, LOW);
}

/**
 * Main program loop
 */
void loop()
{
   rfid();
   liquid();
}   

void rfid() 
{
  while(1)
  {
    if(flaguid==0)
    {
      Serial.println("Insert Your RFID");
      flaguid=1;
    }  
    id="";
    // Look for new cards, and select one if present
    if ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() ) 
    {
       delay(50);
       //return;
    }
    else
    {  
      flaguid=0;
      Serial.print(F("Card UID:"));
      for (byte i = 0; i < mfrc522.uid.size; i++) 
      {
        id=id+mfrc522.uid.uidByte[i];
       
      } 
      Serial.println("\t"+id);
      for(int k=0;k<uidindex;k++)
      {
         if(id==uid[k])
         {
            Serial.println("Success");
            return;
         }
         else
          continue;
         Serial.println("Not valid User! Please insert valid uid!");
         flaguid=1;
      }      
      delay(2000);
      Serial.println();
    }
  }
}

void liquid()
{
  digitalWrite(relayPin,LOW);
  Serial.println("Enter input in ML");
   //Serial.println("Stopped");
  while(!Serial.available());
   {
     pulseCount        = 0;
     flowRate          = 0.0;
     flowMilliLitres   = 0;
     totalMilliLitres  = 0;
     oldTime           = 0;
     int value=0;
     value=Serial.parseInt();
     Serial.println(value);
     if(value>0)
     {
      Serial.println("Started");
      digitalWrite(relayPin,HIGH);  
      while(value>totalMilliLitres)
      {       
       if((millis() - oldTime) > sensedelay)    // Only process counters once per second
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
     digitalWrite(relayPin,LOW);
     Serial.println("Enter input in ML");
   }
}


/*
Insterrupt Service Routine
 */
void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}
