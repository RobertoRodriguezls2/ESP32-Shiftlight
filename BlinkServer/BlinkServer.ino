#include <WiFi.h>
#include <FastLED.h>
#include "aWOT.h"
#include "StaticFiles.h"
#include <Preferences.h>
#include "BluetoothSerial.h"
#include "ELMduino.h"

#include <U8g2lib.h>




U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);


#define NUM_LEDS 10
#define BRIGHTNESS 200
#define ELM_PORT   SerialBT
#define DEBUG_PORT Serial
CRGBArray<NUM_LEDS> leds;

BluetoothSerial SerialBT;
ELM327 myELM327;


TaskHandle_t Task1;
TaskHandle_t Task2;
SemaphoreHandle_t baton;


Preferences preferences;
WiFiServer server(80);
Application app;

bool ledOn;

int light = 0;
byte buffer [100];
//int lowEnd = 0;
//int midRange = 0;
//int highEnd = 0;
int lowEnd = 0;
int midRange = 0;
int highEnd = 0;
//int rpm = 0;
int Testrpm = 0;
uint32_t rpm = 0;
int freq = 0;

double G1, G2, G3, G4, Y1, Y2, Y3, R1, R2, R3 = 0;
//double minimum = preferences.getDouble("minimum", 0);
//double middle = preferences.getDouble("middle", 0);
//unsigned int maximum = preferences.getUInt("maximum", 0);


char x = ' ';
const char *soft_ap_ssid = "MyESP32AP";
//unsigned int minimum = preferences.getUInt("minimum", 0);
//  unsigned int middle = preferences.getUInt("middle", 0);
//  unsigned int maximum = preferences.getUInt("maximum", 0);

unsigned int minimum = 800;
unsigned int middle = 1600;
unsigned int maximum = 3500;




void low(Request &req, Response &res) {
  res.print("response recieved");
}

void updatelow(Request &req, Response &res) {
  //  Serial.print("req.read: ");
  //   Serial.println(req.read());
  //  Serial.print("readBytes: ");
  //  Serial.println(req.readBytes(buffer, 100));
  //x = req.readString();
  lowEnd = req.readString().toInt();

  //Serial.print("req.readString LOW: ");
  //Serial.println(lowEnd);
  minimum = lowEnd;
  preferences.putUInt("minimum", minimum);
  //printSet(minimum);

  char cstr[16];
  itoa(lowEnd, cstr, 10);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_luBS18_tf);
  u8g2.drawStr(0, 24, "Low End");
  u8g2.drawStr(0, 50, cstr);
  u8g2.sendBuffer();
  scale();
  return low(req, res);
}



void mid(Request &req, Response &res) {
  res.print("response recieved");
}

void updatemid(Request &req, Response &res) {
  midRange = req.readString().toInt();
  //Serial.print("req.readString MID: ");
  //Serial.println(midRange);
  middle = midRange;
  preferences.putUInt("middle", middle);
  //printSet(middle);
  char cstr[16];
  itoa(midRange, cstr, 10);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_luBS18_tf);
  u8g2.drawStr(0, 24, "Mid End");
  u8g2.drawStr(0, 50, cstr);
  u8g2.sendBuffer();
  scale();
  return mid(req, res);
}


void high(Request &req, Response &res) {
  res.print("response recieved");
}

void updatehigh(Request &req, Response &res) {
  highEnd = req.readString().toInt();
  //Serial.print("req.readString HIGH: ");
  //Serial.println(highEnd);
  maximum = highEnd;
  preferences.putUInt("maximum", maximum);
  //Serial.printf("inside update: Current maxFlash value: %u\n", maximum);
  //printSet(maximum);
  char cstr[16];
  itoa(highEnd, cstr, 10);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_luBS18_tf);
  u8g2.drawStr(0, 24, "High End");
  u8g2.drawStr(0, 50, cstr);
  u8g2.sendBuffer();
  scale();
  return high(req, res);
}

