
//Kiegészítő könyvtárak meghívása
//Ezeket először telepíteni kell az Arduino IDE-ben, különben hibára fut a fordító!
//Könyvtár telepítés: Tools -> Manage libraries (Ctrl + Shift + I)
#include <Key.h>
#include <Keypad.h> //Billentyűzet könyvtár
#include <LiquidCrystal.h>
#include <Wire.h> // I2C kommunikáció
#include <LiquidCrystal_I2C.h> // LCD könyvtár
#include <Adafruit_Fingerprint.h> //AS608 ujjlenyomat olvasó

SoftwareSerial mySerial(2, 3); //TX, RX

#define mySerial Serial1

//Ujjlenyomat olvasó element létrehozása, erre fogunk hivatkozni a program futása során
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

//text változó tárolja a numPad-en beírt szöveget string-ként.
String text;

//Status változó tárolja a fiók jelenlegi állapotát <CLOSED, OPEN>
String Status = "CLOSED";

int openTime = 30000;

//A Buzzer-en egyetlen csipogás időtartama
int noteTime = 500;

//A fiók érzékelő gomb kiosztása
int BUTTON = A5;

//A Buzzer kiosztása
int BUZZER = 22;

//A relé modul kiosztása
int RELAY = 10;

//LCD kiosztás
int LCD_RS = 24;
int LCD_ENABLE = 26;
int LCD_D4 = 28;
int LCD_D5 = 30;
int LCD_D6 = 32;
int LCD_D7 = 34;


//numPad karakterek sorrendje, minden gombhoz hozzá kell rendelni a karakterét.
char k[4][4]={
  {'D','#','0','*'},
  {'C','9','8','7'},
  {'B','6','5','4'},
  {'A','3','2','1'}
};

//numPad lábkiosztása, minden gombhoz külön pin társul.
byte cpins[4]={2,3,4,5};
byte rpins[4]={6,7,8,9};
char key;

bool drawerOpened = false;
  
//numPad element
Keypad mypad (makeKeymap(k),rpins,cpins,4,4);

//LCD I2C element
LiquidCrystal_I2C lcd(0x27, 16, 2);

//OPEN_DOOR függvény, ez hívódik meg a sikeres authentikációkor.
void OPEN_DOOR(){
  //Logolás
  Serial.println("BELEPES ENGEDELYEZVE!");
  //Státusz frissítés
  Status = "OPEN";

  //LCD logolás
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BELEPES");
  lcd.setCursor(0, 1);
  lcd.print("ENGEDELYEZVE!");
  
  //Visszajelző csipogás
  tone(BUZZER, 2000, noteTime);

  //Relé modul aktiválása
  digitalWrite(RELAY, HIGH);
  //openTime-nek megfelelő időnyi várakozás
  delay(openTime);
  //Relé modul deaktiválása
  digitalWrite(RELAY, LOW);
}

