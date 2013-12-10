#include <OneWire.h>

OneWire ds(8);
int temp = 0;
byte d1 = 15, d2 = 15, d3 = 15, d4 = 15;
byte digitToggle = 1;

void setup()
{
  for(int i=3;i<=6;i++) pinMode(i, OUTPUT);
  for(int i=9;i<=12;i++)
  {
    pinMode(i, OUTPUT);
    digitalWrite(i, HIGH);
  }

  // Disable the timer overflow interrupt
  TIMSK2 &= ~(1 << TOIE2);

  // Set timer2 to normal mode
  TCCR2A &= ~((1 << WGM21) | (1 << WGM20));
  TCCR2B &= ~(1 << WGM22);

  // Use internal I/O clock
  ASSR &= ~(1 << AS2);

  // Disable compare match interrupt
  TIMSK2 &= ~(1 << OCIE2A);

  // Prescalar is clock divided by 128
  TCCR2B |= (1 << CS22) | (1 << CS20);
  TCCR2B &= ~(1 << CS21);

  // Start the counting at 0
  TCNT2 = 0;

  // Enable the timer2 overflow interrupt
  TIMSK2 |= (1 << TOIE2);
}

void loop()
{
  byte i;
  byte addr[8];
  long lastRead = millis();

  if (!ds.search(addr))
  {
    ds.reset_search();
    return;
  }
  if (ds.crc8(addr, 7) != addr[7]) return;
  if (addr[0] != 0x28) return;
  
  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);

  delay(1000);
    
  ds.reset();
  ds.select(addr);
  ds.write(0xBE);

  byte data[9];
  for (i = 0; i < 9; i++) data[i] = ds.read();
  if (ds.crc8(data, 8) != data[8]) return;

  int TReading = (data[1] << 8) + data[0];

  byte SignBit = 0;
  if (TReading < 0) SignBit = 1;
  TReading = abs(TReading);

  int Tc_100 = (6 * TReading) + TReading / 4;

  setValue(Tc_100);
  
  while (millis() - lastRead < 10000) {}
}

void setValue(int n)
{
  d1 = n / 1000;
  d2 = n / 100 % 10;
  d3 = n / 10 % 10;
  d4 = n % 10;
}

// Interrupt service routine is invoked when timer2 overflows.
ISR(TIMER2_OVF_vect)
{
  TCNT2 = 0;
  switch (digitToggle)
  {
    case 1:
      PORTB |= (1 << PORTB4); // turn off digit 4
      setOutput(d1);
      PORTB &= ~(1 << PORTB1); // turn on digit 1
      digitToggle = 2;
      break;
    case 2:
      PORTB |= (1 << PORTB1); // turn off digit 1
      setOutput(d2);
      PORTB &= ~(1 << PORTB2); // turn on digit 2
      digitToggle = 3;
      break;
    case 3:
      PORTB |= (1 << PORTB2); // turn off digit 2
      setOutput(d3);
      PORTB &= ~(1 << PORTB3); // turn on digit 3
      digitToggle = 4;
      break;
    case 4:
      PORTB |= (1 << PORTB3); // turn off digit 3
      setOutput(d4);
      PORTB &= ~(1 << PORTB4); // turn on digit 4
      digitToggle = 1;
  }
}

void setOutput(byte d)
{
  PORTD &= ~0x78; // turn off digital pins 3-6
  PORTD |= (d << PORTD3); // set digital pins 3-6
}
