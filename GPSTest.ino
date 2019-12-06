
#include <Adafruit_GPS.h>

#define GPSSerial Serial3

Adafruit_GPS GPS(&GPSSerial);

String NMEA1;
String NMEA2;
char c;

void setup()
{
  Serial.begin(115200);
  startGPS();
}

void loop() // run over and over again
{
  readGPS();
  delay(1000);
}

void readGPS(){
  clearGPS();
  while(!GPS.newNMEAreceived()){
    c = GPS.read();
  }
  GPS.parse(GPS.lastNMEA());
  NMEA1 = GPS.lastNMEA();

  while(!GPS.newNMEAreceived()){
    c = GPS.read();
  }
  GPS.parse(GPS.lastNMEA());
  NMEA2 = GPS.lastNMEA();

  Serial.println(NMEA1);
  Serial.println(NMEA2);
  Serial.println("");
  
  Serial.print("\nTime: ");
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
void startGPS(){
  GPS.begin(9600);
  GPS.sendCommand("$PGCMD,33,0*6D");
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
  delay(1000);
}