//WRONG_PASSWORD függvény, akkor hívódik meg, ha a numPad-en hibás jelszót adunk meg
void WRONG_PASSWORD(){
  //Logolás
  Serial.println("A MEGADOTT BELEPO KOD HIBAS!");
  //text változó alaphelyzetbe állítása, vagyis töröljük az eddig beírt stringet
  text = "";

  //Visszajelző csipogás (3x rövid)
  for(int i=1; i<=3; i++){
    tone(BUZZER, 1000, noteTime/3);
    delay(noteTime/2);
  }

  //LCD logolás
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

//SETUP függvény, ez fut le minden init fázisban.
//Tápellátás helyreállítása / RESET gomb
void setup() 
   {
    //BUTTON-nak megfelelő kivezetés INPUT-nak definiálása, innen fogunk beolvasni egy értéket
    pinMode(BUTTON, INPUT);
    //Soros porton való kommunikációhoz BAUD RÁTA beállítása 9600-ra.
    Serial.begin(9600);
    //LCD inicializálás lefuttatása, alaphelyzetbe állítása.
    lcd.init();
    lcd.backlight();

    //Buzzer és Relé modulhoz társított kivezetések OUTPUT-nak való definiálása.
    pinMode(BUZZER, OUTPUT);
    pinMode(RELAY, OUTPUT);

    //LCD logolás
    lcd.setCursor(0,0);
    lcd.print("ADD MEG A KODOT:");

    //Soros kommunikáció ellenőrzése az ujjlenyomat olvasóhoz
    while (!Serial); 
    delay(100);

    //Logolás
    Serial.println("Ujjlenyomat olvaso init..");

    //Ujjlenyomat olvasó inicializálása, 57600-as Baud rátán (adatátvitel sebessége)
    finger.begin(57600);

    //Ellenőrizzük, hogy sikeres volt-e az ujjlenyomat olvasó inicializálása
    if (finger.verifyPassword()) {
      Serial.println("Ujjlenyomat olvaso init PASS");
    } else {
      Serial.println("Ujjlenyomas olvaso init FAIL");
      while (1) { delay(1); }
    }

    //Amennyiben sikeres volt az init, beolvassuk az összes paramétert az ujjlenyomat olvasóról.
    finger.getParameters();
    //Beolvasott paraméterek logolása a soros monitorba.
    Serial.print("Ujjlenyomat olvaso kapacitas: "); Serial.print(finger.capacity); Serial.println(" slot.");
  }

//LOOP függvény, ez fut végtelenítve az Arduino-n
void loop() {
  //getFingerprintID függvényt minden loopban meghívjuk, ez kezeli az ujjlenyomat olvasást
  //A loop függvény után van definiálva
  getFingerprintID();

  //Minden triggerelés után várunk 100 msec-et, hogy kíméljük a hardvert.
  //Igény esetén ez a delay tovább növelhető, de a jelenlegi érték az ajánlott.
  delay(100);

  //Minden loopban ellenőrizzük, hogy a numPad-en lenyomásra került-e valamelyik gomb
  key=mypad.getKey();

  //Ha le lett nyomva egy gomb a numPad-en, ÉS a fiók állapota CLOSED, akkor...
  if(key && Status == "CLOSED"){

    //Hozzáfűzzük az eddig beírt értékekhez a lenyomott gombot
    text = text + key;
    //Logoljuk a soros monitorra
    Serial.println("text: " + text);

    lcd.setCursor(0,1);

    //displayCode = false -> A számok helyett *-ot jelenítsen meg az LCD-n
    //displayCode = true -> A beírt számokat direktben kijelzi az LCD-n
    bool displayCode = false;

    //Itt dolgozzuk fel az eddig bevitt értékeket.
    //Ha displayCode = true, akkor a bevitt értékek egyből az LCD-re kerülnek
    //Ha displayCode = false, akkor a számok helyett annyi *-ot adunk a kimenetre, amilyen hosszú a string
    String str;
    if(displayCode) {
      str = text;
    } else {
      for (int i = 0; i< text.length(); i++){
        str = str + "*";
      }
    }
      
    //A feldolgozott kimenetet kiírjuk az LCD-re
    lcd.print(str);

    //Jelenleg 3 passCode van definiálva, igény esetén a minta szerint bővíthető.
    if(text == "8097" || text == "9800" || text == "1245"){
      //Ha helyess passCode-ot írtunk be, akkor meghívjuk az OPEN_DOOR függyényt
      //Fentebb van definiálva
      OPEN_DOOR();
    } else {
      //Ha nem egyezik egyik passCode-dal sem, de a hossza elérte a 4-et, akkor 
      //biztosan rossz jelszó lett beírva, meghívjuk a WRONG_PASSWORD függvényt
      if(text.length() == 4){
        WRONG_PASSWORD();
      }
    }
  } else {
    //Ha Status OPEN ÉS még nem nyomtuk le a gombot ÉS még nincs becsukva a fiók
    if(Status == "OPEN" && !digitalRead(BUTTON) && !drawerOpened){
      //LCD logolás
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("FIOK NYITVA!");
      drawerOpened = true;
    } else if(Status == "OPEN" && digitalRead(BUTTON) && drawerOpened){
      //Ha Status OPEN ÉS lenyomtuk a gombot ÉS nyitva van a fiók,
      
      //LCD logolás
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("FIOK ZARVA!");
      drawerOpened = false;

      //Várunk 2 másodpercet
      delay(2000);

      //LCD logolás
      lcd.clear();
      Status = "CLOSED";
      lcd.setCursor(0,0);
      lcd.print("ADD MEG A KODOT:");

      //Töröljük az eddig beírt értéket a text változóból
      text = "";
    }

  }
}

//getFingerprintID függvény, ez fut le minden loopban a triggereléskor
uint8_t getFingerprintID() {

  //Lekéri az ujjlenyomat olvasó státuszát, ha van rajta éppen ujj
  uint8_t p = finger.getImage();

  //Ez a SWITCH blokk vizsgálja a kapott P értéket
  switch (p) {
    //Ujjlenyomatot érzékelz az olvasó
    case FINGERPRINT_OK:
      //Logolás
      Serial.println("IMG ROGZITES");
      break;
    
    //Nem érzékelt ujjlenyomatot az olvasó
    case FINGERPRINT_NOFINGER:
      return p;
    //Hiba történt a kommunikáció során
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("KOMMUNIKACIO FAIL");
      return p;

    //Hiba történt a kép feldolgozása során
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("KEPALKOTAS FAIL");
      return p;

    //Ha eddig egyik ág sem futott volna le, akkor ez fog.
    //Ideális esetben ennek sosem kellene lefutnia, de biztosra megyünk
    //SWITCH blokkokban illik default értéket is definiálni!
    default:
      Serial.println("ISMERETLEN HIBA (SWITCH: DEFAULT)");
      return p;
  }

  //A kapott képet feldolgozható adatstruktúrává alakítjuk az AS608 beépített függvényével
  p = finger.image2Tz();

  //Ha nem sikerült volna a feldolgozás, hibaüzenetet küldünk a soros portra
  if (p != FINGERPRINT_OK) {
    Serial.println("HIBA A KEP FELDOLGOZASA KOZBEN");
    return p;
  }

  //Sikeres feldolgozás esetén a kapott adatstruktúrát megkeressük a tárolt újjlenyomatok között.
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    //Találat esetén kilogoljuk a soros portra az alábbiakat
    Serial.println("\n\n---SIKERES AZONOSITAS!---");
    //A 300 slot közül melyik slotban talált a hardver egyezést.
    Serial.print("---Találat a slotban: #"); Serial.println(finger.fingerID);
    //A kapott egyezés hány pontot ért el a hardver által definiált skálán 
    //"MAGABIZTOSSÁG"
    Serial.print("--CONFIDENCE: "); Serial.println(finger.confidence);
    Serial.print("\n\n");

    //Beléptetés
    OPEN_DOOR();
    
  } else if (p == FINGERPRINT_NOTFOUND) {
    //A beolvasás érzékelt ujjlenyomatot, de ez nem lett hozzáadva egyik slothoz sem
    //Ezért nincs jogosultság a beléptetéshez
    Serial.println("ISMERETLEN UJJLENYOMAT! HOZZAFERES MEGTAGADVA!");
    return p;
  } else {
    //Hiba lépett fel a keresés során
    Serial.println("FINGERPRINT SEARCH FAIL");
    return p;
  }
  //Ha eddig eljutott a kód (nem futott le egyik hibaüzenet águnk sem), akkor
  //A függvény visszatérési értékként visszaadja a beolvasott ujjlenyomathoz társított 
  //Slot azonosítót
  //pl: ez az ujjlenyomat az 5-ös slothoz lett hozzáadva, te az 5-ös felhasználó vagy
  return finger.fingerID;
}
