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
         Serial.println("Not valid User! Please insert valid uid!");
         flaguid=1;
      }      
      delay(2000);
      Serial.println();
    }
  }
}
