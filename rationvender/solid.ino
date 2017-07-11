void solid()
{ 
  Serial.println("Enter quantity of Rice in grams or 0 for exit:");
  while(!Serial.available());//wait for value
  gm=0.0;
  int val=0;
  val=Serial.parseInt();
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
    if(Serial.available())
      {
        if(Serial.read()=='x')
        {
        gm=0.0;
        val=0;
        Serial.println("Done");    
        myservo.write(0);
        delay(1000);
        Serial.print("Last time "); 
        gm=(califactor*scale.getGram());
        Serial.print(gm,1);  
        Serial.println("gm Vendered "); 
        Serial.println();
        return;
       }
      }
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
