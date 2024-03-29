#include <Adafruit_GPS.h>
#include <Wire.h>
#include <OneWire.h>
#include <SFE_BMP180.h>
#include <DallasTemperature.h>
#include <Adafruit_CCS811.h>
#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h>

int interiorTempPin = 2;
int exteriorTempPin = 3;
float interiorTemperature = 0;
float exteriorTemperature = 0;
char c;
String NMEA1;
String NMEA2;
uint32_t timer = millis();


#define ALTITUDE 1018.0
#define GPSSerial Serial3

SFE_BMP180 pressure;
OneWire interiorOneWirePin(&interiorTempPin);
OneWire exteriorOneWirePin(&exteriorTempPin);
DallasTemperature interiorTemp(&interiorOneWirePin);
DallasTemperature exteriorTemp(&exteriorOneWirePin);
Adafruit_CCS811 ccs;
Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();
Adafruit_GPS GPS(&GPSSerial);





void startComponents();
void displayData();
void startBMP();
void startCCS();
void startLSM();
void startGPS();
void startInteriorTemp();
void startExteriorTemp();


void setup() {
  Serial.begin(9600);

  startComponents();
  
}

void loop() {
  //gpsRead();
  displayData();
}

void displayData(){
  Serial.println();

  
  char status;//                  for bmp
  double T,P,p0,a;//              for bmp
  sensors_event_t accel, m, g, temp;//for lsm

  gpsRead();

  exteriorTemp.requestTemperatures();
  exteriorTemperature = exteriorTemp.getTempCByIndex(0);
  Serial.print("Exterior Temperature is: ");
  Serial.println(exteriorTemperature);

  Serial.println();
  delay(500);
  

  interiorTemp.requestTemperatures();
  interiorTemperature = interiorTemp.getTempCByIndex(0);
  Serial.print("Interior Temperature is: ");
  Serial.println(interiorTemperature);

  Serial.println();
  delay(500);

  
  lsm.read();
  lsm.getEvent(&accel, &m, &g, &temp);

  Serial.print("Accel X: "); Serial.print(accel.acceleration.x); Serial.print(" m/s^2");
  Serial.print("\tY: "); Serial.print(accel.acceleration.y);     Serial.print(" m/s^2 ");
  Serial.print("\tZ: "); Serial.print(accel.acceleration.z);     Serial.println(" m/s^2 ");

  Serial.print("Mag X: "); Serial.print(m.magnetic.x);   Serial.print(" gauss");
  Serial.print("\tY: "); Serial.print(m.magnetic.y);     Serial.print(" gauss");
  Serial.print("\tZ: "); Serial.print(m.magnetic.z);     Serial.println(" gauss");

  Serial.print("Gyro X: "); Serial.print(g.gyro.x);   Serial.print(" dps");
  Serial.print("\tY: "); Serial.print(g.gyro.y);      Serial.print(" dps");
  Serial.print("\tZ: "); Serial.print(g.gyro.z);      Serial.println(" dps");

  Serial.println();
  delay(500);

  if(ccs.available()){
    if(!ccs.readData()){
      Serial.print("eCO2: ");
      Serial.print(ccs.geteCO2());
      Serial.print("ppm, TVOC: ");
      Serial.println(ccs.getTVOC());
    }
    else{
      Serial.println("ERROR!");
      while(1);
    }
  }
  Serial.println();
  delay(500);

  status = pressure.startTemperature();
  if (status != 0){
    delay(status);
    status = pressure.getTemperature(T);
    if (status != 0){
      status = pressure.startPressure(3);
      if (status != 0){
        delay(status);
        status = pressure.getPressure(P,T);
        if (status != 0){
          Serial.print("absolute pressure: ");
          Serial.print(P,2);
          Serial.print(" mb, ");
          Serial.print(P*0.0295333727,2);
          Serial.println(" inHg");

          p0 = pressure.sealevel(P,ALTITUDE); // we're at 1655 meters (Boulder, CO)
          Serial.print("relative (sea-level) pressure: ");
          Serial.print(p0,2);
          Serial.print(" mb, ");
          Serial.print(p0*0.0295333727,2);
          Serial.println(" inHg");

          a = pressure.altitude(P,p0);
          Serial.print("computed altitude: ");
          Serial.print(a,0);
          Serial.print(" meters, ");
          Serial.print(a*3.28084,0);
          Serial.println(" feet");
        }else Serial.println("error retrieving pressure measurement\n");
      }else Serial.println("error starting pressure measurement\n");
    }else Serial.println("error retrieving temperature measurement\n");
  }else Serial.println("error starting temperature measurement\n");

  Serial.println();
  Serial.println("---------------------------------------------------------------------------------------");
  delay(5000);  // Pause for 5 seconds.
}