void printSet(int x) {
  char cstr[16];
  itoa(x, cstr, 10);
  u8g2.clearBuffer();
  //u8g2.setFont(u8g2_font_u8glib_4_tf);
  u8g2.setFont(u8g2_font_luBS18_tf);
  u8g2.drawStr(0, 24, "High End");
  u8g2.drawStr(0, 50, cstr);
  u8g2.sendBuffer();


}



void testRPM(Request &req, Response &res) {
  res.print("response recieved");
}

void updatetestRPM(Request &req, Response &res) {
  Testrpm = req.readString().toInt();
  Serial.print("req.readString RPM: ");
  Serial.println(Testrpm);

  //  char cstr[16];
  //  itoa(rpm, cstr, 10);
  //  u8g2.clearBuffer();
  //  u8g2.setFont(u8g2_font_luBS18_tf);
  //  u8g2.drawStr(0, 24, "RPM:");
  //  u8g2.drawStr(0, 50, cstr);
  //  u8g2.sendBuffer();
  fill_solid( leds, NUM_LEDS, CRGB(0, 0, 0));//**&& these 2 lines cause flickering during the test at home but are smooth while in car
  FastLED.show();
  Shift(Testrpm);

  return testRPM(req, res);
}





void Task1code( void * pvParameters );
void Task2code( void * pvParameters );
void setup() {


  //  Serial.printf("Current counter value: %u\n", minFlash);
  //  Serial.printf("Current counter value: %u\n", midFlash);
  //  Serial.printf("Current counter value: %u\n", maxFlash);

  u8g2.begin();
  DEBUG_PORT.begin(115200);
  //SerialBT.setPin("1234");

  ELM_PORT.begin("ArduHUD", true);
  WiFi.softAP(soft_ap_ssid, "12345678");
  Serial.println(WiFi.softAPIP());
  app.use(staticFiles());
  server.begin();

  if (!ELM_PORT.connect("OBDII"))
  {
    DEBUG_PORT.println("Couldn't connect to OBD scanner - Phase 1");

    while (1);
  }

  if (!myELM327.begin(ELM_PORT, true, 2000))
  {
    Serial.println("Couldn't connect to OBD scanner - Phase 2");

    while (1);
  }

  Serial.println("Connected to ELM327");



  u8g2.clearBuffer();
  //u8g2.setFont(u8g2_font_u8glib_4_tf);
  u8g2.setFont(u8g2_font_luBS18_tf);
  u8g2.drawStr(0, 24, "Connected BT");
  u8g2.sendBuffer();

  FastLED.addLeds<NEOPIXEL, 13>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  //  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  xTaskCreatePinnedToCore(
    Task1code,   /* Task function. */
    "Task1",     /* name of task. */
    50000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task1,      /* Task handle to keep track of created task */
    0);          /* pin task to core 0 */
  delay(500);


  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
    Task2code,   /* Task function. */
    "Task2",     /* name of task. */
    50000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task2,      /* Task handle to keep track of created task */
    1);          /* pin task to core 1 */
  delay(500);

  //  *********************** working wifi code connected to my wifi

  //  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  //  while (WiFi.status() != WL_CONNECTED) {
  //    delay(500);
  //    Serial.print(".");
  //  }
  //  Serial.println(WiFi.localIP());
  //  app.use(staticFiles());

  //  app.get("/led", &readLed);
  //  app.put("/led", &updateLed);
  //app.get("/strip", &snd);
  // app.put("/strip", &controller);
  //app.route(staticFiles());
  //  ******************************************************************
  //working by creating a wifi network and connecting to it
  //    WiFi.softAP(soft_ap_ssid, "12345678");
  //    Serial.println(WiFi.softAPIP());
  //    app.use(staticFiles());
  //    server.begin();


  //  setScale();



}
void setScale () {

  xSemaphoreTake( baton, portMAX_DELAY );
  app.get("/LOW", &low);
  app.put("/LOW", &updatelow);
  app.get("/MID", &mid);
  app.put("/MID", &updatemid);
  app.get("/HIGH", &high);
  app.put("/HIGH", &updatehigh);
  app.get("/TEST", &testRPM);
  app.put("/TEST", &updatetestRPM);

  xSemaphoreGive( baton );

  //scale();
}


