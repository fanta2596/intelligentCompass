#include <Wire.h>
#include <MechaQMC5883.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>



#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
MechaQMC5883 qmc;


#define OLED_RESET     3 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


TinyGPS gps;
SoftwareSerial ss(D5, D6);

float LAT = 52.344977;    // thats the destination coordinates
float LONG = 17.647720;
float flat, flon;
bool newData;
float dist;
unsigned long startmillis;
unsigned long currentmillis = 0;

void setup() {
  Wire.begin();
  Serial.begin(115200);
  ss.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  delay(1000);
  qmc.init();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  UI(); // To set the normal mode

}

void loop() {
  int x, y, z;
  int azimuth;
  newData = false;
  unsigned long chars;
  unsigned short sentences, failed;
  unsigned long age;
  bool gpsAvailable = false;

  float delta;
  float delta2;
  float direc;
  //float azimuth; //is supporting float too
  qmc.read(&x, &y, &z);


  float heading = atan2(x, y);

  float declinationAngle = (7.0 + (20.0 / 60.0)) / (180 / M_PI);
  heading += declinationAngle;

  // Correct for heading < 0deg and heading > 360deg
  if (heading < 0)
  {
    heading += 2 * PI;
  }

  if (heading > 2 * PI)
  {
    heading -= 2 * PI;
  }

  // Convert to degrees
  float headingDegrees = heading * 180 / M_PI;
  for (unsigned long start = millis(); millis() - start < 500;)
  {
    while (ss.available())
    {
      char c = ss.read();
      // Serial.write(c); // uncomment this line if you want to see the GPS data flowing
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }


  if (newData) {
    gps.f_get_position(&flat, &flon, &age);
    Serial.println("\nLAT=");
    Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
    Serial.print(" LON=");

    direc =  course_to_Dest(flat, flon, LAT, LONG);
    Serial.print(" DIREC");
    Serial.print(direc);
    // Serial.print(gps.course_to(flat, flon, LAT, LONG));
    gpsAvailable = true;

  }

  if (flat > LAT - 0.000500 && flat < LAT + 0.000500)
  {
    if (flon > LONG - 0.000500 && flon < LONG + 0.000500)
    { Serial.println("AT HOME");
      display.clearDisplay();
      display.setCursor((SCREEN_WIDTH / 2) - 50, SCREEN_HEIGHT / 2);
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.print("WELCOME HOME :) !");
      display.display();
      delay(1000);
    }
  }
  else {
    startmillis = currentmillis;
    float oput;
    if (gpsAvailable == true) {
      UI(); // brauchst dich nicht zu aktualisieren, wenn ehh nichts passiert...
    }
    Serial.print(" Heading = ");
    Serial.print(heading);
    Serial.print(" Degress = ");
    Serial.print(headingDegrees);
    Serial.println();

    if (newData == true) {
      oput = 0;
      delta = 0;

      if (headingDegrees <= direc)
      {
        delta = direc - headingDegrees;
        oput = delta;
      }
      if ( headingDegrees > direc)
      {
        delta = headingDegrees - direc;
        oput = 360 - delta;
      }

      Serial.print(" oput = ");
      Serial.print(oput);
      testdrawline((int16_t)oput);
      delay(1000);
      gpsAvailable = false;

    }
    else
    {


      currentmillis = millis();
      if (currentmillis - startmillis > 10000) {
        display.setCursor(0, 50);
        display.print("No GPS");
        display.display();
      }


    }
  }
}



void testdrawline(int16_t angle )
{

  display.drawLine(display.width() / 2 - 1, display.height() / 2 - 1, (int)(64 + (20 * cos((angle * M_PI / 180) - (90 * M_PI / 180)))),  (int)(32 + (20 * sin((angle * M_PI / 180) - (90 * M_PI / 180)))), WHITE);
  display.display();
}


void drawcircle(void) {
  display.drawCircle(display.width() / 2, display.height() / 2, 20, 1);
  display.display();

}

void UI()
{
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);

  if (newData == true) {
    dist = 6378.388 * acos(sin(LAT * M_PI / 180) * sin(flat * M_PI / 180) + cos(LAT * M_PI / 180) * cos(flat * M_PI / 180) * cos((flon * M_PI / 180 ) - (LONG * M_PI / 180)));
    display.setCursor(0, 0);
    display.print("to Home:");
    display.print(dist);
    display.print("km");
    display.display();
  }
  drawcircle();

}

float course_to_Dest(float lat1, float long1, float lat2, float long2) {

  float dlon = radians(long2 - long1);
  lat1 = radians(lat1);
  lat2 = radians(lat2);
  float a1 = sin(dlon) * cos(lat2);
  float a2 = sin(lat1) * cos(lat2) * cos(dlon);
  a2 = cos(lat1) * sin(lat2) - a2;
  a2 = atan2(a1, a2);
  if (a2 < 0.0)
  {
    a2 += TWO_PI;
  }
  return degrees(a2);
}


