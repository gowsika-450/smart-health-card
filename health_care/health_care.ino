#define pulsePin A0
#include <ESP8266WiFi.h>





const int postingInterval = 10 * 1000;

int rate[10];                    
unsigned long sampleCounter = 0;
unsigned long lastBeatTime = 0;
unsigned long lastTime = 0, N;
int BPM = 0;
int IBI = 0;
int P = 512;
int T = 512;
int thresh = 512;
int amp = 100;
int Signal;
boolean Pulse = false;
boolean firstBeat = true;
boolean secondBeat = true;
boolean QS = false;

#include "UbidotsMicroESP8266.h"
#define TOKEN  "BBFF-PFtiLaVDCZcwpK77Py9Sdpjym4emTc"  // Put here your Ubidots TOKEN
#define WIFISSID "No wifi available"
#define PASSWORD "kalaza123"
Ubidots client(TOKEN);
unsigned long lastMillis = 0;
 
int LDR = A0;
int ldr_data = 0;
#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>
#include <DHT.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "25e3db22603642d898a56c32cb3a260d"; //Enter the Auth code which was send by Blink


char ssid[] = "No wifi available";  //Enter your WIFI Name
char pass[] = "kalaza123";  //Enter your WIFI Password

#define DHTPIN 2          // Digital pin 4

// Uncomment whatever type you're using!
#define DHTTYPE DHT11     // DHT 11
//#define DHTTYPE DHT22   // DHT 22, AM2302, AM2321
//#define DHTTYPE DHT21   // DHT 21, AM2301

DHT dht(DHTPIN, DHTTYPE);
SimpleTimer timer;

// This function sends Arduino's up time every second to Virtual Pin (5).
// In the app, Widget's reading frequency should be set to PUSH. This means
// that you define how often to send data to Blynk App.
void sendSensor()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V5, h);  //V5 is for Humidity
  Blynk.virtualWrite(V6, t);  //V6 is for Temperature
}




void setup(){  
  Serial.begin(9600); // See the connection status in Serial Monitor
  Blynk.begin(auth, ssid, pass);

  dht.begin();

  // Setup a function to be called every second
  timer.setInterval(1000L, sendSensor);
    Serial.begin(115200);
    pinMode(LDR, INPUT);
    delay(10);
    client.wifiConnection(WIFISSID, PASSWORD);
}
void loop(){ if (QS == true) {
   Serial.println("BPM: "+ String(BPM));

   QS = false;
   } else if (millis() >= (lastTime + 2)) {
     readPulse();
     lastTime = millis();
   }    
    if (millis() - lastMillis > 1000) {  // the ldr value is sent every 1 second.
  float t = dht.readTemperature();
     ldr_data = analogRead(LDR);
            lastMillis = millis();
          float h = dht.readHumidity();
            client.add("Humidity",h);
             client.add("BPM",BPM);
            client.add("Temprature",t);
            client.sendAll(true);
              Blynk.run(); // Initiates Blynk
  timer.run(); // Initiates SimpleTimer
            }
   
}



void readPulse() {

  Signal = analogRead(pulsePin);              
  sampleCounter += 2;                          
  int N = sampleCounter - lastBeatTime;  

  detectSetHighLow();

  if (N > 250) {  
    if ( (Signal > thresh) && (Pulse == false) && (N > (IBI / 5) * 3) )
      pulseDetected();
  }

  if (Signal < thresh && Pulse == true) {  
    Pulse = false;
    amp = P - T;
    thresh = amp / 2 + T;  
    P = thresh;
    T = thresh;
  }

  if (N > 2500) {
    thresh = 512;
    P = 512;
    T = 512;
    lastBeatTime = sampleCounter;
    firstBeat = true;            
    secondBeat = true;          
  }

}

void detectSetHighLow() {

  if (Signal < thresh && N > (IBI / 5) * 3) {
    if (Signal < T) {                      
      T = Signal;                        
    }
  }

  if (Signal > thresh && Signal > P) {    
    P = Signal;                          
  }                                      

}

void pulseDetected() {
  Pulse = true;                          
  IBI = sampleCounter - lastBeatTime;    
  lastBeatTime = sampleCounter;          

  if (firstBeat) {                      
    firstBeat = false;                
    return;                            
  }
  if (secondBeat) {                    
    secondBeat = false;                
    for (int i = 0; i <= 9; i++) {  
      rate[i] = IBI;
    }
  }

  word runningTotal = 0;                  

  for (int i = 0; i <= 8; i++) {          
    rate[i] = rate[i + 1];            
    runningTotal += rate[i];          
  }

  rate[9] = IBI;                      
  runningTotal += rate[9];            
  runningTotal /= 10;                
  BPM = 60000 / runningTotal;        
  QS = true;
 
  delay(postingInterval);                              
}
