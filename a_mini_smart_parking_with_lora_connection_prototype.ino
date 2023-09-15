#include <SPI.h>
#include <RH_RF95.h>
#include <Wire.h>
#include <Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_MOSI 9
#define OLED_CLK 10
#define OLED_DC 11
#define OLED_CS 12
#define OLED_RESET 13
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

Servo myservo;
RH_RF95 rf95;

#define s1 2;
#define s2 4;
#define s3 5;
#define s4 6;
#define ser 3;

#define in 7;
#define out 8;

int flag1 = 0, flag2 = 0;

int slots = 4;

//-------------------------Functions-------------------
void loraDataTransfer() {
  rf95.send(data, sizeof(data));
  rf95.waitPacketSent();

  //-------------------------Data Size------------------
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  //-------------------------Receiving Data-------------
  if (rf95.waitAvailableTimeout(3000)) {
    if (rf95.recv(buf, &len)) {
      Serial.print("Reply: ");
      Serial.println((char*)buf);
      display.println("\nLora: working");
    } else {
      Serial.println("Receive failed!");
      display.println("\nLora: Receive failed!");
    }
  } else {
    Serial.println("No reply, is rf95_server running?\n");
    display.println("\nLora: No reply!\nIs the server running?");
  }
}


void setup() {
  Serial.begin(9600);
  myservo.attach(ser);
  myservo.write(0);

  pinMode(s1, INPUT);
  pinMode(s2, INPUT);
  pinMode(s3, INPUT);
  pinMode(s4, INPUT);

  pinMode(in, INPUT);
  pinMode(out, INPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  display.clearDisplay();

  if (!rf95.init()) {
    Serial.println("Lora initialization is failed!");
    display.setTextColor(WHITE);
    display.setCursor(0, 12);
    display.setTextSize(2);
    display.println("LORA INIT.");
    display.println("FAILED");
    display.display();
    delay(1000);
    display.clearDisplay();
    return;
  }

  rf95.setFrequency(868.0);

  display.setTextColor(WHITE);
  display.setCursor(0, 12);
  display.setTextSize(2);
  display.println("SMART");
  display.println("PARKING");
  display.display();
  delay(2000);
  display.clearDisplay();

  slots = slots - digitalRead(s1) - digitalRead(s2) - digitalRead(s3) - digitalRead(s4);
}


void loop() {
  //-------------------------OLED Screen Title-----------------------
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("SMART PARKING");

  //-------------------------Statuses Define-------------------------
  int s1Status = digitalRead(s1);
  int s2Status = digitalRead(s2);
  int s3Status = digitalRead(s3);
  int s4Status = digitalRead(s4);

  int inStatus = digitalRead(in);
  int outStatus = digitalRead(out);

  //-------------------------OLED Display-----------------------------
  if (s1Status == 1)
    display.println("P1: Reserved!");
  else
    display.println("P1: Empty!");

  if (s2Status == 1)
    display.println("P2: Reserved!");
  else
    display.println("P2: Empty!");

  if (s3Status == 1)
    display.println("P3: Reserved!");
  else
    display.println("P3: Empty!");

  if (s4Status == 1)
    display.println("P4: Reserved!");
  else
    display.println("P4: Empty!");

  //-------------------------Servo Status--------------------------------
  if (digitalRead(in) == 0 && flag1 == 0) {
    if (slot > 0) {
      flag1 = 1;
      if (flag2 == 0) {
        myservo.write(180);
        slot = slot - 1;
      }
    }
    //----------If the Garage is Full---------------
    else {
      //----------OLED Display----------------------
      display.clearDisplay();
      display.setTextColor(BLACK, WHITE);
      display.setCursor(0, 12);
      display.setTextSize(2);
      display.println("DON'T PASS\nALL FULL!");

      //----------Lora Sending Data to the Server----
      uint8_t data[] = "\nGarage 1: parking lots are full!\n";
      loraDataTransfer();
    }
  }

  if (digitalRead(out) == 0 && flag2 == 0) {
    flag2 = 1;
    if (flag1 == 0) {
      myservo.write(180);
      slot = slot + 1;
    }
  }

  if (flag1 == 1 && flag2 == 1) {
    delay(1000);
    myservo.write(90);
    flag1 = 0, flag2 = 0;
  }

  //-------------------------Lora Sending Data to the Server-----------
  uint8_t data[] = "\nGarage 1: There are empty parking lots.\n";
  loraDataTransfer();

  display.display();
  delay(500);
  display.clearDisplay();
}