void scale () {

  G1 = minimum + (middle * .025);
  G2 = G1 + (middle * .05);
  G3 = G1 + (middle * .10);
  G4 = G2 + (middle * .15);
  Y1 = middle;
  Y2 = Y1 + (maximum * .05);
  Y3 = Y2 + (maximum * .02);
  R1 = Y3 + (maximum * .06);
  R2 = R1 + (maximum * .09);
  R3 = maximum;
  //  Serial.print("min: ");
  //  Serial.println(minimum);
  //  Serial.print("G1: ");
  //  Serial.println(G1);
  //  Serial.print("G2: ");
  //  Serial.println(G2);
  //  Serial.print("G3: ");
  //  Serial.println(G3);
  //  Serial.print("G4: ");
  //  Serial.println(G4);
  //  Serial.print("Y1: ");
  //  Serial.println(Y1);
  //  Serial.print("Y2: ");
  //  Serial.println(Y2);
  //  Serial.print("Y3: ");
  //  Serial.println(Y3);
  //  Serial.print("R1: ");
  //  Serial.println(R1);
  //  Serial.print("R2: ");
  //  Serial.println(R2);
  //  Serial.print("R3: ");
  //  Serial.println(R3);

}
void GreenZone(int rpm)
{

  //Serial.print("inside greenzone: ");

  //Serial.println(rpm);

  if (rpm >= minimum && rpm <= G1)
  {
    leds[0] = CRGB::Green;
    FastLED.show();


    //Serial.print("led should be on: ");

  }
  if (rpm >= G1 + 1 && rpm <= G2)
  {
    for (int i = 0; i < 2; i++)
    {
      leds[i] = CRGB::Green;
    }
    FastLED.show();
    //    leds[0] = CRGB::Green;
    //    leds[1] = CRGB::Green;
    //    FastLED.show();

  }
  if (rpm >= G2 + 1 && rpm <= G3)
  {
    for (int i = 0; i < 3; i++)
    {
      leds[i] = CRGB::Green;
    }
    FastLED.show();
    //    leds[0] = CRGB::Green;
    //    leds[1] = CRGB::Green;
    //    leds[2] = CRGB::Green;
    //    FastLED.show();
  }
  if (rpm >= G3 + 1 && rpm <= G4)
  {
    for (int i = 0; i < 4; i++)
    {
      leds[i] = CRGB::Green;
    }
    FastLED.show();
    //    leds[0] = CRGB::Green;
    //    leds[1] = CRGB::Green;
    //    leds[2] = CRGB::Green;
    //    leds[3] = CRGB::Green;
    //    FastLED.show();
  }
}

