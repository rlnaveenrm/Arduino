#include <Keypad.h>
#include <LiquidCrystal.h>

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keys[ROWS][COLS] = {
  {'1','2','3','C'},
  {'4','5','6','S'},
  {'7','8','9','*'},
  {'*','0','*','R'}
};

byte rowPins[ROWS] = {5, 4, 3, 2}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {9, 8, 7, 6}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

const int rs = 0, en = 1, d4 = 10, d5 = 11, d6 = 12, d7 = 13;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int set = 1;
String inputString = "";
int inputWeight = 0;
int bagsDispensed = 0;

void setup() {
  //Text on Display "Enter wt in g"
  lcd.begin(16, 2);
  lcd.print("Enter wt in gms");
  //Move cursor to second row
  lcd.setCursor(0, 1);
  //Takes input in Numpad
  keypad.setDebounceTime(10);
  while(set){
    char key = keypad.getKey();
    if(key){
      inputString += key;
      lcd.print(inputString);
      if(key == 'S'){
        if(inputString != "0" || inputString != ""){
          set = 0;  
        }  
      }
      if(key == 'C'){
        inputString = "";
      }
    }
  }
  //Saves entered weight
  inputWeight = inputString.toInt();
  lcd.setCursor(0,0);
  lcd.print("No.Bags W " + inputString);
  lcd.setCursor(0,1);
  
  //Sets Load cell to 0

}

void loop() {
  char key = keypad.getKey();
  // when dispense button is pressed
  if(key == 'R'){
      // run stepper motor for x seconds
      // loop until loadcell weight less than set weight
      // run stepper in -ve for x seconds
      bagsDispensed++;
      lcd.print(String(bagsDispensed));
   }
}
