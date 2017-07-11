#include <SPI.h>
#include <MFRC522.h>
#include <Ethernet.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };//MAC address
IPAddress ip(192,168,137,20);//
EthernetServer server(80);
EthernetClient client;

/* -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno           Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          8             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      7             53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 */

#define RST_PIN   8     // Configurable, see typical pin layout above
#define SS_PIN    7   // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

/* Set your new UID here! */
//#define NEW_UID {0xDE, 0xAD, 0xBE, 0xEF}

MFRC522::MIFARE_Key key;

String id="";
const char *uid[]={"1062722543","8615180171"};
const char *pass[]={"12345","qwerty"};
char *Name[]={"ARUN","SAGAR"};
const char *allotoil[]={"5","4"};
int uidindex=2;
float remoil[2]={0,0};
int flag=0;
int flagether=0;
int flagtemp=0;
float tempL=0.0;
int k=0;

//----------------------------------------------------------------------------------------\\

byte sensorInterrupt = 0; 
byte flowPin       = 2;
int relayPin       = 4;

// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 9;

volatile byte pulseCount;  

int sensedelay=500;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

unsigned long oldTime;

//-------------------------------------------------------------------------------------\\

void setup()
{ 
  SPI.begin();         // Init SPI bus
  mfrc522.PCD_Init();         // Init MFRC522 card
  Serial.begin(9600);
  pinMode(flowPin, INPUT);
  pinMode(relayPin, OUTPUT);
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);

  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

  for(int a=0;a<uidindex;a++)
  {
    String st;
    st=allotoil[a];
    remoil[a]=st.toFloat(); 
  }
}

//-----------------------------------------------------------------------------------------\\

void loop()
{
   ether();
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
           client.println("<TITLE>Arduino Project</TITLE>");
           client.println("</HEAD>");
           
           client.println("<BODY>");  
                  
           client.println("<br/>");  
           client.println("<H2>SMART RATION STORE</H2>");
           client.println("<br />");
           rfid();
           
           if(flag==1)
           {
             client.println("<b>USER ID :</b>");
             client.println(uid[k]);
             client.println("<br/>");
             client.println("<b>NAME :</b>");
             client.println(Name[k]);
             client.println("<br/>");
             client.println("<br/>");
             client.println("<b>EDIBLE OIL</b>");
             client.println("<br/>");
             client.println("ALLOTED:");
             client.println(allotoil[k]);
             client.println("Ltr. &nbsp &nbsp Remaining:");
             client.println(remoil[k]);
             client.println("Ltr.");
             client.println("<br/>");
            
             client.println("<br/>");
             client.println("<FORM ACTION='/' method=get >"); //uses IP/port of web page
             client.println("<p><b>PASSWORD :</b> <INPUT TYPE=PASSWORD NAME='pass' VALUE='' SIZE='25' MAXLENGTH='6'></p>");
             client.println("Enter the Quantity Riquired");
             client.println("<br/>");
             client.println("<p><b>EDIBLE OIL :</b> <INPUT TYPE=TEXT NAME='ML' VALUE='' SIZE='25' MAXLENGTH='6'>");
             client.println("Ltr.</p>");
             client.println("<INPUT TYPE=SUBMIT NAME='submit' VALUE='SUBMIT'>");
             client.println("<INPUT TYPE=SUBMIT NAME='cancel' VALUE='CANCEL'>");
             client.println("</FORM>");  
             client.println("<br />");
             flagtemp=1;
           }
           
           client.println("</BODY>");
           if(flag==0)
           {
             if(flagtemp==1)
                client.println("On Processing. Please wait!");
             if(flagtemp==2)
                client.println("Wrong password!");   
             if(flagtemp==3) 
                client.println("Exceeded limit!");   
             client.println("<head>");
             client.println("<meta http-equiv='refresh' content='1; URL=http://192.168.137.20'>");
             client.println("</head>");
           }
           client.println("</HTML>");
            
           client.stop();
           Serial.println(readString);
           if(readString.indexOf("&cancel")>0)
           { 
             flag=0;
             flagtemp=0;
             return;
           }
           //stopping client
           //controls the Arduino if you press the buttons 
           if (readString.length() >0)
           { 
             int pos1 = readString.indexOf("pass=");
             int pos2 = readString.indexOf("&ML=");
             if(pos1>0 && pos2>pos1)
             {
               String datapass = readString.substring(pos1+5, pos2);          
               if(datapass!=pass[k])
               {
                 datapass="";
                 flagtemp=2;
                 flag=0;
                 k=0;
                 return;
               }
               k=0;
             }
              int pos3 = readString.indexOf("&submit");
              if(pos2>0 && pos3>pos2)
              {
                String data = readString.substring(pos2+4, pos3); 
                tempL=data.toFloat();//convert to integer
                flagtemp=1;
              }
           } 
           if(tempL>0)
            {
              readString=""; //clears variable for new input
              if(tempL>remoil[k])
              {
                flagtemp=3;
                flag=0;
                tempL=0;
              }
              else
              {
                liquid();
                flagtemp=0;
              }  
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
       client.println("Insert Your RFID");
       client.println("<br/>");
       flag=0;
       return;
    }
    else
    {  
      for (byte i = 0; i < mfrc522.uid.size; i++) 
      {
        id=id+mfrc522.uid.uidByte[i];
       
      } 
      Serial.println(id);
      for(k=0;k<uidindex;k++)
      {
         if(id==uid[k])
         {            
            id="";
            flag=1;
            break;
         }
         else
         {
         continue;
         }     
         id="";
      }      
    }
}

void liquid()
{
  float value = tempL * 1000;
  tempL=0;
  Serial.print("Required ML: ");
  Serial.println(value);
  digitalWrite(relayPin,LOW);
   {
     pulseCount        = 0;
     flowRate          = 0.0;
     flowMilliLitres   = 0;
     totalMilliLitres  = 0;
     oldTime           = 0;
     if(value>0)
     {
      remoil[k]=remoil[k]-value/1000;
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
      remoil[k]=remoil[k]-totalMilliLitres/1000;
      Serial.println("Done\n");
     }
     value=0;
     digitalWrite(relayPin,LOW);
   }
}

void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}
 
