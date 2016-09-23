// pins 1 to 6 on the MM3 connect via a 6 pin riser easily with one interface on pins 8 to 2 (excluding pin 4)
// pin 1 mm3 to 8 Arduino
// 2 to 7
// 3 to 6
// 4 to 5
// 5 to 3
// 6 to 2
// pin 7 on the MM3 is GND on arduino
// pin 12 on the MM3 is Vcc (5V)

#define SCLK 8 //the clock to pulse
#define MISO 7 //master in, slave out
#define MOSI 6 //master out, slave in 
#define SSNOT 5 //when low, the device is enabled
// NO 4, Pin 4 is used by the Ethernet Board for the CD Card, It Needs to be Skipped.
#define DRDY 3 //this is low after a reset, high when data is ready
#define RESET 2 //this needs to be toggled low-high-low before each measurement

int wait = 2; //2ms wait time when toggling clock etc.
int samples = 90; //the number of samples to take before calculating an average. (~2 a second).

void send_bit(int _high){

  //this sends the bit on the rising edge of the clock
  digitalWrite(MOSI, _high); //make the data ready
  delay(wait);
  digitalWrite(SCLK, HIGH); //clock goes high, data is read.
  delay(wait);
  digitalWrite(SCLK, LOW); //clock goes low.
  delay(wait);
}

int receive_bit(){

  //this receives the data on the falling edge of the clock
  digitalWrite(SCLK, HIGH); //start the clock cycle
  delay(wait);
  int bit = digitalRead(MISO); //read the data
  delay(wait);
  digitalWrite(SCLK, LOW); //stop the clock cycle
  delay(wait);
  return bit;
}

float readaxis(int _axis){
  //this function sends eight bits, waits until the data is ready
  //then receives 16 bits

  long runningtotal = 0; //we are going to put the final value in here.
  
  //pulse the reset, this must be done each time to tell the MM3 to take a reading.
  digitalWrite(RESET, LOW);
  delay(5);
  digitalWrite(RESET, HIGH);
  delay(5);
  digitalWrite(RESET, LOW);
  delay(5);

  //Pulsing the reset will make the Data Ready Go Low.
  if (digitalRead(DRDY) == LOW) {
       //send the command byte
       //this sends data that we are not in debug mode
       //and sets the amount of time to read the magnetic sensors (the ASIC period)
       // /4096 (011100XX) (Has a 60mS wait time for /4096) XX is the axis 01,10 or 11.
       send_bit(LOW);
       send_bit(HIGH);
       send_bit(HIGH);
       send_bit(HIGH);
       send_bit(LOW);
       send_bit(LOW);
     
       //the last two bits select the axis
       if (_axis == 0) { //x axis
         send_bit(LOW);
         send_bit(HIGH);
         if (Debug) Serial.print("X");
       }
       else if (_axis == 1) { //y axis
         send_bit(HIGH);
         send_bit(LOW);
         if (Debug) Serial.print("Y");
       }
       else { //z axis 
         send_bit(HIGH);
         send_bit(HIGH);
         if (Debug) Serial.print("Z");
       }

       //Now wait until the drdy line is high
       // we are only going to wait up to 70ms tops. it should only take 60ms for the finest mode.
       for (int w = 0; w < 70/wait; w++) {
         if (digitalRead(DRDY) == HIGH) {
           //receive the results and tally them up as they come in 
           if (Debug) Serial.print(":"); //indicates about to read.
           
           //the leftmost bit signs the number as positive or negative
           long sign = receive_bit();
           
           //the remaining bits need to be translated from individual bits into an integer
           for (int i = 14; i >= 0; i = i - 1){
             long thisbit = receive_bit();
             thisbit = thisbit << i;
             runningtotal = runningtotal | thisbit;
           }
           
           if (sign == 1) {
             runningtotal = runningtotal - 32768; 
           }
           break; //break from the loop, we got the sample.
         }
         delay(wait); // Data Ready not HIGH, Wait.
       } //end loop
       
       //if for some reason the result was not a number or it was -1 we set it to 0, which is not a valid return value.
       if (runningtotal == -1 || isnan(runningtotal)) runningtotal = 0;
       
     } else  if (Debug) Serial.print("E");//end data ready never went low after reset. Error.
     
     return runningtotal;
}
