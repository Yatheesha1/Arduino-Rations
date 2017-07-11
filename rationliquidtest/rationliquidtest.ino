byte sensorInterrupt = 0; 
byte flowPin       = 2;
int relayPin       = 4;


// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 8;
int processdelay=500;

volatile byte pulseCount;  

float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

unsigned long oldTime;

void setup()
{
  Serial.begin(9600); 
  pinMode(flowPin, INPUT);
  pinMode(relayPin, OUTPUT);
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  digitalWrite(relayPin,LOW);
  Serial.println("Enter input in ML");
}

/**
 * Main program loop
 */
void loop()
{
   liquid();
}   

void liquid()
{

   //Serial.println("Stopped");
  if(Serial.available())
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
       if((millis() - oldTime) > processdelay)    // Only process counters once per second
       { 
          detachInterrupt(sensorInterrupt);
          flowRate = ((1000.0 /processdelay) * pulseCount) / calibrationFactor;
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
      Serial.println("Enter input in ML");
     }
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
