#include <Key.h>
#include <Keypad.h>
#include <LiquidCrystal.h>
#include <Wire.h> // I2C communication
#include <LiquidCrystal_I2C.h> // LCD library
#include <Adafruit_Fingerprint.h>

SoftwareSerial mySerial(2, 3); //TX, RX

#define mySerial Serial1

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);


String text;
String Status = "CLOSED";

int openTime = 3000;
int noteTime = 500;

int BUTTON = A5;

int BUZZER = 22;
int RELAY = 10;


byte y = 0 ;
byte z = 0 ;

int LCD_RS = 24;
int LCD_ENABLE = 26;
int LCD_D4 = 28;
int LCD_D5 = 30;
int LCD_D6 = 32;
int LCD_D7 = 34;



char k[4][4]={
  {'D','#','0','*'},
  {'C','9','8','7'},
  {'B','6','5','4'},
  {'A','3','2','1'}};

byte cpins[4]={2,3,4,5};
byte rpins[4]={6,7,8,9};
char key;

bool drawerOpened = false;
  
Keypad mypad (makeKeymap(k),rpins,cpins,4,4);
//LiquidCrystal lcd(LCD_RS, LCD_ENABLE, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
//LiquidCrystal lcd(LCD_RS, 12, 11, 6, 5, 4, 3);

LiquidCrystal_I2C lcd(0x27, 16, 2);

void OPEN_DOOR(){
  Serial.println("BELEPES ENGEDELYEZVE!");
  Status = "OPEN";

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BELEPES");
  lcd.setCursor(0, 1);
  lcd.print("ENGEDELYEZVE!");
  

  tone(BUZZER, 2000, noteTime);
  digitalWrite(RELAY, HIGH);
  delay(openTime);
  digitalWrite(RELAY, LOW);
}

void WRONG_PASSWORD(){
  Serial.println("A MEGADOTT BELEPO KOD HIBAS!");
  text = "";

  for(int i=1; i<=3; i++){
    tone(BUZZER, 1000, noteTime/3);
    delay(noteTime/2);
  }

  lcd.setCursor(0, 1);
  lcd.print("HIBAS JELSZO!");

  delay(3000);

  lcd.setCursor(0, 1);

  String str;
  for (int i = 0; i<16; i++){
    str = str + " ";
  }

  lcd.print(str);
}

void setup() 
   {
    pinMode(BUTTON, INPUT);
    Serial.begin(9600);
    lcd.init();
    lcd.backlight();
    //lcd.begin(16,2);
    pinMode(BUZZER, OUTPUT);
    pinMode(RELAY, OUTPUT);

    lcd.setCursor(0,0);
    lcd.print("ADD MEG A KODOT:");

    while (!Serial); 
    delay(100);

    Serial.println("Ujjlenyomat olvaso init..");

    finger.begin(57600);

    if (finger.verifyPassword()) {
      Serial.println("Ujjlenyomat olvaso init PASS");
    } else {
      Serial.println("Ujjlenyomas olvaso init FAIL");
      while (1) { delay(1); }
    }

    finger.getParameters();
    Serial.print("Ujjlenyomat olvaso kapacitas: "); Serial.print(finger.capacity); Serial.println(" slot.");
  }

void loop() {
  getFingerprintID();
  delay(100);
  // Serial.println(digitalRead(BUTTON));
  key=mypad.getKey();
  if(key && Status == "CLOSED"){
    text = text + key;
    Serial.println("text: " + text);

    lcd.setCursor(0,1);

    bool displayCode = false;

    String str;
    if(displayCode) {
      str = text;
    } else {
      for (int i = 0; i< text.length(); i++){
        str = str + "*";
      }
    }
      
    
    lcd.print(str);

    if(text == "8097" || text == "9800" || text == "1245"){
      OPEN_DOOR();
    } else {
      if(text.length() == 4){
        WRONG_PASSWORD();
      }
    }
  } else {
    
    if(Status == "OPEN" && !digitalRead(BUTTON) && !drawerOpened){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("FIOK NYITVA!");
      drawerOpened = true;
    } else if(Status == "OPEN" && digitalRead(BUTTON) && drawerOpened){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("FIOK ZARVA!");
      drawerOpened = false;

      delay(2000);
      lcd.clear();
      Status = "CLOSED";
      lcd.setCursor(0,0);
      lcd.print("ADD MEG A KODOT:");
      text = "";
    }

  }
}

uint8_t getFingerprintID() {
  // 1. lépés: Megvárjuk, amíg egy ujjat a szenzorra tesznek
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("IMG ROGZITES");
      break;
    case FINGERPRINT_NOFINGER:
      // Nincs ujj a szenzoron, visszatérünk a loop-ba
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("KOMMUNIKACIO FAIL");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("KEPALKOTAS FAIL");
      return p;
    default:
      Serial.println("ISMERETLEN HIBA (SWITCH: DEFAULT)");
      return p;
  }

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) {
    Serial.println("HIBA A KEP FELDOLGOZASA KOZBEN");
    return p;
  }

  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("\n\n---SIKERES AZONOSITAS!---");
    Serial.print("---Találat a slotban: #"); Serial.println(finger.fingerID);
    Serial.print("--CONFIDENCE: "); Serial.println(finger.confidence);
    Serial.print("\n\n");

    OPEN_DOOR();
    
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("ISMERETLEN UJJLENYOMAT! HOZZAFERES MEGTAGADVA!");
    return p;
  } else {
    Serial.println("FINGERPRINT SEARCH FAIL");
    return p;
  }

  return finger.fingerID;
}
