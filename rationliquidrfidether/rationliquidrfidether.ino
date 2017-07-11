#include <SPI.h>
#include <MFRC522.h>
#include<String.h>
#include <Ethernet.h>

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
char *uid[]={"1062722543"};
int uidindex=1;
int flaguid=0;
int flag=0;
int flagether=0;
int flagtemp=0;

//----------------------------------------------------------------------------------------\\

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };//MAC address

IPAddress ip(192,168,137,20);//
EthernetServer server(80);
EthernetClient client;

//----------------------------------------------------------------------------------------\\

byte sensorInterrupt = 0; 
byte flowPin       = 2;
int relayPin       = 4;
float value=0.0;

// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 3;
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
  Ethernet.begin(mac, ip);
  server.begin();
  pinMode(flowPin, INPUT);
  pinMode(relayPin, OUTPUT);
  attachInterrupt(sensorInterrupt, pulseCounter, RISING);
  //digitalWrite(flowPin, LOW);
}

void loop()
{
   //rfid();
   ether();
   //liquid();
}   


void ether()
{
  String readString;
  client = server.available();
  if (client) 
  {
    while (client.connected()) 
    {   
      if (client.available()) 
      {
         char c = client.read(); 
         //read char by char HTTP request
         if (readString.length() < 100) 
         {
          //store characters to string
          readString += c;
         }


         //if HTTP request has ended
         if (c == '\n') 
         {              
           client.println("HTTP/1.1 200 OK"); //send new page
           client.println("Content-Type: text/html");
           
           client.println();  
           client.println("<HTML>");
           
           client.println("<HEAD>");
           client.println("<meta name='apple-mobile-web-app-status-bar-style' content='black-translucent' />");
           client.println("<link rel='stylesheet' type='text/css' href='http://randomnerdtutorials.com/ethernetcss.css' />");
           client.println("<TITLE>Arduino Project</TITLE>");
           client.println("</HEAD>");
           
           client.println("<BODY>");  
                  
           client.println("<br/>");  
           client.println("<H2>RATION VENDER</H2>");
           rfid();
           if(flag==1)
           {
           client.println("<FORM ACTION='/' method=get >"); //uses IP/port of web page
           client.println("Enter Quantity in Liter   : <INPUT TYPE=TEXT NAME='ML' VALUE='' SIZE='25' MAXLENGTH='13'>");
           client.println("<br />");
           client.println("<br />");
           client.println("<INPUT TYPE=SUBMIT NAME='submit' VALUE='SUBMIT'>");
           client.println("</FORM>");  
           client.println("<br />");
           flagtemp=1;
           }
           
           client.println("</BODY>");
           if(flag==0)
           {
            if(flagtemp==1)
               client.println("On Processing. Please wait!");
           client.println("<head>");
           client.println("<meta http-equiv='refresh' content='1; URL=http://192.168.137.20'>");
           client.println("</head>");
           }
           client.println("</HTML>");
            
           client.stop();  
           //stopping client
           //controls the Arduino if you press the buttons
           if (readString.length() >0)
           {
            Serial.println(readString); //prints string to serial port out
            int pos1 = readString.indexOf("ML=");
            int pos2 = readString.indexOf("&submit");
            if(pos1>0)
            {
            String data = readString.substring(pos1+3, pos2); 
            float temp=data.toFloat();//convert to integer
            value=temp*1000;//////////////////////////////////////////////////////to milli liter 
            flagtemp=0;
            }
           } 
            if(value>0)
            {
              Serial.print("Required ML: ");
              Serial.println(value);
              readString=""; //clears variable for new input
              liquid();
              flag=0;
            }      
           readString="";            
        }
      }
    }
  }
}
void rfid() 
{ 
    // Look for new cards, and select one if present
    if ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() ) 
    {
       if(flaguid==0)
       {
       client.println("Insert Your RFID");
       client.println("<br/>");
       }
       flag=0;
    }
    else
    {  
      client.println("Card UID:");
      for (byte i = 0; i < mfrc522.uid.size; i++) 
      {
        id=id+mfrc522.uid.uidByte[i];
       
      } 
      client.println(id);
      Serial.println(id);
      client.println("<br/>");
      for(int k=0;k<uidindex;k++)
      {
         if(id==uid[k])
         {
            client.println("Success");
            client.println("<br/>");
            flag=1;
         }
         else
         {
         client.println("Not valid User! Please insert valid uid!");
         client.println("<br/>");
         flaguid=1;
         }     
         id="";
      }      
    }
}


void liquid()
{
  digitalWrite(relayPin,LOW);
   {
     pulseCount        = 0;
     flowRate          = 0.0;
     flowMilliLitres   = 0;
     totalMilliLitres  = 0;
     oldTime           = 0;
     if(value>0)
     {
      Serial.println("Started");
      digitalWrite(relayPin,HIGH);  
      while(value>totalMilliLitres)
      {       
       if((millis() - oldTime) > 1000)    // Only process counters once per second
       { 
          detachInterrupt(sensorInterrupt);
          flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
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
     value=0;
     digitalWrite(relayPin,LOW);
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
