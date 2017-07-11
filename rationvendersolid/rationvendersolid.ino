//----------------------------------------------------------------------------------------\\

#include <Servo.h>

Servo myservo;  // create servo object to control a servo
int val;    // variable to read the value from the analog pin
int servopin=4;

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
  Serial.begin(9600);
}

//-----------------------------------------------------------------------------------------\\

void loop()
{
   //rfid();     
    solid(); 
}   

void solid()
{ 
  Serial.println("Enter quantity of Rice in gm or 0 for exit:");
  while(!Serial.available());//wait for value
  gm=0;
  int val=0;
  val=Serial.parseInt();
  if(val==0)
  {
        Serial.println("Done");
        Serial.println();
        return;
  }  
  myservo.write(180);                  // sets the servo position according to the scaled value

  while(gm<val)
  {
    gm=(califactor*scale.getGram());
    Serial.print(gm,1);
    Serial.println(" g");
    delay(200);
    if(Serial.available())
      {
        if(Serial.read()=='x')
        {
        gm=0;
        val=0;
        Serial.println("Done");    
        Serial.println();
        myservo.write(0);
        return;
       }
      }
  }
  
  Serial.println("Done");    
  Serial.println();
  myservo.write(0);   
}  