void YellowZone(int rpm)
{
  if (rpm >= G4 + 1 && rpm <= Y1)
  {
    for (int i = 0; i < 4; i++)
    {
      leds[i] = CRGB::Green;
    }
    //    leds[0] = CRGB::Green;
    //    leds[1] = CRGB::Green;
    //    leds[2] = CRGB::Green;
    //    leds[3] = CRGB::Green;
    leds[4] = CRGB::Yellow;
    FastLED.show();
  }
  if (rpm >= Y1 + 1 && rpm <= Y2)
  {
    for (int i = 0; i < 4; i++)
    {
      leds[i] = CRGB::Green;
    }
    for (int i = 4; i < 6; i++)
    {
      leds[i] = CRGB::Yellow;
    }
    //    leds[0] = CRGB::Green;
    //    leds[1] = CRGB::Green;
    //    leds[2] = CRGB::Green;
    //    leds[3] = CRGB::Green;
    //    leds[4] = CRGB::Yellow;
    //    leds[5] = CRGB::Yellow;
    FastLED.show();
  }
  if (rpm >= Y2 + 1 && rpm <= Y3)
  {
    for (int i = 0; i < 4; i++)
    {
      leds[i] = CRGB::Green;
    }
    for (int i = 4; i < 7; i++)
    {
      leds[i] = CRGB::Yellow;
    }
    //    leds[0] = CRGB::Green;
    //    leds[1] = CRGB::Green;
    //    leds[2] = CRGB::Green;
    //    leds[3] = CRGB::Green;
    //    leds[4] = CRGB::Yellow;
    //    leds[5] = CRGB::Yellow;
    //    leds[6] = CRGB::Yellow;
    FastLED.show();
  }
}
void RedZone(int rpm)
{
  if (rpm >= Y3 + 1 && rpm <= R1)
  {
    for (int i = 0; i < 4; i++)
    {
      leds[i] = CRGB::Green;
    }
    for (int i = 4; i < 7; i++)
    {
      leds[i] = CRGB::Yellow;
    }
    //    leds[0] = CRGB::Green;
    //    leds[1] = CRGB::Green;
    //    leds[2] = CRGB::Green;
    //    leds[3] = CRGB::Green;
    //    leds[4] = CRGB::Yellow;
    //    leds[5] = CRGB::Yellow;
    //    leds[6] = CRGB::Yellow;
    leds[7] = CRGB::Red;
    FastLED.show();
  }
  if (rpm >= R1 + 1 && rpm <= R2)
  {
    for (int i = 0; i < 4; i++)
    {
      leds[i] = CRGB::Green;
    }
    for (int i = 4; i < 7; i++)
    {
      leds[i] = CRGB::Yellow;
    }
    for (int i = 7; i < 9; i++)
    {
      leds[i] = CRGB::Red;
    }
    //    leds[0] = CRGB::Green;
    //    leds[1] = CRGB::Green;
    //    leds[2] = CRGB::Green;
    //    leds[3] = CRGB::Green;
    //    leds[4] = CRGB::Yellow;
    //    leds[5] = CRGB::Yellow;
    //    leds[6] = CRGB::Yellow;
    //    leds[7] = CRGB::Red;
    //    leds[8] = CRGB::Red;
    FastLED.show();
  }
  if (rpm >= R2 + 1 && rpm <= R3)
  {
    for (int i = 0; i < 4; i++)
    {
      leds[i] = CRGB::Green;
    }
    for (int i = 4; i < 7; i++)
    {
      leds[i] = CRGB::Yellow;
    }
    for (int i = 7; i < 10; i++)
    {
      leds[i] = CRGB::Red;
    }
    //    leds[0] = CRGB::Green;
    //    leds[1] = CRGB::Green;
    //    leds[2] = CRGB::Green;
    //    leds[3] = CRGB::Green;
    //    leds[4] = CRGB::Yellow;
    //    leds[5] = CRGB::Yellow;
    //    leds[6] = CRGB::Yellow;
    //    leds[7] = CRGB::Red;
    //    leds[8] = CRGB::Red;
    //    leds[9] = CRGB::Red;
    FastLED.show();
  }
}

void HighZone(int rpm)
{


  if (rpm >= R3 + 1)
  {

    fill_solid( leds, NUM_LEDS, CRGB::Purple);
    FastLED.show();
    fill_solid( leds, NUM_LEDS, CRGB::Black);
    FastLED.show();





  }
}

void Shift(int rpm)
{
  GreenZone(rpm);
  YellowZone(rpm);
  RedZone(rpm);
  HighZone(rpm);
}

void printRPM() {
  int num = rpm;
  char cstr[16];
  itoa(num, cstr, 10);
  u8g2.clearBuffer();
  //u8g2.setFont(u8g2_font_u8glib_4_tf);
  u8g2.setFont(u8g2_font_luBS18_tf);

  //u8g2.drawStr(0, 24, "RPM:");
  u8g2.drawStr(0, 24, cstr);
  u8g2.sendBuffer();

}