void startBMP(){
  if (pressure.begin())
    Serial.println("BMP180 init success");
  else
  {
    Serial.println("BMP180 init fail\n\n");
    while(1); // Pause forever.
  }
  Serial.println();
  Serial.print("provided altitude: ");
  Serial.print(ALTITUDE,0);
  Serial.print(" meters, ");
  Serial.print(ALTITUDE*3.28084,0);
  Serial.println(" feet");
}

void startCCS(){
  if(ccs.begin()){
    Serial.println("ccs Started");
  }else{
    Serial.println("ccs Failed");
    while(1);
  }
}

void startLSM(){
  if (!lsm.begin()){
    Serial.println("unable to initialize the LSM9DS1. Check your wiring!");
    while (1);
  }
  lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_2G);
  lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS);
  lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_245DPS);
}

void startInteriorTemp(){
  interiorTemp.begin();
}

void startExteriorTemp(){
  exteriorTemp.begin();
}

void startGPS(){
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  delay(1000);
}

void gpsRead() {
  // read data from the GPS in the 'main loop'
  clearGPS();
  while(!GPS.newNMEAreceived()){
    c = GPS.read();
  }
  //GPS.parse(GPS.lastNMEA());
  //NMEA1 = GPS.lastNMEA();

  while(!GPS.newNMEAreceived()){
    c = GPS.read();
  }
  //GPS.parse(GPS.lastNMEA());
  //NMEA2 = GPS.lastNMEA();
  
  Serial.print("Time: ");
  if (GPS.hour < 10) { Serial.print('0'); }
    Serial.print(GPS.hour, DEC); Serial.print(':');
  if (GPS.minute < 10) { Serial.print('0'); }
    Serial.print(GPS.minute, DEC); Serial.print(':');
  if (GPS.seconds < 10) { Serial.print('0'); }
    Serial.print(GPS.seconds, DEC); Serial.print('.');
  if (GPS.milliseconds < 10) {
    Serial.print("00");
  } else if (GPS.milliseconds > 9 && GPS.milliseconds < 100) {
    Serial.print("0");
  }
  Serial.println(GPS.milliseconds);
  Serial.print("Date: ");
  Serial.print(GPS.day, DEC); Serial.print('/');
  Serial.print(GPS.month, DEC); Serial.print("/20");
  Serial.println(GPS.year, DEC);
  Serial.print("Fix: "); Serial.print((int)GPS.fix);
  Serial.print(" quality: "); Serial.println((int)GPS.fixquality);
  if (GPS.fix) {
    Serial.print("Location: ");
    Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
    Serial.print(", ");
    Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
    Serial.print("Speed (knots): "); Serial.println(GPS.speed);
    Serial.print("Angle: "); Serial.println(GPS.angle);
    Serial.print("Altitude: "); Serial.println(GPS.altitude);
    Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
  }
}

void clearGPS(){
  // if millis() or timer wraps around, we'll just reset it
  while(!GPS.newNMEAreceived()){
    c=GPS.read();
  }
  GPS.parse(GPS.lastNMEA());
  while(!GPS.newNMEAreceived()){
    c=GPS.read();
  }
  GPS.parse(GPS.lastNMEA());
  while(!GPS.newNMEAreceived()){
    c=GPS.read();
  }
  GPS.parse(GPS.lastNMEA());
}

void startComponents(){
  startBMP();
  startCCS();
  startLSM();
  startInteriorTemp();
  startExteriorTemp();
  startGPS();
}
