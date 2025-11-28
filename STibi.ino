#include <Key.h>
#include <Keypad.h>
#include <LiquidCrystal.h>

String PASSWORD = "1245";
String text;
String Status = "CLOSED";

int openTime = 3000;
int noteTime = 500;

int BUTTON = 44;

int BUZZER = 22;

byte y = 0 ;
byte z = 0 ;

int LCD_RS = 24;
int LCD_ENABLE = 26;
int LCD_D4 = 28;
int LCD_D5 = 30;
int LCD_D6 = 32;
int LCD_D7 = 34;

char k[4][4]={
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}};

byte rpins[4]={2,3,4,5};
byte cpins[4]={6,7,8,9};
char key;
  
Keypad mypad (makeKeymap(k),rpins,cpins,4,4);
LiquidCrystal lcd(LCD_RS, LCD_ENABLE, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
//LiquidCrystal lcd(LCD_RS, 12, 11, 6, 5, 4, 3);

void OPEN_DOOR(){
  Serial.println("BELEPES ENGEDELYEZVE!");
  Status = "OPEN";

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BELEPES");
  lcd.setCursor(0, 1);
  lcd.print("ENGEDELYEZVE!");

  tone(BUZZER, 2000, noteTime);
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
    lcd.begin(16,2);
    pinMode(BUZZER, OUTPUT);

    lcd.setCursor(0,0);
    lcd.print("ADD MEG A KODOT:");
  }

void loop() {
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

    if(text == PASSWORD){
      OPEN_DOOR();
    } else {
      if(text.length() == 4){
        WRONG_PASSWORD();
      }
    }
  } else {
    bool pushed = false;
    if(Status == "OPEN" and digitalRead(BUTTON)){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("FIOK NYITVA!");
      pushed = true;

    }

  }
}
