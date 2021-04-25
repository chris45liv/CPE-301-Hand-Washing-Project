#include <Arduino.h>
#include <LiquidCrystal.h>

//ADC Registers
volatile unsigned char *my_ADMUX = (unsigned char *)0x7C;
volatile unsigned char *my_ADCSRB = (unsigned char *)0x7B;
volatile unsigned char *my_ADCSRA = (unsigned char *)0x7A;
volatile unsigned int *my_ADC_DATA = (unsigned int *)0x78;

//Variables for thermister
int Vo;
float R1 = 10000;
float logR2, R2, T;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
//LCD Pins
LiquidCrystal lcd(7,8,9,10,11,12);

void adc_init();
unsigned int adc_read(unsigned char);

void setup()
{
  // setup the UART
  Serial.begin(9600);

  // setup the ADC
  adc_init();
}
void loop()
{
  // get the reading from the ADC
  unsigned int adc_reading = adc_read(0);
  Vo = adc_reading;
  R2 = R1 * (1023.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
  T = T - 273.15;
  T = (T * 9.0)/ 5.0 + 32.0;
  
  Serial.print("Temperature: "); 
  Serial.print(T);
  Serial.print(" F; ");
 
  lcd.print("Temp = ");
  lcd.print(T);   
  lcd.print(" F");
  
  delay(500);            
  lcd.clear();
}
void adc_init()
{
  // set up the A register
  *my_ADCSRA |= 0b10000000; // set bit   7 to 1 to enable the ADC
  *my_ADCSRA &= 0b11011111; // clear bit 5 to 0 to disable the ADC trigger mode
  *my_ADCSRA &= 0b11110111; // clear bit 3 to 0 to disable the ADC interrupt
  *my_ADCSRA &= 0b11111000; // clear bit 2-0 to 0 to set prescaler selection to slow reading

  // set up the B register
  *my_ADCSRB &= 0b11110111; // clear bit 3 to 0 to reset the channel and gain bits
  *my_ADCSRB &= 0b11111000; // clear bit 2-0 to 0 to set free running mode

  // set up the MUX Register
  *my_ADMUX &= 0b01111111; // clear bit 7 to 0 for AVCC analog reference
  *my_ADMUX |= 0b01000000; // set bit   6 to 1 for AVCC analog reference
  *my_ADMUX &= 0b11011111; // clear bit 5 to 0 for right adjust result
  *my_ADMUX &= 0b11100000; // clear bit 4-0 to 0 to reset the channel and gain bits
}
unsigned int adc_read(unsigned char adc_channel_num)
{
  // reset the channel and gain bits
  *my_ADMUX &= 0b11100000;

  // clear the channel selection bits
  *my_ADCSRB &= 0b11110111;

  if (adc_channel_num <= 7)
  {
    // set the channel selection bits
    *my_ADMUX |= adc_channel_num;
  }
  else
  {
    *my_ADCSRB |= 0b00001000;
    *my_ADMUX |= adc_channel_num >> 3;
  }
  // set bit 6 of ADCSRA to 1 to start a conversion
  *my_ADCSRA |= 0b01000000;

  // wait for the conversion to complete
  while ((*my_ADCSRA & 0b00010000) == 0)
    ;

  // return the result in the ADC data register
  return *my_ADC_DATA;
}