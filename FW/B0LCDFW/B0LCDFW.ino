//Arduino AVR Boards: Arduino Pro or Pro Mini | ATmega328p (5V, 16MHz)

#include <U8g2lib.h> //U8g2 2.27.6

#include <Wire.h> //for RTClib.h
//#include <Time.h>
#include "lib/RTClib.h" //RTClib 1.2.1 https://github.com/adafruit/RTClib
#include "lib/RTClib.cpp"

#include "fonts/Fonts.h" 
#include "fonts/u8g_font_04b_03b.h" //Шрифт Мелкие цифры Батарея V, ..
#include "fonts/u8g2_font_ncenB18m.h"  //Шрифт больших цифр скорости расхода
#include "fonts/u8g2_font_7x13B.h"  //Шрифт Батарея 200%

#define PIN_VMETER A7 //вход вольтметра
#define PIN_AMETER A6 //вход амперметра
//#define PIN_TAHO A0 //вход тахометра // int0 d2
#define PIN_F 8 //вход обнаружения частоты ICP // mega328 pin12
#define KM_SYM 5 // количество символов спидометра, 4 или 5







// инициализация экрана
U8G2_ST7920_128X64_F_8080 u8g2(U8G2_R0, 9,11,7,6,5,4,3,10, 16,U8X8_PIN_NONE,17,15); 

// инициализация часов
RTC_DS1307 rtc; // "rtc" используется в начале функций, которые прилагаются с библиотекой
//int displayrtc = 1234 ;



byte  lup = 0; // циклы отрисовки экрана
 

//float vmax = 54.8500 ; // максимальное напряжение измерения = 54.85
//const float vmax = 54.85;
const float vmax = 548.5;
const float vminus = 28;

// Напряжение
#define V_ADCE 15+1 // смещение ОУ АЦП вольтметра, ADC
#define V_MIN 28 // минимальное напряжение вольтметра, V
#define V_MAX 51.6// максимальное напряжение вольтметра, V
int V_ADC ; // ADC напряжения батареи
float Volt_Bat ; // напряжение на батарее
static char Volt_BatTxt[5]; // вывод напряжения на экран

// Ток
const float A_MAX = 21.9648 ; // 15.36 // максимальный ток амперметра, A
#define A_ADCE 0 //11 // смещение ОУ АЦП амперметра, ADC
int A_ADC ; // ADC ток от батареи
float Amper_Bat ; // потребляемый ток от батареи
static char Amper_BatTxt[5]; // вывод тока на экран

// Мощность
float Power_Bat ; // потбребляемая мощность

// Скорость
// Оценка методов измерения низких частот на Arduino. Способ 2. задействуем Timer1 https://habr.com/ru/post/373213/
volatile byte time2 = 0; //старший разряд времени, два младших - регистры таймера
volatile unsigned long ttime = 0;        //Время срабатывания датчика
volatile unsigned long time_old = 0;        //Предыдущее время
volatile uint8_t flag=0;
float SpeedKmH ;
static char SpeedKmHtxt[KM_SYM+1]; //строка скорости спидометра 5 = 9.999 99.99 // 69.9

/* ---------------------------------------------------------------------------------------------- */


/* ---------------------------------------------------------------------------------------------- */

