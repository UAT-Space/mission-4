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
float interiorTemperature = 0;
float exteriorTemperature = 0;
char c;
String NMEA1;
String NMEA2;
uint32_t timer = millis();

#define SD_CS          53
#define FILE_BASE_NAME "Data"

#define ALTITUDE 1018.0
#define GPSSerial Serial3

#define RADIO     Serial1

SFE_BMP180 pressure;
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

  size_t write(uint8_t c) {
    size_t count1 = p1.write(c);
    size_t count2 = p2.write(c);
    return min(count1, count2);
  }

private:
  Print &p1, &p2;
};

PrintCombo printer(RADIO, file);

void startComponents();
void displayData();
void startBMP();
void startCCS();
void startLSM();
void startGPS();
void startSD();
void startRadio();
void startInteriorTemp();
void startExteriorTemp();


void setup() {
  Serial.begin(9600);

  startComponents();
  
}

void loop() {
  //gpsRead();
  displayData();
  if (!file.sync() || file.getWriteError()) {
    return;
  }
}

void displayData(){
  printer.println();

  
  char status;//                  for bmp
  double T,P,p0,a;//              for bmp
  sensors_event_t accel, m, g, temp;//for lsm

  gpsRead();

  exteriorTemp.requestTemperatures();
  exteriorTemperature = exteriorTemp.getTempCByIndex(0);
  printer.print("Exterior Temperature is: ");
  printer.println(exteriorTemperature);

  printer.println();
  delay(500);
  

  interiorTemp.requestTemperatures();
  interiorTemperature = interiorTemp.getTempCByIndex(0);
  printer.print("Interior Temperature is: ");
  printer.println(interiorTemperature);

  printer.println();
  delay(500);

  
  lsm.read();
  lsm.getEvent(&accel, &m, &g, &temp);

  printer.print("Accel X: "); printer.print(accel.acceleration.x); printer.print(" m/s^2");
  printer.print("\tY: "); printer.print(accel.acceleration.y);     printer.print(" m/s^2 ");
  printer.print("\tZ: "); printer.print(accel.acceleration.z);     printer.println(" m/s^2 ");

  printer.print("Mag X: "); printer.print(m.magnetic.x);   printer.print(" gauss");
  printer.print("\tY: "); printer.print(m.magnetic.y);     printer.print(" gauss");
  printer.print("\tZ: "); printer.print(m.magnetic.z);     printer.println(" gauss");

  printer.print("Gyro X: "); printer.print(g.gyro.x);   printer.print(" dps");
  printer.print("\tY: "); printer.print(g.gyro.y);      printer.print(" dps");
  printer.print("\tZ: "); printer.print(g.gyro.z);      printer.println(" dps");

  printer.println();
  delay(500);

  if(ccs.available()){
    if(!ccs.readData()){
      printer.print("eCO2: ");
      printer.print(ccs.geteCO2());
      printer.print("ppm, TVOC: ");
      printer.println(ccs.getTVOC());
    }
    else{
      printer.println("ERROR!");
      while(1);
    }
  }
  printer.println();
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
          printer.print("absolute pressure: ");
          printer.print(P,2);
          printer.print(" mb, ");
          printer.print(P*0.0295333727,2);
          printer.println(" inHg");

          p0 = pressure.sealevel(P,ALTITUDE); // we're at 1655 meters (Boulder, CO)
          printer.print("relative (sea-level) pressure: ");
          printer.print(p0,2);
          printer.print(" mb, ");
          printer.print(p0*0.0295333727,2);
          printer.println(" inHg");

          a = pressure.altitude(P,p0);
          printer.print("computed altitude: ");
          printer.print(a,0);
          printer.print(" meters, ");
          printer.print(a*3.28084,0);
          printer.println(" feet");
        }else printer.println("error retrieving pressure measurement\n");
      }else printer.println("error starting pressure measurement\n");
    }else printer.println("error retrieving temperature measurement\n");
  }else printer.println("error starting temperature measurement\n");

  printer.println();
  printer.println("---------------------------------------------------------------------------------------");
  delay(5000);  // Pause for 5 seconds.
}

void startBMP(){
  if (pressure.begin())
    printer.println("BMP180 init success");
  else
  {
    printer.println("BMP180 init fail\n\n");
    while(1); // Pause forever.
  }
  printer.println();
  printer.print("provided altitude: ");
  printer.print(ALTITUDE,0);
  printer.print(" meters, ");
  printer.print(ALTITUDE*3.28084,0);
  printer.println(" feet");
}

void startCCS(){
  if(ccs.begin()){
    printer.println("ccs Started");
  }else{
    printer.println("ccs Failed");
    while(1);
  }
}

void startLSM(){
  if (!lsm.begin()){
    printer.println("unable to initialize the LSM9DS1. Check your wiring!");
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

void startSD() {
  if (!sd.begin(SD_CS, SD_SCK_MHZ(50))) {
    return;
  }

  const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
  char fileName[13] = FILE_BASE_NAME "00.txt";

  if (BASE_NAME_SIZE > 6) {
    return;
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
      return;
    }
  }
  if (!file.open(fileName, O_WRONLY | O_CREAT | O_EXCL)) {
    return;
  }
}

void startRadio() {
  RADIO.begin(9600);
}

void startComponents(){
  startBMP();
  startCCS();
  startLSM();
  startInteriorTemp();
  startExteriorTemp();
  startGPS();
  startSD();
  startRadio();
}
