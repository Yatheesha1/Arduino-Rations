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
 * RST/Reset   RST          7             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      8             53        D10        10               10
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
char *uid[]={"7339177171","24021612481"};
char *pass[]={"12345","qwerty"};
char *Name[]={"ARUN","SAGAR"};
char *allotrice[]={"10","8"};
int uidindex=2;
float remrice[2]={0,0};
int flag=0;
int flagether=0;
int flagtemp=0;
float tempR=0.0;
int k=0;

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
int califactor=5;

//-------------------------------------------------------------------------------------\\

void setup()
{ 
  myservo.attach(servopin);  // attaches the servo on pin 9 to the servo object
  myservo.write(0);
  SPI.begin();         // Init SPI bus
  mfrc522.PCD_Init();         // Init MFRC522 card
  Serial.begin(9600);

  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

  for(int a=0;a<uidindex;a++)
  {
    String st;
    st=allotrice[a];
    remrice[a]=st.toFloat();
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
         k=0;
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
             client.println("ALLOTED RICE:");
             client.println(allotrice[k]);
             client.println("Kg. &nbsp &nbsp Remaining:");
             client.println(remrice[k]);
             client.println("Kg.");
             client.println("<br/>");
            
             client.println("<br/>");
             client.println("<FORM ACTION='/' method=get >"); //uses IP/port of web page
             client.println("<p><b>PASSWORD :</b> <INPUT TYPE=PASSWORD NAME='pass' VALUE='' SIZE='25' MAXLENGTH='6'></p>");
             client.println("Enter the Quantity Riquired");
             client.println("<br/>");
             client.println("<p><b>RICE :</b> <INPUT TYPE=TEXT NAME='kg' VALUE='' SIZE='25' MAXLENGTH='6'>");
             client.println("Kg.</p>");
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
             int pos2 = readString.indexOf("&kg=");
             if(pos1>0 && pos2>pos1)
             {
               String datapass = readString.substring(pos1+5, pos2);               
               if(datapass!=pass[k])
               {
                 datapass="";
                 return;
               }
             }
              int pos3 = readString.indexOf("&submit");
              if(pos2>0 && pos3>pos2)
              {
                String data = readString.substring(pos2+4, pos3); 
                tempR=data.toFloat();//convert to integer
                flagtemp=1;
              }
           } 
           if(tempR>0)
            {
              readString=""; //clears variable for new input
              solid();
              flagtemp=0;
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

void solid()
{ 
  gm=0.0;
  float val=0.0;
  val=tempR*1000;
  Serial.print("Required Rice: ");
  Serial.println(tempR);
  Serial.print(" KG");
  tempR=0;
  if(val==0)
  {
        Serial.println("Done");
        Serial.println();
        return;
  }  
  myservo.write(180);                  // sets the servo position according to the scaled value
  remrice[k]=remrice[k]-val/1000;
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