void setup(void) {
  u8g2.begin();

pinMode(PIN_VMETER, INPUT);
pinMode(PIN_AMETER, INPUT);

analogReference(EXTERNAL); //внешний ИОН 4.00v. TL431 Rs=150 R2=3k R1=2k2+560/2 Raref=5k1 
// DEFAULT: стандартное опорное напряжение 5 В (на платформах с напряжением питания 5 В) или 3.3 В (на платформах с напряжением питания 3.3 В)
// INTERNAL: встроенное опорное напряжение 1.1 В на микроконтроллерах ATmega168 и ATmega328, и 2.56 В на ATmega8.
// EXTERNAL: внешний источник опорного напряжения, подключенный к выводу AREF

//1307
//Запуск секундного выхода часов
// Wire.beginTransmission(0x68); Wire.write(0x7); Wire.write(0x10); Wire.endTransmission(); 
//NOTE SquareWave= 32.768khz 8.192khz 4.096khz 1hz
//rtc.writeSqwPinMode( SquareWave1HZ ); // OFF, ON, SquareWave1HZ, SquareWave4kHz, SquareWave8kHz, SquareWave32kHz
rtc.writeSqwPinMode( SquareWave32kHz ); // OFF, ON, SquareWave1HZ, SquareWave4kHz, SquareWave8kHz, SquareWave32kHz

// rtc.adjust(DateTime(2021, 5, 7,      3, 53,1 )); // задаём год/ месяц/ дата/ часы/ минуты/ секунды
DateTime now = rtc.now();
if ( now.year() < 2005 ) { rtc.adjust(DateTime(2019, 12, 31, 23, 59,1 )); } // set YYYY,MM,dd,hh,mm
// https://www.radiokot.ru/forum/viewtopic.php?f=2&t=174215
// rtc.year()


// Скорость
// http://electronics-lab.ru/blog/4012.html


  pinMode(PIN_F, INPUT); //вывод обнаружения частоты на вход
  TCCR1A = 0; // установка регистров таймера1
  TCCR1B =1<<ICES1 | 1<<CS11 ; // Устанавливаем захват по фронту и делитель 1/8 к тактовой частоте 16МГц
  //TCCR1B =1<<ICES1 | 1<<CS11  | 1<<CS10; // Устанавливаем захват по фронту и делитель 1/64 к тактовой частоте 16МГц
  TIMSK1 = 1<<TOIE1 | 1<<ICIE1; //разрешаем прерывание по переполнению и захвата Timer1
  TCNT1 = 0; //Обнуляем счетные регистры

  
}// End void setup

/* ---------------------------------------------------------------------------------------------- */

ISR( TIMER1_OVF_vect ) //прерывание счета времени
{
  time2++; //увеличиваем на единицу старший разряд времени
}

ISR(TIMER1_CAPT_vect) //прерывание захвата
{ 
  TCNT1 = 0; // очищаем Timer1 Count Register
  uint16_t time01 = ICR1; //сохраняем состояние счетчика таймера, разряды 0 и 1 
  if(flag!=1) ttime = ((unsigned long)time2 << 16) +time01; //собираем
  time2=0;
} 
/* ---------------------------------------------------------------------------------------------- */



/* ---------------------------------------------------------------------------------------------- */

