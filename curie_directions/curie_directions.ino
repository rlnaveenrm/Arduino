#include <CurieBLE.h>
#include <Adafruit_NeoPixel.h>
 
#define PIN 6
Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, PIN, NEO_GRB + NEO_KHZ800);

BLEPeripheral bP;

BLEService dirSer("6f31586c-265d-4008-8d3b-8d88bf51a94e");

BLEIntCharacteristic dirChar("2ce8c149-396f-4f01-8b18-ac934c9cb262",BLERead | BLENotify | BLEWrite);

uint16_t dirIns = 0;
uint8_t dirDis = 0;
uint8_t dirPat = 0;

void setup() {
  pinMode(13,OUTPUT);
  Serial.begin(9600);
  bP.setLocalName("**********");
  bP.setAdvertisedServiceUuid(dirSer.uuid()); 
  bP.addAttribute(dirSer); 
  bP.addAttribute(dirChar); 
  dirChar.setValue(dirIns);
  bP.begin();
  strip.begin();
  strip.setBrightness(5);
  strip.show();    
  
}

void loop() {
  BLECentral central = bP.central();
  if(central){
    digitalWrite(13,HIGH);
    while (central.connected()) {
      if(dirChar.written()){
        dirIns = dirChar.value();
        if(dirIns == 100){
          navigateStart();          
        }else if(dirIns == 999){
          navigateEnd();          
        }else{
          navigate(dirIns);
        }          
                
      }
    }
  }
  else{
  Serial.println(F("Disconnected from central: "));
  digitalWrite(13,LOW);
  }

}




void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}
void refresh(){
  colorWipe(strip.Color(0,0,0), 0);
}




void navigateStart(){
  colorWipe(strip.Color(0, 255, 0), 50);
  refresh();
  
}
void navigateEnd(){
  float maxBrightness = 100;
  float speedFactor = 0.05; 
  uint8_t stepDelay = 5;   
  for (int i = 0; i < 450; i++) {    
    float intensity = maxBrightness /2.0 * (1.0 + sin(speedFactor * i));
    strip.setBrightness(intensity);      
    for (uint8_t i=0; i<16; i++) {
      strip.setPixelColor(i, 255, 255, 255);
    }      
    strip.show();      
    delay(stepDelay);
  }
  strip.setBrightness(5);
  for(uint8_t j = 0; j<16; j++){
    strip.setPixelColor(j,strip.Color(255, 255, 255));
    strip.show(); 
  }
  colorWipe(strip.Color(0,0,0),50);  
}

void navigate(uint16_t dirIns){
  refresh();
  uint8_t dirDis = dirIns / 100;
  uint8_t dirPat = (dirIns%100) / 10;
  uint8_t dirPatAux = (dirIns%100) % 10; 
  uint8_t mainArr[22] = {0,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1};
  uint8_t seedArr[8] = {8,16,6,18,4,10,14,4};
  uint8_t finalArr[7];
  uint8_t a = seedArr[dirPat-1]+3;
  uint8_t x = 6;
  
  for(uint8_t i = a; i >0 ; i--){
    finalArr[x] = mainArr[i];
    x--; 
  }
  for(uint8_t j = 0; j<7; j++){
    strip.setPixelColor(finalArr[j],strip.Color(255, 255, 255));
    strip.show(); 
  }
  strip.setPixelColor(finalArr[3],strip.Color(0,255,0));
  strip.show();
  if(dirDis == 5){    
    float maxBrightness = 100;
    float speedFactor = 0.05; 
    uint8_t stepDelay = 5;   
    for (int i = 0; i < 450; i++) {    
      float intensity = maxBrightness /2.0 * (1.0 + sin(speedFactor * i));
      strip.setBrightness(intensity);      
      for (uint8_t i=0; i<7; i++) {
        strip.setPixelColor(finalArr[i], 0, 255, 0);
      }      
      strip.show();      
      delay(stepDelay);
    }
    strip.setBrightness(5);
    for(uint8_t j = 0; j<7; j++){
      strip.setPixelColor(finalArr[j],strip.Color(0, 255, 0));
      strip.show(); 
    }    
  }else{
    for(uint8_t i = 1; i < dirDis; i++){
     strip.setPixelColor(finalArr[3+i],strip.Color(0,255,0));
     strip.setPixelColor(finalArr[3-i],strip.Color(0,255,0));
     strip.show(); 
    }    
  }
  
  if(dirPatAux != 0){
    if(dirPat == 8){
      if(dirPatAux%2 != 0 ){
        strip.setPixelColor(8,strip.Color(0,0,255));
        dirPatAux--;
      }
      if(dirPatAux != 0){
        for(uint8_t i = 1; i < ((dirPatAux/2)+1); i++){
          strip.setPixelColor((8+i),strip.Color(0,0,255));
          strip.setPixelColor((8-i),strip.Color(0,0,255));
        }
      }
      strip.show();
    }
    
    if((dirPat == 1 || dirPat == 2) && dirPatAux == 1){
      strip.setPixelColor(8,strip.Color(0,0,255));
      strip.show();
    }    
  }
}
  

