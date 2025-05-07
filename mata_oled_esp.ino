#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
#include "BluetoothSerial.h"
BluetoothSerial bluetooth;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#include "robo.h"
roboEyes roboEyes;

// For setting mood expression and eye position
byte mood = 0;
byte position = 0;
bool showConfigMode = 0;
unsigned long showConfigModeTimer = 0;
int showConfigModeDuration = 1500;

// Configuration modes
#define EYES_WIDTHS 0
#define EYES_HEIGHTS 1
#define EYES_BORDERRADIUS 2
#define EYES_SPACEBETWEEN 3
#define CYCLOPS_TOGGLE 4
#define CURIOUS_TOGGLE 5
#define PREDEFINED_POSITIONS 6
byte configMode = 6;
// #define LED_BUILTIN 1  // LED bawaan di GPIO1 (TX)

void setup() {
  Serial.begin(38400);
  bluetooth.begin("OLED Eye");
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); // LED mati saat mulai (HIGH = OFF, LOW = ON)

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }


  roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 100);
  roboEyes.close();
  roboEyes.setPosition(0);
  roboEyes.setAutoblinker(ON, 3, 2);
  roboEyes.setIdleMode(ON, 3, 1);

  printHelp();
}

void loop() {
  if (!showConfigMode) {
    roboEyes.update();
  } else {
    displayConfigMode();
    if(millis() >= showConfigModeTimer + showConfigModeDuration) {
      showConfigMode = 0;
    }
  }

  // Cek data dari Serial Monitor
  if (Serial.available() > 0) {
    processSerialCommand(Serial.readStringUntil('\n'));
  }

  // Cek data dari Bluetooth
  if (bluetooth.available() > 0) {
    String btCommand = bluetooth.readStringUntil('\n');
    btCommand.trim();
    processSerialCommand(btCommand);
  }
}

void processSerialCommand(String cmd) {
  cmd.trim(); // Hapus karakter whitespace

  if (cmd == "mood") {
    if (mood < 3) mood++; else mood = 0;
    updateMood();
  } else if (cmd == "angry" || cmd == "Q 5") {
    roboEyes.setMood(ANGRY);
  } else if (cmd == "tired" || cmd == "S 0") { //duduk
    roboEyes.setMood(TIRED);
  } else if (cmd == "happy") { // Wave and Shake
    roboEyes.setMood(HAPPY);
  } else if (cmd == "W 5" || cmd == "H 5"){
    roboEyes.setMood(HAPPY);
    roboEyes.setPosition(0);
    roboEyes.idle = false;
  } else if (cmd == "normal") {
    roboEyes.setMood(0);
  } else if (cmd == "idle" || cmd == "S 1" || cmd == "I 0") { // stand and init
    roboEyes.setPosition(0);
    roboEyes.idle = true;
  } else if (cmd == "free" || cmd == "F 1" || cmd == "B 1") { // Maju
    roboEyes.setPosition(0);
    roboEyes.idle = false;
  } else if (cmd == "laugh") {
    roboEyes.anim_laugh();
  } else if (cmd == "confused") {
    roboEyes.anim_confused();
  } else if (cmd == "flicker") {
    static bool flickerState = false;
    flickerState = !flickerState;
    roboEyes.setHFlicker(flickerState ? ON : OFF, 1);
  } else if (cmd == "kanan" || cmd == "R 1") { // Kanan
    roboEyes.eyeLxNext = constrain(-50, 0, roboEyes.getScreenConstraint_X());
    roboEyes.eyeLyNext = constrain(0, 0, roboEyes.getScreenConstraint_Y());
    roboEyes.idle = false;
  } else if (cmd == "kiri" || cmd == "L 1") { // Kiri
    roboEyes.eyeLxNext = constrain(50, 0, roboEyes.getScreenConstraint_X());
    roboEyes.eyeLyNext = constrain(0, 0, roboEyes.getScreenConstraint_Y());
    roboEyes.idle = false;
  } else if (cmd == "yPos") {
    roboEyes.eyeLyNext = constrain(50, 0, roboEyes.getScreenConstraint_Y());
  } else if (cmd == "help") {
    printHelp();
  }if (cmd.startsWith("w=")) {
    int val = cmd.substring(2).toInt();
    if (val >= 1 && val <= 100) { // batasan nilai w
      roboEyes.setWidth(val, val);
    }
  } else if (cmd.startsWith("h=")) {
    int val = cmd.substring(2).toInt();
    if (val >= 1 && val <= 100) {
      roboEyes.setHeight(val, val);
    }
  } else if (cmd.startsWith("b=")) {
    int val = cmd.substring(2).toInt();
    if (val >= 0 && val <= 50) {
      roboEyes.setBorderradius(val, val);
    }
  } else if (cmd.startsWith("s=")) {
    int val = cmd.substring(2).toInt(); // bisa negatif
    roboEyes.setSpacebetween(val);
  }
}