void draw(void) {

//region скорость/расход

//29" 2.33m  =1.382789317507418
//20" 1.685m =1
//1024adc=60km/h or; 1024adc=51.2km/h , 512adc=25.6km/h , 500adc=25km/h

// SpeedKmH = (f*3600*1.685)/1000 // 1i=6066mh=6.066kmh
// f= ? adc

// Umax=3.4v = 59.392kmh = 0.058*1024 // adc = kmh 426 = 24.96 427 = 25.01
// 6.066/0.058=1045.86v


//u8g2.setFont(  u8g2_font_18d  ); // u8g2_font_ncenB18_te
u8g2.setFont(  u8g2_font_ncenB18m  );

//Измерение скорости
flag=1; //чтобы ttime не изменилось в процессе вывода
  if (ttime!=0) {//на случай отсутствия частоты
    //        6087230959
    //SpeedKmH =6087230959/ttime  ;// Вычисляем частоту сигнала в Гц // x=6087230959
    SpeedKmH =6087230959/ttime  ;// Вычисляем частоту сигнала в Гц // x=6087230959
  }
flag=0; 
//dtostrf(SpeedKmH/1000,KM_SYM,3,SpeedKmHtxt); 
dtostrf(SpeedKmH/1000,KM_SYM,1,SpeedKmHtxt); 
u8g2.setCursor(26, 18); u8g2.print(SpeedKmHtxt);  u8g2.print("/");
//end Измерение скорости


//u8g2.setFont(u8g_font_04b_03b);u8g2.setCursor(28, 24); u8g2.print(SpeedADC); u8g2.print("  "); 
//u8g2.setCursor(22, 30); u8g2.print(debugADC); u8g2.print("  "); 
//u8g2.setCursor(28, 36); u8g2.print(SpeedADCstep); u8g2.print("  "); 




u8g2.setFont(  u8g2_font_ncenB18m  );

  float ran1 = random(1,30);
  ran1=ran1*100; ran1= ran1/1000;
  //ran1=tab(ran1/10),1
    // u8g2.setCursor(26, 18); u8g2.print("v:");u8g2.print(ran1); //km/h
       //   u8g2.setCursor(40, 18);u8g2.print(ran1); //km/h
          
         //u8g2.drawStr(40,18,ran1); //km/h
    
 // u8g2.setCursor(45, 40); u8g2.print("f");
 // u8g2.setCursor(45+12, 40); u8g2.print("35"); //w*h/km
    u8g2.setCursor(45, 40); u8g2.print("35"); //w*h/km
  
 // u8g2.drawHLine(74, 9, 11);
  //u8g2.drawHLine(73, 31, 13);  u8g2.drawHLine(79, 26, 1);
  
 // u8g2.setFont(u8g2_font_5x7_tr); 


//u8g2.setFont(u8g2_font_6x10_tf); 
//u8g2.setFont(u8g2_font_siji_t_6x10); 

//u8g2.setCursor(74, 8); u8g2.print("km"); u8g2.setCursor(77, 18); u8g2.print("h");  
//u8g2.setCursor(73, 30); u8g2.print("W"); u8g2.setCursor(81, 30); u8g2.print("h"); u8g2.setCursor(74, 40); u8g2.print("km");
    //u8g2.setCursor(74, 8); u8g2.print("Lx"); u8g2.setCursor(77, 18); u8g2.print("t");  
  //u8g2.setCursor(73, 30); u8g2.print("W"); u8g2.setCursor(81, 30); u8g2.print("h"); u8g2.setCursor(74, 40); u8g2.print("Km");
    //u8g2.setCursor(73, 30); u8g2.print("P"); u8g2.setCursor(81, 30); u8g2.print("t"); u8g2.setCursor(74, 40); u8g2.print("Lx");
//end region скорость/расход


//region батарея

// нижняя, верхняя, левая
u8g2.drawHLine(0, 63,42); u8g2.drawHLine(0, 42, 42); u8g2.drawVLine(0, 42, 21); 
u8g2.drawVLine(46, 49, 8); // плюс, и верхний, и нижний
u8g2.drawHLine(46-5, 49, 5); u8g2.drawHLine(46-5, 56, 5);
u8g2.drawVLine(41, 43, 6); u8g2.drawVLine(41, 43+14, 6); 


// напряжение

u8g2.setFont(u8g_font_04b_03b);


dtostrf(Volt_Bat,3,1,Volt_BatTxt); 
u8g2.setCursor(3, 61);//  u8g2.print(Volt_BatTxt); u8g2.print("V");
u8g2.print(V_ADC); u8g2.print(" ") ;//отладка




u8g2.setFont(u8g2_font_7x13B); // u8g2.setCursor(8, 54); 
//u8g2.print("200");u8g2.print("%"); //! 15002 48%

/*

     if ( Volt_Bat > 953 ) { u8g2.print("99.9"); }
else if ( Volt_Bat < 47.5 ) { u8g2.print("95"); }
else if ( Volt_Bat < 47.3 ) { u8g2.print("85"); }
else if ( Volt_Bat < 47.1 ) { u8g2.print("60"); }
else if ( Volt_Bat < 47 ) { u8g2.print("55"); }
else if ( Volt_Bat < 46.78 ) { u8g2.print("50"); }
else if ( Volt_Bat < 46.7 ) { u8g2.print("45"); }
else if ( Volt_Bat < 46.6 ) { u8g2.print("40"); }
else if ( Volt_Bat < 46.5 ) { u8g2.print("20"); }
else if ( Volt_Bat < 46.3 ) { u8g2.print("15"); }
else if ( Volt_Bat < 45.9 ) { u8g2.print("15"); }
else if ( Volt_Bat < 45.5 ) { u8g2.print("5"); }
else if ( Volt_Bat < 45 ) { u8g2.print("1"); }
else if ( Volt_Bat = 0 ) { u8g2.print("0"); }
else { u8g2.print("0"); }
u8g2.print("%");

*/
u8g2.setCursor(4, 54); u8g2.print(Volt_BatTxt);  u8g2.print("V"); 
//u8g2.setCursor(4, 54);  u8g2.print("51.1V"); 

// vout > 51.1

//u8g2.setFont(u8g2_font_7x13B); u8g2.setCursor(8, 54); u8g2.print("200");u8g2.print("%"); //! 15002 48%
//u8g2.setFont(u8g2_font_pxplusibmcga_8u ); u8g2.setCursor(8, 54); u8g2.print("200");u8g2.print("%"); //15536 50%
//u8g2.setFont(u8g2_font_7x13B_tr); u8g2.setCursor(8, 54); u8g2.print("200");u8g2.print("%"); //! 15808 51%
//u8g2.setFont(u8g2_font_7x13B_mr); u8g2.setCursor(8, 54); u8g2.print("200");u8g2.print("%"); //! 16130 52%
//u8g2.setFont(u8g2_font_7x13B_tf); u8g2.setCursor(8, 54); u8g2.print("200");u8g2.print("%"); //! 16986 55%
//u8g2.setFont(u8g2_font_7x13B_mf); u8g2.setCursor(8, 54); u8g2.print("200");u8g2.print("%"); //! 17690 57%



// напряжение и температура батареи
// u8g2_font_04b_03b https://github.com/olikraus/u8g2lib/wiki/fontgroup04
// u8g2_font_orgv01r https://github.com/olikraus/u8g2lib/wiki/fontgrouporgdot
// - u8g2_font_baby v ° https://github.com/olikraus/u8g2lib/wiki/fontgroupfontstruct
//u8g2.setFont(u8g2_font_04b_03b); 
//u8g2.setCursor(3, 61); u8g2.print("52.9");u8g2.print("V");

//!
//u8g2.setFont(u8g2_font_blipfest_07_tr); 
//u8g2.setFont(u8g2_font_trixel_square_tr  ); 
//u8g2.setFont(u8g2_font_trixel_square_tf ); 
//u8g2.setFont(u8g_font_04b_03b  );



//u8g2.setCursor(25, 61); u8g2.print("150");u8g2.print("C°");


//if (lup == 1) {


//u8g2.setCursor(25, 61); u8g2.print(ran2);u8g2.print("C°");
//}



//u8g2.setCursor(0, 35); u8g2.print("5W.h/Km");


//region токи

// Амперметр



u8g2.setFont(u8g2_font_6x12_tr); 
//u8g2.setCursor(58, 52); u8g2.print("20.4");u8g2.print("A");

//dtostrf(Amper_Bat,4,1,Amper_BatTxt); 
u8g2.setCursor(50, 52); u8g2.print("I"); u8g2.print(Amper_Bat);u8g2.print("A");
//u8g2.setCursor(58, 52); 




//u8g2.setCursor(58, 61); u8g2.print("3.25");u8g2.print("A");
u8g2.setCursor(98, 10); u8g2.print(A_ADC);u8g2.print("a"); // debug


// Мощность

Power_Bat = Volt_Bat*Amper_Bat ;

u8g2.setCursor(50, 61); u8g2.print("P"); //ток электроники
u8g2.print(Power_Bat); u8g2.print("W");


//end region токи




//  u8g2.setFont(u8g2_font_10x20_78_79);
  u8g2.setCursor(0, 35); 
 // u8g2.print("IiJjKkLlMm");
  //u8g2.print("1234567890");
 // u8g2.print("лмнопА");

//  u8g2.setFont(u8g2_font_10x20_78_79);
  // ! u8g2.setCursor(0, 50); 
  //u8g2.print("QqRrSsTtUuVvWw"); 
  //u8g2.print("абвгдеёжзик"); 



// RTC 1307
   DateTime now = rtc.now();

//u8g2.setFont(u8g2_font_chroma48medium8_8u);
u8g2.setFont(u8g_font_04b_03b); //u8g2_font_blipfest_07_tr
//u8g2.setCursor(87, 64); u8g2.print(now.year()); u8g2.print("."); u8g2.print(now.month()); u8g2.print("."); u8g2.print(now.day()); // обычная версия

int tmpo ;
byte dataShift = 0 ;
 
//tmpo = (now.second()+1)/2 ;
tmpo =  now.day() ;
if ( tmpo == 1 ) { u8g2.setCursor(126, 64); dataShift = dataShift+7; } 
else if ( tmpo == 11 ) { u8g2.setCursor(123, 64);  dataShift = dataShift+4; }
else if ( tmpo < 10 ) { u8g2.setCursor(124, 64);  dataShift = dataShift+5;  }
//else if ( tmpo < 10 or tmpo == 0 ) { u8g2.setCursor(124, 64);  dataShift = dataShift+5;  } // extend
else if ( tmpo == 10 or tmpo == 12 or tmpo == 13 or tmpo == 14 or tmpo == 15 or tmpo == 16 or tmpo == 17 or tmpo == 18 or tmpo == 19 or tmpo == 21 or tmpo == 31) 
  { u8g2.setCursor(121, 64);  dataShift = dataShift+2; }
else if ( tmpo == 20 or tmpo > 21) { u8g2.setCursor(119, 64) ; dataShift = dataShift+0; }
u8g2.setCursor(119+dataShift, 64); u8g2.print(tmpo) ;

//tmpo = now.second()/5 ;
tmpo = now.month() ;
if ( tmpo == 1 ) {  dataShift = dataShift+5; } // for 1
else if ( tmpo == 11 ) { dataShift = dataShift+2; } // for 11
else if ( tmpo == 10 or  tmpo == 12 ) {  }//dataShift = dataShift+0;  } // for 10,12
 // else if ( tmpo == 0 ) { dataShift = dataShift+3; } // extend for 0
else { dataShift = dataShift+3; } // for 2,3,4,5,6,7,8,9,etc
u8g2.setCursor(109+dataShift, 64) ; u8g2.print(tmpo) ; u8g2.print(".");

//tmpo = 2000+now.second() ;
tmpo =  now.year()  ;
if ( tmpo == 2021 or tmpo == 2031 or tmpo == 2041 or tmpo == 2051 or tmpo == 2061 or tmpo == 2071 or tmpo == 2081) { dataShift = dataShift+2; } 
//if ( tmpo == 2001 or tmpo == 2010 or tmpo == 2012 or tmpo == 2013 or tmpo == 2014 or tmpo == 2015 or tmpo == 2016 or tmpo == 2017 or tmpo == 2018 or tmpo == 2019 or tmpo == 2021 or tmpo == 2031 or tmpo == 2041 or tmpo == 2051 or tmpo == 2061 or tmpo == 2071 or tmpo == 2081) { dataShift = dataShift+2; } 
u8g2.setCursor(87+dataShift, 64) ; u8g2.print(tmpo) ; u8g2.print(".");


//--------------------------
u8g2.setFont(u8g2_font_7d);
u8g2.setCursor(95, 56); u8g2.print(now.hour()); u8g2.print(":"); //95 50

//int min = now.minute() ;
//int sec = now.second() ;
//if ( sec < 10 ) { u8g2.print("0"); } u8g2.print(sec); 
if ( now.minute() < 10 ) { u8g2.print("0"); } u8g2.print(now.minute()); u8g2.print(":");
if ( now.second() < 10 ) { u8g2.print("0"); } u8g2.print(now.second()); 

// батарейки СЦ установлены 2020.10.18
// End RTC 1307


} // End void draw



/* ---------------------------------------------------------------------------------------------- */

void loop(void) {
// picture loop
  u8g2.firstPage();  
  do {
    draw();
// timertc();
  } while( u8g2.nextPage() );
  
// rebuild the picture after some delay

lup = lup+1 ;
if (lup > 10) lup = 0;

// Вольтметр
V_ADC = analogRead(PIN_VMETER);
//Volt_Bat=1000* ( (V_ADC-V_ADCE)*(V_MAX-V_MIN)/(1024-V_ADCE)+V_MIN ) ;
Volt_Bat=(V_ADC-V_ADCE)*(V_MAX-V_MIN)/(1024-V_ADCE)+V_MIN ;
if (V_ADC <= V_ADCE) { Volt_Bat = 0 ;}

// Амперметр
A_ADC = analogRead(PIN_AMETER);
Amper_Bat = A_MAX/(1024-A_ADCE)*(A_ADC-A_ADCE)  ;
//Amper_Bat = A_MAX/1024*A_ADC ; 
if (A_ADC <= A_ADCE) { Amper_Bat = 0 ;}
  
  delay(40 ); //int0 250//40
} //End void loop

/* ---------------------------------------------------------------------------------------------- */
