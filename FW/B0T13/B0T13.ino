// MicroCore ATtiny13A  https://github.com/MCUdude/MicroCore#how-to-install
// ATtiny13 9.6Mhz int. 
int LDR  = 0;
// float LED = 0 ;
byte LED = 0 ;
const byte LimitL = 160 ; // test max 190 // limit 160

void setup() {
  pinMode(0, OUTPUT);
}

void loop() {

LDR  = analogRead(A1)  ; //чтение фоторезистора // A1/D2
LED = ( LDR - 356 ) /3  ;  //2.8
if (LED <= 1) {  LED = 1 ; }

if (LED < LimitL) {
analogWrite(0,LED );  // D0
}
else {
  analogWrite(0,1);  // D0 //  //digitalWrite(0, LOW) ;
LED = 0 ;
}
 // delay (10);
}
