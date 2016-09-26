#include <SoftwareSerial.h>

/**
 * Setup function.
 */
void setup() {
  // Set up serial link to the PC.
  Serial.begin(57600);
  
  // Set up serial link to the SDVN board.
  Serial1.begin(57600);
}

/**
 * Arduino Main loop.
 */
void loop() {
  char c;
  
  // If there is incoming data from the board, send it to the PC.
  if(Serial1.available()){
    c = Serial1.read();
    Serial.write(c);  
  }

  // If there is incoming data from the PC, send it to the board.
  if(Serial.available()){
    c = Serial.read();
    Serial1.write(c);
  }
  /*
  if(Serial.available()){
    c = Serial.read();
    Serial.write(c);
  }*/
}
