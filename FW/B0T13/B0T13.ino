// MicroCore ATtiny13A  https://github.com/MCUdude/MicroCore#how-to-install
// ATtiny13 9.6Mhz int. 
int LDR  = 0;
float LED = 0 ;
int LimitL = 300 ;

void setup() {
  pinMode(0, OUTPUT);
}

void loop() {

LDR  = analogRead(A1)  ; //чтение фоторезистора // A1/D2
LED = ( LDR - 356 ) /2.8  ; 
if (LED <= 1) {  LED = 1 ; }

if (LED < LimitL) {
analogWrite(0,LED );  // D0
}
else {
  digitalWrite(0, LOW) ;
LED = 0 ;
}
 // delay (10);
}
