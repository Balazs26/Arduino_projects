#include <Key.h>
#include <Keypad.h>
String PASSWORD = "1245";
String text;
String Status = "CLOSED";

int openTime = 3000;
int noteTime = 500;

int BUZZER = 10;

byte y = 0 ;
byte z = 0 ;

char k[4][4]={
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}};

byte rpins[4]={2,3,4,5};
byte cpins[4]={6,7,8,9};
char key;
  
Keypad mypad (makeKeymap(k),rpins,cpins,4,4);

void OPEN_DOOR(){
  Serial.println("BELEPES ENGEDELYEZVE!");
  Status = "OPEN";

  tone(BUZZER, 2000, noteTime);
  

  delay(openTime);
  Serial.println("AZ AJTO VISSZA LETT ZARVA!");
  Status = "CLOSED";
  text = "";
}

void WRONG_PASSWORD(){
  Serial.println("A MEGADOTT BELEPO KOD HIBAS!");
  text = "";

  for(int i=1; i<=3; i++){
    tone(BUZZER, 1000, noteTime/3);
    delay(noteTime/2);
  }
}

void setup() 
   {
  
    Serial.begin(9600);
    pinMode(BUZZER, OUTPUT);
  }

void loop() {
  key=mypad.getKey();
  if(key && Status == "CLOSED"){
    text = text + key;
    Serial.println("text: " + text);

    if(text == PASSWORD){
      OPEN_DOOR();
    } else {
      if(text.length() == 4){
        WRONG_PASSWORD();
      }
    }
  }
}

