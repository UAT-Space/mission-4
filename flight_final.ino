//#define DEBUG // uncomment to debug

#include <Adafruit_GPS.h>
#include <Wire.h>
#include <OneWire.h>
#include <SFE_BMP180.h>
#include <DallasTemperature.h>
#include <Adafruit_CCS811.h>
#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h>
#include <SdFat.h>


int interiorTempPin = 2;
int exteriorTempPin = 3;
double T,P,p0,a;
char status;
char c;
uint32_t timer = millis();

sensors_event_t accel, m, g, temp;

#define ALTITUDE 357.0
#define GPSSerial Serial2
#define SD_CS 53
#define FILE_BASE_NAME "Data"
#ifndef DEBUG
#define COM Serial1   // COM is radio
#else
#define COM Serial    // COM is serial monitor
#endif



SFE_BMP180 bmp;
OneWire interiorOneWirePin(&interiorTempPin);
OneWire exteriorOneWirePin(&exteriorTempPin);
DallasTemperature interiorTemp(&interiorOneWirePin);
DallasTemperature exteriorTemp(&exteriorOneWirePin);
Adafruit_CCS811 ccs;
Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();
Adafruit_GPS GPS(&GPSSerial);
SdFat sd;
SdFile file;

class PrintCombo : public Print {
  public:
    PrintCombo(Print &_p1, Print &_p2) : p1(_p1), p2(_p2) {}

    size_t write(uint8_t c){
      size_t count1 = p1.write(c);
      size_t count2 = p2.write(c);
      return min(count1, count2);
    }
  private:
    Print &p1, &p2;
};


PrintCombo printer(Serial, file);

const uint32_t SAMPLE_TIME = 3000;
bool logData = false;
long logTime;

void setup() {
  Serial.begin(9600);

  startComponents();
  
  printer.print("log time");             printer.print(F(","));
  printer.print("GPS hour");             printer.print(F(","));
  printer.print("GPS minute");           printer.print(F(","));
  printer.print("GPS seconds");          printer.print(F(","));
  printer.print("GPS latitude");         printer.print(F(","));
  printer.print("GPS longitude");        printer.print(F(","));
  printer.print("GPS angle");            printer.print(F(","));
  printer.print("GPS speed");            printer.print(F(","));
  printer.print("exterior temperature"); printer.print(F(","));
  printer.print("interior temperature"); printer.print(F(","));
  printer.print("BMP pressure (mb)");    printer.print(F(","));
  printer.print("BMP pressure (inHG)");  printer.print(F(","));
  printer.print("BMP altitude");         printer.print(F(","));
  printer.print("CCS eCO2");             printer.print(F(","));
  printer.print("CCS TVOC");             printer.print(F(","));
  printer.print("Acceleration X");       printer.print(F(","));
  printer.print("Acceleration Y");       printer.print(F(","));
  printer.print("Acceleration Z");       printer.print(F(","));
  printer.print("Magnetic X");           printer.print(F(","));
  printer.print("Magnetic Y");           printer.print(F(","));
  printer.print("Magnetic Z");           printer.print(F(","));
  printer.print("Gyro X");               printer.print(F(","));
  printer.print("Gyro Y");               printer.print(F(","));
  printer.print("Gyro Z");               printer.println(F(","));

  logTime = millis() / SAMPLE_TIME + 1;
  logTime *= SAMPLE_TIME;
}

void loop() {
  if (millis() - logTime >= 3000) logData = true;
  
  if (logData == true) {
    processData();
    if (!file.sync() || file.getWriteError()){
      error(1);
    }
    logData = false;
    logTime += 3000;
  }
}

