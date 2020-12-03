// MicroCore ATtiny13A  https://github.com/MCUdude/MicroCore#how-to-install
// ATtiny13 9.6Mhz int. 

const byte PIN_LDR = A1 ; // вход фоторезистор //D2/A1
const byte PIN_LED = 0 ; // выход подсветка
const byte PIN_T = 4 ; // вход тахометра
const byte PIN_PWMV = 1 ; // выход напряжения


//MODE1
int LDR  = 0;
// float LED = 0 ;
byte LED = 0 ;
const byte LimitL = 170 ; // test max 190 // limit 160

//MODE2
//unsigned int min = 0;

byte RPM;
//unsigned 
//long lastflash;
//unsigned long duration;



//

//#define Tone_PIN 12 // выход частоты– см. в тексте 
//#define IN_PIN 4 //вход обнаружения частоты
volatile unsigned long ttime = 0;        //Время срабатывания датчика
volatile unsigned long time_old = 0;        //Предыдущее время
volatile uint8_t flag=0;






void setup() {
//MODE1
  analogReference(DEFAULT);
  
  pinMode(PIN_LED, OUTPUT);


//MODE2
  pinMode(PIN_PWMV, OUTPUT);
  
 //   pinMode(PIN_T, INPUT); // вход тахо
  //  digitalWrite(PIN_T, HIGH);
    
 //attachInterrupt(PIN_T,impuls,RISING);

 // setitr();

  
}

//void impuls(){
//   if(flag!=1) ttime =micros()-time_old;
//    time_old = micros();
//}

void loop() {
  
//MODE1
LDR  = analogRead(PIN_LDR)  ; //чтение фоторезистора // A1/D2
LED = ( LDR - 356 ) /3  ;  //2.8
if (LED <= 1) {  LED = 1 ; }

if (LED < LimitL) {
analogWrite(PIN_LED,LED );  // D0

}
else {
  analogWrite(PIN_LED,1);  // D0 //  //digitalWrite(0, LOW) ;
//LED = 0 ;
}

//MODE2

//min = min+128 ;
//if (min = 65535 ){   min = 0; }
//min = 45535 ;




//duration = pulseInLong(PIN_T, LOW, 3000 ) ;
 
   //  flag=1; //чтобы ttime не изменилось в процессе вывода
      
     //  if (ttime!=0) {//на случай отсутствия частоты
      // RPM = 1000000/float(ttime); // Вычисляем частоту сигнала в Гц
    //     RPM = float(ttime); // Вычисляем частоту сигнала в Гц

// analogWrite(PIN_PWMV, ttime/16777216 );
       //  }
        // else {
          // analogWrite(PIN_PWMV,1);
        // }
       
  //    flag=0;

 //analogWrite(PIN_PWMV, 255 );
// analogWrite(PIN_PWMV, ttime/16777216 );
 //analogWrite(PIN_PWMV, RPM );


 

/*
detachInterrupt(4);   
time = ( millis() - time_last);
//rpm=(rev/time)*60000;
rpm=(rev/time);
time_last = millis();
rev=0;

setitr();

analogWrite(1, rpm );
*/

  //delay (50);
}
