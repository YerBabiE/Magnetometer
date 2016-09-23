//1.0 release, Brendan Davey (www.brendandaveyphotography.com

bool Debug = true; //enable if you want to check output via serial interface etc. Connected to PC.

//If submitting to a webpage, Enable Ethernat, supply website URL and POST location in the code below.
bool Eth = false; 

#include <Ethernet.h>
#include <SPI.h>
#include "Mag.h"
#include <math.h>

//reserved address MAC is { 0x0C, 0x4D, 0xE9, 0xB3, 0xD0, 0x04 }
byte mac[] = { 0x0C, 0x4D, 0xE9, 0xB3, 0xD0, 0x04 }; // RESERVED MAC ADDRESS

EthernetClient client;

//to store our data (running total)
float x, y, z;
//new (current) values
float xn, yn, zn;
//old (previous) values
float xo, yo, zo;
//counters to work out averages.
int xc, yc, zc;

float tot = 0;
String da;
unsigned long timer = 0;


void setup() {

  if (Debug) { Serial.begin(115200); } //used if connected to a PC for serial comms. I it is assumed that if Debug is on, it's connect to PC.
  
  if (Eth) { 
    //If Ethernet is enabled, print out the current settings.
    Ethernet.begin(mac);
    if (Debug) {
      Serial.print("IP = ");
      Serial.println(Ethernet.localIP());
      Serial.print("DNS = ");
      Serial.println(Ethernet.dnsServerIP());
      Serial.print("gatewayIP = ");
      Serial.println(Ethernet.gatewayIP());
      Serial.print("subnetMask = ");
      Serial.println(Ethernet.subnetMask());
    }
  }
  
  //Settings for the MM3 see Mag.h more more details.
  pinMode(RESET, OUTPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(MISO, INPUT);
  pinMode(DRDY, INPUT);
  pinMode(SCLK, OUTPUT);
  pinMode(SSNOT, OUTPUT);
  digitalWrite(SSNOT, LOW); //put the device into slave mode, it no work without this. (LOW IS ON, 'NOT' SS).

}

void loop(){

  timer = millis(); //get the current time since boot. We will use this at the end of the loop to ensure we wait 1 min until next run from now.
 
  if (Debug) Serial.println(timer); //a check to ensure we are waiting 1 min between checks.

  //reset all variables for this run.
  x=0; y=0; z=0; //the running totals
  xo=0; yo=0; zo=0; //the old value
  xn=0; yn=0; zn=0; //the new value
  xc=0; yc=0; zc=0; //the count for each
  tot=0;
  
  //take some samples, we are going to use the average.
  for (int l = 0; l < samples; l++) {
    
    //read an axis, check it's ok, and within 5% of previous, then update the total and count etc.
    xn = readaxis(0); //new value
    if (xn != 0) { //if it's not 0, it's 'ok'.
      if (Debug) { Serial.print(xn); Serial.print(", "); } //if debug is on, we output via serial for checking etc.
      if ( abs((xn-xo)/xn * 100) <= 5) { x = x + xn; xc++; } //if it's within 5% of previous value add it to total and inc counter.
    }
    
    yn = readaxis(1);
    if (yn != 0) {
      if (Debug) { Serial.print(yn); Serial.print(", "); }
      if ( abs((yn-yo)/yn * 100) <= 5) { y = y + yn; yc++; }
    }

    zn = readaxis(2);
    if (zn != 0) { 
      if (Debug) { Serial.print(zn); Serial.print(", "); }
      if ( abs((zn-zo)/zn * 100) <= 5) { z = z + zn; zc++; }
    }
    
    //The old value is replaced with the new value.
    xo = xn; yo = yn; zo = zn;    
    if (Debug) Serial.println(); //used to start a new line.
  }

  
  //convert to averages.
  x = x/xc; y = y/yc; z = z/zc;
  // workout the total magnetic field strength (pi-thag)
  tot = sqrt( pow(x,2) + pow(y,2) + pow(z,2) );
  
  //we need to build the POST data, it's just comman seperated values.
  da = "mags="; da.concat(x); da.concat(","); da.concat(y); da.concat(","); da.concat(z); da.concat(","); da.concat(tot);
 
  //Make it easy, create a post URL.
  if (Eth) { //Only if Ethernet is true.
    //----------------------------------------------------------------------------
    // REQUIRED FOR DOMAIN POSTING:
    //----------------------------------------------------------------------------
    if (client.connect("*",80)) { //replace *, example client.connect("www.fred.com",80)
      if (Debug) {
        Serial.println();
        Serial.println("Client connection successfull.");
        Serial.println("POST * HTTP/1.1"); //POST URL, i.e. "POST /FOLDER/form.php HTTP/1.1" (NEEDED BELOW AS WELL)
        Serial.print("Host: ");
        Serial.println(Ethernet.localIP());
        Serial.println("Content-Type: application/x-www-form-urlencoded");
        Serial.print("Content-Length: ");
        Serial.println(da.length());
        Serial.println();
        Serial.print(da);
        Serial.println();
      }
      //----------------------------------------------------------------------------
      // REQUIRED FOR DOMAIN POSTING:
      //----------------------------------------------------------------------------
      client.println("POST * HTTP/1.1"); //Replace *, i.e. "POST /FOLDER/form.php HTTP/1.1"
      client.print("Host: ");
      client.println(Ethernet.localIP());
      client.println("Content-Type: application/x-www-form-urlencoded");
      client.print("Content-Length: ");
      client.println(da.length());
      client.println();
      client.print(da); //This is the data being submitted
      client.stop(); // DISCONNECT FROM THE SERVER
    } else if (Debug) Serial.println("Client connection failed.");
  } else if (Debug) Serial.println(da); //End Ethernet Block, if disabled and debug just show the values.
 
  while ( ( timer <= millis() ) && ( millis() < timer + 60000  ) ) {} //we have to wait for a min since the start of this run.
}
