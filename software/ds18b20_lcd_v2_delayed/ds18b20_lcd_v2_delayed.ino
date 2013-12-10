#include <OneWire.h>
#include <LiquidCrystal.h>

OneWire ds(10);  // DS18B20 data bus is on pin 10
LiquidCrystal lcd(12, 11, 5, 4, 3, 2); // Initialize the library with the numbers of the interface pins

void setup(void)
{
  lcd.begin(16, 2); // Set up the LCD's number of columns and rows
  lcd.print("Temperature:"); // Print a message to the LCD
  lcd.setCursor(0, 1); // Set the cursor to column 0, line 1
  lcd.print("Reading...");
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
  if (!ds.search(addr))
  {
      ds.reset_search(); // If nothing is returned, begin a new search.
      return;
  }
  
  // Check the family
  if (addr[0] != 0x28)
  {
    return; // If not a DS18B20, exit loop
  }
  
  // Check the CRC
  if (ds.crc8(addr, 7) != addr[7])
  {
    return; // If invalid, exit loop
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
    return; // If invalid, exit loop
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

  lcd.setCursor(0, 1); // Set the cursor to column 0, line 1

  // Print the temperature
  if (SignBit)
  {
     lcd.print("-"); // If temperature is less than zero, add the negative sign
  }
  lcd.print(Whole); // Print the whole bit
  lcd.print("."); // Print the decimal point
  if (Fract < 10)
  {
    lcd.print("0");
  }
  lcd.print(Fract); // Print the fraction bit
  lcd.print((char)223); // Print the degrees sign
  lcd.print("C"); // Print the celcuius symbol
  lcd.print("   "); // This is to clear the existing text

  delay(9000); // Leave 10 seconds between readings (to avoid self-heating)
  ds.reset_search(); // Begin a new search (don't care about additional sensors for LCD)
}