void processData(){
  exteriorTemp.requestTemperatures();
  interiorTemp.requestTemperatures();
  readGPS();
  readBMP();
  readCCS();
  lsm.read();
  lsm.getEvent(&accel, &m, &g, &temp);

//  p0 = bmp.sealevel(P,ALTITUDE);
//  a = bmp.altitude(P,p0);

  
  printer.print(logTime);                         printer.print(F(","));
  printer.print(GPS.hour);                        printer.print(F(","));
  printer.print(GPS.minute);                      printer.print(F(","));
  printer.print(GPS.seconds);                     printer.print(F(","));
  printer.print(GPS.latitude, 4);                 printer.print(F(","));
  printer.print(GPS.longitude, 4);                printer.print(F(","));
  printer.print(GPS.altitude);                    printer.print(F(","));
  printer.print(GPS.angle);                       printer.print(F(","));
  printer.print(GPS.speed);                       printer.print(F(","));
  printer.print(exteriorTemp.getTempCByIndex(0)); printer.print(F(","));
  printer.print(interiorTemp.getTempCByIndex(0)); printer.print(F(","));
//  printer.print(P,2);                             printer.print(F(","));
//  printer.print(P*0.0295333727,2);                printer.print(F(","));
//  printer.print(a, 0);                            printer.print(F(","));
  printer.print(ccs.geteCO2());                   printer.print(F(","));
  printer.print(ccs.getTVOC());                   printer.print(F(","));
  printer.print(accel.acceleration.x);            printer.print(F(","));
  printer.print(accel.acceleration.y);            printer.print(F(","));
  printer.print(accel.acceleration.z);            printer.print(F(","));
  printer.print(m.magnetic.x);                    printer.print(F(","));
  printer.print(m.magnetic.y);                    printer.print(F(","));
  printer.print(m.magnetic.z);                    printer.print(F(","));
  printer.print(g.gyro.x);                        printer.print(F(","));
  printer.print(g.gyro.y);                        printer.print(F(","));
  printer.print(g.gyro.z);                        printer.println(F(","));
  
  
  /*
  printer.println(exteriorTemp.getTempCByIndex(0));
  printer.println(interiorTemp.getTempCByIndex(0));
  printer.print("Time: ");
  if (GPS.hour < 10) { printer.print('0'); }
    printer.print(GPS.hour, DEC); printer.print(':');
  if (GPS.minute < 10) { printer.print('0'); }
    printer.print(GPS.minute, DEC); printer.print(':');
  if (GPS.seconds < 10) { printer.print('0'); }
    printer.print(GPS.seconds, DEC); printer.print('.');
  if (GPS.milliseconds < 10) {
    printer.print("00");
  } else if (GPS.milliseconds > 9 && GPS.milliseconds < 100) {
    printer.print("0");
  }
  printer.println(GPS.milliseconds);
  printer.print("Date: ");
  printer.print(GPS.day, DEC); printer.print('/');
  printer.print(GPS.month, DEC); printer.print("/20");
  printer.println(GPS.year, DEC);
  printer.print("Fix: "); printer.print((int)GPS.fix);
  printer.print(" quality: "); printer.println((int)GPS.fixquality);
  if (GPS.fix) {
    printer.print("Location: ");
    printer.print(GPS.latitude, 4); printer.print(GPS.lat);
    printer.print(", ");
    printer.print(GPS.longitude, 4); printer.println(GPS.lon);
    printer.print("Speed (knots): "); printer.println(GPS.speed);
    printer.print("Angle: "); printer.println(GPS.angle);
    printer.print("Altitude: "); printer.println(GPS.altitude);
    printer.print("Satellites: "); printer.println((int)GPS.satellites);
  }
  
  printer.print("absolute pressure: ");
  printer.print(P,2);
  printer.print(" mb, ");
  printer.print(P*0.0295333727,2);
  printer.println(" inHg");
  p0 = bmp.sealevel(P,ALTITUDE); // we're at 1655 meters (Boulder, CO)
  printer.print("relative (sea-level) pressure: ");
  printer.print(p0,2);
  printer.print(" mb, ");
  printer.print(p0*0.0295333727,2);
  printer.println(" inHg");
  a = bmp.altitude(P,p0);
  printer.print("computed altitude: ");
  printer.print(a,0);
  printer.print(" meters, ");
  printer.print(a*3.28084,0);
  printer.println(" feet");
  printer.print("eCO2: ");
  printer.print(ccs.geteCO2());
  printer.print("ppm, TVOC: ");
  printer.println(ccs.getTVOC());
  printer.print("Accel X: "); printer.print(accel.acceleration.x); printer.print(" m/s^2");
  printer.print("\tY: "); printer.print(accel.acceleration.y);     printer.print(" m/s^2 ");
  printer.print("\tZ: "); printer.print(accel.acceleration.z);     printer.println(" m/s^2 ");
  printer.print("Mag X: "); printer.print(m.magnetic.x);   printer.print(" gauss");
  printer.print("\tY: "); printer.print(m.magnetic.y);     printer.print(" gauss");
  printer.print("\tZ: "); printer.print(m.magnetic.z);     printer.println(" gauss");
  printer.print("Gyro X: "); printer.print(g.gyro.x);   printer.print(" dps");
  printer.print("\tY: "); printer.print(g.gyro.y);      printer.print(" dps");
  printer.print("\tZ: "); printer.print(g.gyro.z);      printer.println(" dps");
  printer.println("-------------------------------------------------------");
  */
}

