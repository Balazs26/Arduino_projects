 
#include "EasyMFRC522.h"
#include <LiquidCrystal.h>
#define MAX_STRING_SIZE 100  
#define BLOCK 1    

int openTime = 2000;
int button = 2;
bool waitForMasterCard = false;
bool waitForBlankCard = false;
int LCD_RS = 12;
int LCD_ENABLE = 11;
int LCD_D4 = 10;
int LCD_D5 = 9;
int LCD_D6 = 8;
int LCD_D7 = 7;
int BUZZER = 48;
int noteTime = 500;

bool readBlocking = false;



EasyMFRC522 rfidReader(53, 5);
LiquidCrystal lcd(LCD_RS, LCD_ENABLE, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

String removeQuotes(String input) {
  String result = "";
  for (unsigned int i = 0; i < input.length(); i++) {
    if (input[i] != '"') { 
      result += input[i];
    }
  }
  return result;
}

void readBlock(int blockTime){
  readBlocking = true;
  delay(blockTime);
  readBlocking = false;
}

void setup() {
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  Serial.begin(9600);
  Serial.setTimeout(20000);
  rfidReader.init(); 
  pinMode(3, OUTPUT);
  pinMode(14, OUTPUT);
  digitalWrite(14, HIGH);
  pinMode(button, INPUT_PULLUP);
  lcd.begin(16,2);
  lcdWrite("OLVASD LE A", "KARTYAD");
  pinMode(BUZZER, OUTPUT);
}

void openSound(){
  tone(BUZZER, 2000, noteTime);
}

void deniedSound(){
  for(int i=1; i<=3; i++){
    tone(BUZZER, 1000, noteTime/3);
    delay(noteTime/2);
  }
}



void loop() {
  bool success;
  do {
    success = rfidReader.detectTag();    
    delay(50);
    if(!waitForMasterCard) {
      checkForButton();
    }
  } while (!success);
  int result;
  char stringBuffer[MAX_STRING_SIZE];
  result = rfidReader.readFile(BLOCK, "mylabel", (byte*)stringBuffer, MAX_STRING_SIZE);
  stringBuffer[MAX_STRING_SIZE-1] = 0;
  bool skipUnknownMsg = false;
  if (result > 0) {
    bool skipErrMsg = false;
    bool skipEntrance = false;
    String data;
    data = removeQuotes(stringBuffer);
    if(waitForMasterCard){
        skipEntrance = true;
        if(data != "MASTER"){
            waitForMasterCard = false;
            lcdWrite("OLVASD LE A", "KARTYAD");
            skipErrMsg = true;
            skipUnknownMsg = true;
            readBlock(1000);
        }
    } else if(waitForBlankCard){
        if(removeQuotes(stringBuffer) != "MASTER") {
            skipEntrance = true;
            skipErrMsg = true;
            write(1);
            skipUnknownMsg = true;
            readBlock(1000);

        } else {
            lcdWrite("KLONOZAS", "SIKERTELEN!");
            waitForMasterCard = false;
            waitForBlankCard = false;
            delay(3000);
            skipEntrance = true;
            lcdWrite("OLVASD LE A", "KARTYAD");
            readBlock(1000);
        }
    }

    if(result >= 0 && (data == "OPEN" || data == "MASTER")) {
        if(waitForMasterCard) {
            if(data == "MASTER") {
                waitForMasterCard = false;
                waitForBlankCard = true;
                lcdWrite("MASOLAS...", "UJ KARTYA");
            }
            skipEntrance = true;
        } else {
            if(!skipEntrance && !waitForMasterCard && !waitForBlankCard) {
              if(data == "OPEN" || data == "MASTER"){
                OPEN();
              }
            }
        }
    } else {
        if(!skipErrMsg) {
            Serial.println("Belepes megtagadva!");
            deniedSound();
            lcd.setCursor(0,0);
            lcdWrite("BELEPES", "MEGTAGADVA!");
            delay(openTime);
            lcd.setCursor(0,0);
            lcdWrite("OLVASD LE A", "KARTYAD");
        }
    }
  } else if(success) {
    write(10);
    if(!skipUnknownMsg) {
        lcdWrite("ISMERETLEN", "KARTYA!");
        delay(openTime);
    }
    lcdWrite("OLVASD LE A", "KARTYAD");
  }
  rfidReader.unselectMifareTag();
  delay(100);
}

void checkForButton(){
  int pressed = not digitalRead(button);

  if(pressed) {
    waitForMasterCard = true;
    lcdWrite("OLVASD LE A", "MASTERT!");
  }
}

void write(int a){
    char writeStringBuffer[100];
    int result;
    if(a == 10) {
      strcpy(writeStringBuffer, "10");
    } else if(a == 1) {
      strcpy(writeStringBuffer, "OPEN");
      lcdWrite("KLONOZAS", "SIKERES!");
      
    }
    int stringSize = strlen(writeStringBuffer);
    
    result = rfidReader.writeFile(BLOCK, "mylabel", (byte*)writeStringBuffer, stringSize+1);

    if(a == 1){
      delay(3000);
    }
    

    waitForMasterCard = false;
    waitForBlankCard = false;
    lcdWrite("OLVASD LE A", "KARTYAD");
    readBlock(1000);
}

void OPEN(){
  Serial.println("Belepes engedelyezve!");
  lcd.setCursor(0,0);
  lcdWrite("BELEPES", "ENGEDELYEZVE!");
  openSound();
  digitalWrite(3, HIGH);
  delay(openTime);
  digitalWrite(3, LOW);
  lcd.setCursor(0,0);
  lcdWrite("OLVASD LE A", "KARTYAD");
}

void lcdWrite(String line1, String line2){
  lcd.clear();

  if(line1){
    lcd.setCursor(0, 0);
    lcd.print(line1);
  }

  if(line2){
    lcd.setCursor(0, 1);
    lcd.print(line2);
  }
}
