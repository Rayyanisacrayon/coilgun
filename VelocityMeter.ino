/*
 CoilGun Velocity Meter
 a) Measures and prints voltage until it is stable. Divider sized for 500V
 b) Measure pulse width as projectlie passes in front of CNY70
 c) Converts pulse duration to velocity based on the length of projectile and prints it
  
  The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)

 */

#include <avr/io.h>
#include <avr/interrupt.h>

// include the library code:
#include <LiquidCrystal.h>

//volatile uint16_t cnt = 0;
// unsigned int pulse_counts = 0;
const byte INPUTPIN = 6;
const byte FIREIN = A0; 
const byte FIREOUT = 13; 


// Controls
#define noPulseInterrupts 1
#define usePulseInLong    0

// Must have interrupts for pulseInLong
#if usePulseInLong
  #define noPulseInterrupts 0
#endif

const uint32_t PWM_TIMEOUT=5000000UL;

int analogInput=A5;


// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);


void setup()
{
  Serial.begin(9600);
  Serial.println("Velocity Meter 1.0");
  pinMode(INPUTPIN, INPUT);
  pinMode(analogInput, INPUT);
  pinMode(FIREIN, INPUT);
  pinMode(FIREOUT, OUTPUT);


  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.println("Velocity Meter");
}



// the loop function runs over and over again forever
void loop() {
  unsigned long val;

start:
  // Read voltage until  it  is  stable
  lcd.clear();
  lcd.print("V: ");
  float v;
  uint32_t i = 0;
  do {
      v = readVoltage();
      lcd.setCursor(3, 0);
      lcd.println(v);
      delay (100);
  } while (i++ < 200 && abs(readVoltage() - v) >= 1);
 
  Serial.print("Time: ");
  Serial.print(millis());
  Serial.print(" Voltage: ");
  Serial.println(v);
  Serial.println(" Armed");
  Serial.flush(); // Needed since serial requires interrupts to operate.

  lcd.setCursor(0, 1);
  lcd.print("Armed...");

  // Read voltage and  read fire buttom
  i = 0;
  do {
    if (digitalRead(FIREIN))
      break;
        
    v = readVoltage();
    lcd.setCursor(3, 0);
    lcd.println(v);
    delay (100);
  } while (i++ < 200 && abs(readVoltage() - v) <= 1);

  // Check  if loop terminated  b/c of FIRE button or timeout
  if  (!digitalRead(FIREIN))
     goto start;
      
  lcd.setCursor(0, 1);
  lcd.print("Fire...");
  delay(100);
  digitalWrite(FIREOUT, HIGH);
     
  if(noPulseInterrupts) 
      noInterrupts();
  if (usePulseInLong)
      val = pulseInLong(INPUTPIN,HIGH,PWM_TIMEOUT);
  else
      val = pulseIn(INPUTPIN,HIGH,PWM_TIMEOUT);
  interrupts();
  digitalWrite(FIREOUT, LOW);

  lcd.setCursor(0, 1);
  lcd.print("Fired. Analyzing");
  delay(1000);
  
  //val = 200;
  if (val) {
    
     float velocity44mm = 44*1000./val;
     float velocity38mm = 38*1000./val;
      
     Serial.print("Time high (us): ");
     Serial.print(val);

     Serial.print(" Velocity (m/s) 44mm/38mm: ");
     Serial.print(velocity44mm);
     Serial.print("/");
     Serial.println(velocity38mm);

     lcd.clear();
     lcd.print("Vm/s-44mm: ");
     lcd.print(velocity44mm);
     lcd.setCursor(0, 1);
     lcd.print("Vm/s-38mm: ");
     lcd.println(velocity38mm);
     delay(5000);
  } else
  {
    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    lcd.setCursor(0, 1);
    lcd.print("No reading    ");
    Serial.println("No reading");
    delay(2000);
  }

}



float readVoltage() {
  
  float vout;
  float vin;
  float R1 = 1000000.0;
  float R2 = 10000.0;
  float cal = 1.0;
  
  int value;
  value = analogRead(analogInput);
  if (value == 1) // Under resolution margin
    value = 0;
  // Serial.print("v: ");
  //  Serial.println(value);
  vout = (value * 5) / 1023.0;
  vin = cal* vout / (R2 / (R1 + R2));
  

  return vin;
}

//void loop()
//{
//  // initialize Timer1
//  cli();
//  // reset counters
//  cnt = 0;
//  TCNT1 = 0;
//  // reset registers
//  TCCR1A = 0;
//  TCCR1B = 0;
//
//  pulse_counts++;
//  // wait for HIGH
//  while ((PIND & B00001000) == B00000000);
//  // Set CS10 bit so timer runs at clock speed: 16 MHz
//  TCCR1B |= (1 << CS10);
//  // enable Timer1 overflow interrupt:
//  TIMSK1 = (1 << TOIE1);
//
//  // enable global interrupts:
////  sei();
//
//  // keep counting until LOW
//  while ((PIND & B00001000) == B00001000);
//  // stop IRQ's
//
//
//  TIMSK1 = 0;
//  TCCR1B = 0;
//// cli();
//
//    // Read the counters and convert to long
//  uint16_t x = TCNT1;  // work copy
//  uint32_t total = (uint32_t)cnt  + (uint32_t)x;
//  float usec = (1.0 * total) / 16;
//
//  // Display values
//  Serial.print(x, DEC);
//  Serial.print(" \t ");
//  Serial.print(usec, 2);
//  Serial.print(" \t ");
//  Serial.print("Count = ");
//  Serial.println(pulse_counts);
//
//  // Wait a while
////  delay(1000);
//}
//
//// count the overflows in IRQ
//ISR(TIMER1_OVF_vect)
//{
//  cnt++;
//}