void readCCS(){
  if(ccs.available()){
    if(!ccs.readData()){
      
    }
    else{
      error(2);
      Serial.println("HELLO");
    }
  }
}

void readBMP(){
  status = bmp.startTemperature();
  if (status != 0){
    delay(status);
    status = bmp.getTemperature(T);
    if (status != 0){
      status = bmp.startPressure(3);
      if (status != 0){
        delay(status);
        status = bmp.getPressure(P,T);
        if (status != 0){
          
        }else Serial.println("error retrieving pressure measurement\n");
      }else Serial.println("error starting pressure measurement\n");
    }else Serial.println("error retrieving temperature measurement\n");
  }else Serial.println("error starting temperature measurement\n");
}

void readGPS(){
  clearGPS();
  while(!GPS.newNMEAreceived()){
    c = GPS.read();
  }

  while(!GPS.newNMEAreceived()){
    c = GPS.read();
  }
  
  
}

void clearGPS(){
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

void startBMP(){
  if(!bmp.begin()){
    error(3);
  }
}

void startCCS(){
  if(!ccs.begin()){
    error(4);
  }
}

void startLSM(){
  if(!lsm.begin()){
    error(5);
  }
  lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_2G);
  lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS);
  lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_245DPS);
}

void startInteriorTemp(){
  interiorTemp.begin();
}

void startExteriorTemp(){
  interiorTemp.begin();
}

void startGPS(){
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  delay(1000);
}

void startSD(){
  if (!sd.begin(SD_CS, SD_SCK_MHZ(50))) {
    error(6);
  }

  const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
  char fileName[13] = FILE_BASE_NAME "00.csv";

  if (BASE_NAME_SIZE > 6) {
    error(7);
  }

  while (sd.exists(fileName)) {
    if (fileName[BASE_NAME_SIZE + 1] != '9') {
      fileName[BASE_NAME_SIZE + 1]++;
    }
    else if (fileName[BASE_NAME_SIZE] != '9') {
      fileName[BASE_NAME_SIZE + 1] = '0';
      fileName[BASE_NAME_SIZE]++;
    }
    else {
      error(8);
    }
  }
  if (!file.open(fileName, O_WRONLY | O_CREAT | O_EXCL)) {
    error(9);
  }
}

void startCOM(){
  COM.begin(9600);
}

void error(uint8_t c){
  Serial.print(F("ERROR: "));
  Serial.println(c);
}

void startComponents(){
  startBMP();
  startCCS();
  startLSM();
  startInteriorTemp();
  startExteriorTemp();
  startGPS();
  startSD();
  startCOM();
}
