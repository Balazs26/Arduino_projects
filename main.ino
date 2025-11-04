
#include "EasyMFRC522.h"


#define MAX_STRING_SIZE 100  
#define BLOCK 1          

int openTime = 2000;

EasyMFRC522 rfidReader(53, 5); 

String removeQuotes(String input) {
  String result = "";
  for (unsigned int i = 0; i < input.length(); i++) {
    if (input[i] != '"') { 
      result += input[i];
    }
  }
  return result;
}


void setup() {
  Serial.begin(9600);
  Serial.setTimeout(20000);
  rfidReader.init(); 

  pinMode(3, OUTPUT);
}



void loop() {



  bool success;
  do {
    success = rfidReader.detectTag();    
    delay(50);
  } while (!success);

  
  int result;
  char stringBuffer[MAX_STRING_SIZE];

  result = rfidReader.readFile(BLOCK, "mylabel", (byte*)stringBuffer, MAX_STRING_SIZE);

  stringBuffer[MAX_STRING_SIZE-1] = 0;

  if (result >= 0) {
    if(removeQuotes(stringBuffer) == "OPEN") {
      OPEN();
    } else {
      Serial.println("Belepes megtagadva!");
    }
  }


  rfidReader.unselectMifareTag();
  delay(100);
}

void OPEN(){
  Serial.println("Belepes engedelyezve!");

  digitalWrite(3, HIGH);
  delay(openTime);
  digitalWrite(3, LOW);
}


