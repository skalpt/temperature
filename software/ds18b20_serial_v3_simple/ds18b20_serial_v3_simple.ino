#include <OneWire.h>

OneWire ds(10);  // DS18B20 data bus is on pin 10

void setup(void)
{
  Serial.begin(9600); // Initiate the serial port
}

void loop(void)
{
  int i; // For counting interations
  byte addr[8]; // For storing the address (ROM code)
  byte data[9]; // For storing the data
  int TReading; // For storing the raw temperature data
  int SignBit; // For storing the polarity of the temperature
  int Tc_100; // For storing the temperature
  int Whole; // For storing the whole part of the temperature
  int Fract; // For storing the fraction part of the temperature
  
  // Send request down the 1-wire bus for the next address
  if (!ds.search(addr)) // If nothing is returned...
  {
      Serial.print("\n"); // ...print a message...
      ds.reset_search(); // ...and begin a new search.
      return;
  }
  
  // Check the family
  if (addr[0] != 0x28)
  {
    Serial.print("Found non-DS18B20 device on bus!\n");
    return;
  }
  
  // Check the CRC
  if (ds.crc8(addr, 7) != addr[7])
  {
    Serial.print("Invalid address CRC!\n");
    return;
  }
  
  // Commence data retrieval
  ds.reset(); // Reset the 1-wire bus
  ds.select(addr); // Select the current device
  ds.write(0x44, 1); // Start temperature conversion, using parasite power
  
  digitalWrite(13, HIGH); // Set the LED on pin 13 to on (to signal temperature conversion in progress)
  delay(1000); // Wait for 1 second (maybe 750ms is enough, maybe not)
  digitalWrite(13, LOW); // Set the LED on pin 13 to off (to signal temperature conversion complete)
  
  ds.reset(); // Reset the 1-wire bus
  ds.select(addr); // Select the current device
  ds.write(0xBE); // Request data from the scratchpad

  // Read thr data
  for (i = 0; i < 9; i++) // We need 9 bytes
  {
    data[i] = ds.read(); // Read a byte from the scratchpad
  }

  // Check the CRC
  if (ds.crc8(data, 8) != data[8])
  {
    Serial.print("Invalid data CRC!\n");
    return;
  }

  // Convert the hex data into a meaningful temperature reading
  TReading = (data[1] << 8) + data[0]; // Take the first two bytes of the data
  SignBit = TReading & 0x8000;  // Test the most significant bit
  if (SignBit) // Is it a negative number?
  {
    TReading = (TReading ^ 0xffff) + 1; // Convert from two's complement format
  }
  Tc_100 = (6 * TReading) + TReading / 4; // Multiply by (100 * 0.0625) or 6.25

  // Separate off the whole and fractional portions
  Whole = Tc_100 / 100;
  Fract = Tc_100 % 100;

  // Print the temperature
  if (SignBit) // If its negative...
  {
     Serial.print("-"); // ...add the negative sign
  }
  Serial.print(Whole); // Print the whole bit
  Serial.print("."); // Print the decimal point
  if (Fract < 10)
  {
     Serial.print("0");
  }
  Serial.print(Fract); // Print the fraction bit
  Serial.print(byte(186
  )); // Print the degrees sign
  Serial.print("C"); // Print the celcuius symbol
  Serial.print("  ");
}