void Task1code( void * pvParameters ) {
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());
  baton = xSemaphoreCreateMutex();

  preferences.begin("my-app", false);
  //unsigned int counter = preferences.getUInt("counter", 0);
  //counter++;
  //Serial.printf("Current counter value: %u\n", counter);
  //preferences.putUInt("counter", counter);
  minimum = preferences.getUInt("minimum", 800);
  //Serial.printf("Current minFlash value: %u\n", minimum);
  lowEnd = minimum;
  middle = preferences.getUInt("middle", 1600);
  //Serial.printf("Current midFlash value: %u\n", middle);
  midRange = middle;
  maximum = preferences.getUInt("maximum", 3500);
  //Serial.printf("Current maxFlash value: %u\n", maximum);
  highEnd = maximum;
  //Serial.printf("Current maximum after being assigned value: %i\n", highEnd);
  if ( baton != NULL)
  {
    scale();
    setScale();
  }

  //  scale();
  //  setScale();




  for (;;) {

    // *************************** Working wifi for my wifi
    //    WiFiClient client = server.available();
    //
    //  if (client.connected()) {
    //    app.process(&client);
    //  }
    //*************************************************************

    WiFiClient client = server.available();
    if (client) {
      app.process(&client);
      if (Testrpm == 400) {
        Serial.println("Inside WIFI loop");
        rpm = 2900;
        printRPM();
        Shift(2900);
        client.stop();
        Serial.println("Wifi client closed");
      }


    }

    //    float tempRPM = myELM327.rpm();
    //
    //
    //    if (myELM327.nb_rx_state == ELM_SUCCESS)
    //    {
    //      rpm = (uint32_t)tempRPM;
    //      Serial.print("RPM: "); Serial.println(rpm);
    //      freq = rpm;
    //
    //    }
    //      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    //    myELM327.printError();





    vTaskDelay( 5 / portTICK_PERIOD_MS );
  }
}


void Task2code( void * pvParameters ) {
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());



  FastLED.addLeds<NEOPIXEL, 13>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();




  for (;;) {
    //    WiFiClient client = server.available();
    //    if (client) {
    //      app.process(&client);
    //      if (Testrpm == 400) {
    //        Serial.println("Inside WIFI loop");
    //        rpm = 2900;
    //        printRPM();
    //        Shift(2900);
    //        client.stop();
    //        Serial.println("Wifi client closed");
    //      }
    //
    //
    //    }


    //    int num = rpm;
    //    char cstr[16];
    //  itoa(num, cstr, 10);
    //  u8g2.clearBuffer();
    //  u8g2.setFont(u8g2_font_luBS18_tf);
    //  //u8g2.drawStr(0, 24, "RPM:");
    //  u8g2.drawStr(0, 24, cstr);
    //  u8g2.sendBuffer();
    if (Testrpm == 500) {
      Serial.println("Inside 2nd loops core");
    }
    //    float tempRPM = myELM327.rpm();
    //
    //
    //    if (myELM327.nb_rx_state == ELM_SUCCESS)
    //    {
    //      rpm = (uint32_t)tempRPM;
    //      Serial.print("RPM: "); Serial.println(rpm);
    //      freq = rpm;
    //      printRPM();
    //      fill_solid( leds, NUM_LEDS, CRGB(0, 0, 0));//**&& these 2 lines cause flickering during the test at home but are smooth while in car
    //      FastLED.show();
    //      Shift(freq);
    //
    //    }
    //    else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    //      myELM327.printError();



    //    printRPM();
    //    fill_solid( leds, NUM_LEDS, CRGB(0, 0, 0));//**&& these 2 lines cause flickering during the test at home but are smooth while in car
    //    FastLED.show();
    //    Shift(freq);





  }
}

void loop() {
  //  if(Testrpm == 300)
  //  {
  //    Serial.println("Inside  core 1 loopy loop");
  //  }
  //  Serial.println("Core 1 running");
  //  delay(10000);



  xSemaphoreTake( baton, portMAX_DELAY );


  float tempRPM = myELM327.rpm();


  if (myELM327.nb_rx_state == ELM_SUCCESS)
  {
    rpm = (uint32_t)tempRPM;
    Serial.print("RPM: "); Serial.println(rpm);
    freq = rpm;
    printRPM();
    fill_solid( leds, NUM_LEDS, CRGB(0, 0, 0));//**&& these 2 lines cause flickering during the test at home but are smooth while in car
    FastLED.show();
    Shift(freq);

  }
  else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    myELM327.printError();

  xSemaphoreGive( baton );
}
