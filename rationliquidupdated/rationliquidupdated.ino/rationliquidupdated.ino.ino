#include <FlowMeter.h>                  // https://github.com/sekdiy/FlowMeter

// connect a flow meter to an interrupt pin (see notes on your Arduino model for pin numbers)
FlowMeter Meter = FlowMeter(2);
int flowpin=2;
int relaypin=4;
float liter=0.0;

// set the measurement update period to 1s (1000 ms)
const unsigned long period = 1000;

// define an 'interrupt service handler' (ISR) for every interrupt pin you use
void MeterISR() 
{
  // let our flow meter count the pulses
  Meter.count();
}

void setup() 
{
  // prepare serial communication
  Serial.begin(9600);
  pinMode(relaypin,OUTPUT);
  // enable a call to the 'interrupt service handler' (ISR) on every rising edge at the interrupt pin
  // do this setup step for every ISR you have defined, depending on how many interrupts you use
  attachInterrupt(INT0, MeterISR, RISING);

  // sometimes initializing the gear generates some pulses that we should ignore
  Meter.reset();
  digitalWrite(relaypin,LOW);
  Serial.println("Enter required Liter:" );
}

void loop() 
{
  if(Serial.available())
  { 
    liter=0.0;
    liter=Serial.parseFloat();
    Serial.println("You requested " + String(liter,3) + " of Kerosene");
    if(liter>0)
       digitalWrite(relaypin,HIGH);
    while((Meter.getTotalVolume())<liter)
    {
  // process the (possibly) counted ticks
  Meter.tick(period);

  // output some measurement result
  Serial.println("Flow Rate: " + String(Meter.getCurrentFlowrate()) + " L/min\tTotal quantity: " + String(Meter.getTotalVolume(),3)+ " L");
  }
    digitalWrite(relaypin,LOW); 
    Serial.println("Done" );  
    Serial.println("Enter required Liter:" ); 
    attachInterrupt(INT0, MeterISR, RISING);
    Meter.reset();
  }
}
