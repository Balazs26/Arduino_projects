#include <Keypad.h>

const byte ROWS = 4;
const byte COLS = 4;

byte columnPins[COLS] = {10, 11, 12, A4};
byte rowPins[ROWS] = {2, 4, 7, 8};

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, columnPins, ROWS, COLS);



void setup() {
  Serial.begin(9600);

}

void loop() {
  char key = keypad.getKey();
  Serial.println(key);

}
