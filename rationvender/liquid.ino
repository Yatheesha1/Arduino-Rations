void liquid()
{
   Serial.println("Enter quantity of Kerosene in ML or 0 for exit:");
   while(!Serial.available());
   int value=0;
   value=Serial.parseInt();
   if(value==0)
   {
        Serial.println("Done");
        Serial.println();
        return;
   }
   digitalWrite(relayPin,LOW);
   //Serial.println("Stopped");
   //if(Serial.available())
   {
     pulseCount        = 0;
     flowRate          = 0.0;
     flowMilliLitres   = 0;
     totalMilliLitres  = 0;
     oldTime           = 0;
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
          attachInterrupt(sensorInterrupt, pulseCounter, FALLING);                                          
       }
     }
     digitalWrite(relayPin,LOW);
     Serial.println("Done");
     delay(1000);
     Serial.print("Last time "); 
     Serial.print(totalMilliLitres);  
     Serial.println("ML Vendered "); 
     Serial.println();
     Serial.println();
   }
}  