void processIncrement() {
  showConfigMode = 0;
  switch(configMode) {
    case EYES_WIDTHS:
      roboEyes.eyeLwidthNext++;
      roboEyes.eyeRwidthNext++;
      break;
    case EYES_HEIGHTS:
      roboEyes.eyeLheightNext++;
      roboEyes.eyeRheightNext++;
      roboEyes.eyeLheightDefault++;
      roboEyes.eyeRheightDefault++;
      break;
    case EYES_BORDERRADIUS:
      if(roboEyes.eyeLborderRadiusNext < 50) {
        roboEyes.eyeLborderRadiusNext++;
        roboEyes.eyeRborderRadiusNext++;
      }
      break;
    case EYES_SPACEBETWEEN:
      roboEyes.spaceBetweenNext++;
      break;
    case CYCLOPS_TOGGLE:
      roboEyes.setCyclops(OFF);
      break;
    case CURIOUS_TOGGLE:
      roboEyes.setCuriosity(ON);
      break;
    case PREDEFINED_POSITIONS:
      if (position < 8) position++; else position = 0;
      updatePosition();
      break;
  }
}

void processDecrement() {
  showConfigMode = 0;
  switch(configMode) {
    case EYES_WIDTHS:
      roboEyes.eyeLwidthNext--;
      roboEyes.eyeRwidthNext--;
      break;
    case EYES_HEIGHTS:
      roboEyes.eyeLheightNext--;
      roboEyes.eyeRheightNext--;
      roboEyes.eyeLheightDefault--;
      roboEyes.eyeRheightDefault--;
      break;
    case EYES_BORDERRADIUS:
      if(roboEyes.eyeLborderRadiusNext > 0) {
        roboEyes.eyeLborderRadiusNext--;
        roboEyes.eyeRborderRadiusNext--;
      }
      break;
    case EYES_SPACEBETWEEN:
      roboEyes.spaceBetweenNext--;
      break;
    case CYCLOPS_TOGGLE:
      roboEyes.setCyclops(ON);
      break;
    case CURIOUS_TOGGLE:
      roboEyes.setCuriosity(OFF);
      break;
    case PREDEFINED_POSITIONS:
      if (position > 0) position--; else position = 8;
      updatePosition();
      break;
  }
}

void updateMood() {
  switch(mood) {
    case 0: roboEyes.setMood(0); break;
    case 1: roboEyes.setMood(TIRED); break;
    case 2: roboEyes.setMood(ANGRY); break;
    case 3: roboEyes.setMood(HAPPY); break;
  }
}

void updatePosition() {
  switch(position) {
    case 0: roboEyes.setPosition(0); break;
    case 1: roboEyes.setPosition(N); break;
    case 2: roboEyes.setPosition(NE); break;
    case 3: roboEyes.setPosition(E); break;
    case 4: roboEyes.setPosition(SE); break;
    case 5: roboEyes.setPosition(S); break;
    case 6: roboEyes.setPosition(SW); break;
    case 7: roboEyes.setPosition(W); break;
    case 8: roboEyes.setPosition(NW); break;
  }
}

void displayConfigMode() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,3);
  
  switch(configMode) {
    case EYES_WIDTHS:
      display.println("Widths"); 
      display.println(roboEyes.eyeLwidthCurrent);
      break;
    case EYES_HEIGHTS:
      display.println("Heights"); 
      display.println(roboEyes.eyeLheightCurrent);
      break;
    case EYES_BORDERRADIUS:
      display.println("Border \nRadius"); 
      display.println(roboEyes.eyeLborderRadiusCurrent);
      break;
    case EYES_SPACEBETWEEN:
      display.println("Space \nBetween"); 
      display.println(roboEyes.spaceBetweenCurrent);
      break;
    case CYCLOPS_TOGGLE:
      display.println("Cyclops \nToggle");
      break;
    case CURIOUS_TOGGLE:
      display.println("Curiosity \nToggle");
      break;
    case PREDEFINED_POSITIONS:
      display.println("Predefined\nPositions");
      roboEyes.setIdleMode(0);
      roboEyes.setPosition(0);
      break;
  }
  display.display();
}

void printHelp() {
  Serial.println(F("Robo Eyes Serial Control Commands:"));
  Serial.println(F("m - Change mood (DEFAULT/TIRED/ANGRY/HAPPY)"));
  Serial.println(F("l - Trigger laugh animation"));
  Serial.println(F("c - Trigger confused animation"));
  Serial.println(F("f - Toggle flicker animation"));
  Serial.println(F("g - Change configuration mode"));
  Serial.println(F("+ - Increment current config value"));
  Serial.println(F("- - Decrement current config value"));
  Serial.println(F("i - Toggle idle mode"));
  Serial.println(F("x[value] - Set X position (e.g., x100)"));
  Serial.println(F("y[value] - Set Y position (e.g., y50)"));
  Serial.println(F("h - Show this help message"));
}