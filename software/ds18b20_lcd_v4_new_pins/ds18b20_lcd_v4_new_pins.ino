#include <OneWire.h> // For communicating with the DS18B20
#include <LiquidCrystal.h> // For printing to the LCD
#include <SD.h> // For communicating with the SD card

OneWire ds(9);  // DS18B20 data bus is on pin 9
LiquidCrystal lcd(8, 7, 6, 5, 4, 3); // Initialize the library with the numbers of the interface pins

// Define the temperature symbol
byte lcdTemp[8] =
{
  B11111,
  B00000,
  B01110,
  B00100,
  B00100,
  B00000,
  B11111,
  B00000
};

// Define the SD symbol
byte lcdSD[8] =
{
  B11100,
  B10010,
  B10001,
  B10001,
  B10001,
  B10001,
  B11111,
  B00000
};

void setup(void)
{
  // Initialise the serial port
  Serial.begin(9600);

  // Initialise the LCD
  lcd.begin(16, 2); // Set up the LCD's number of columns and rows
  lcd.createChar(0, lcdTemp); // Add the temperature symbol
  lcd.createChar(1, lcdSD); // Add the SD symbol

  // Initialise the SD card
  lcd.setCursor(0, 0); // Set the cursor to column 0, line 0
  lcd.print("Initialising:");
  lcd.setCursor(0, 1); // Set the cursor to column 0, line 1
  lcd.print("Checking SD...");
  lcd.setCursor(0, 1); // Set the cursor to column 0, line 1
  Serial.print("Initialising SD card...");
  if (SD.begin(16)) // Initialise the SD card
  {
    // SD initialisation succeeded
    lcd.print("SD card found!");
    Serial.println("done.");
  } else
  {
    // SD initialisation failed
    lcd.print("No SD card.   ");
    Serial.println("failed!");
  }
  delay(3000); // Wait a few seconds so the user can see the message

  lcd.clear(); // Clear LCD display
  lcd.print("Temperature:");
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

  long lastRead = millis(); // Save current time as last read
  Serial.println();
  Serial.print("Retreiving timestamp...");
  Serial.println(lastRead);

  // Send request down the 1-wire bus for the next address
  Serial.print("Searching for a DS18B20...");
  lcd.setCursor(15, 0); // Set the cursor to column 15, line 0
  lcd.write(0); // Print the temperature symbol
  if (ds.search(addr))
  {
    Serial.print("found 0x");
    for (i = 0; i < 8; i++)
    {
      if (addr[i] < 0x10)
      {
        Serial.print("0"); // If the byte value is less than 0x10, add a leading zero
      }
      Serial.print(addr[i], HEX); // Print each byte of the address
      Serial.print(" "); // Separate the bytes with a space
    }
    Serial.println();
  } else {
    Serial.println("not found!");
    ds.reset_search(); // If nothing is returned, begin a new search.
    return;
  }

  // Check the CRC
  if (ds.crc8(addr, 7) != addr[7])
  {
    Serial.println("CRC failed! Skipping this device.");
    return; // If invalid, exit loop
  }
  
  // Check the family
  if (addr[0] != 0x28)
  {
    Serial.println("Not a DS18B20! Skipping this device.");
    return; // If not a DS18B20, exit loop
  }
  
  // Commence data retrieval
  Serial.print("Retreiving data...");
  ds.reset(); // Reset the 1-wire bus
  ds.select(addr); // Select the current device
  ds.write(0x44, 1); // Start temperature conversion, using parasite power
  
  delay(1000); // Wait for 1 second (maybe 750ms is enough, maybe not)
  
  ds.reset(); // Reset the 1-wire bus
  ds.select(addr); // Select the current device
  ds.write(0xBE); // Request data from the scratchpad

  // Read the data
  Serial.print("retrieved 0x");
  for (i = 0; i < 9; i++) // We need 9 bytes
  {
    data[i] = ds.read(); // Read a byte from the scratchpad
    if (data[i] < 0x10)
    {
      Serial.print("0"); // If the byte value is less than 0x10, add a leading zero
    }
    Serial.print(data[i], HEX); // Print each byte of the data
    Serial.print(" "); // Separate the bytes with a space
  }
  Serial.println();
  
  // Check the CRC
  if (ds.crc8(data, 8) != data[8])
  {
    Serial.println("CRC failed! Skipping this device.");
    return; // If invalid, exit loop
  }

  // Convert the hex data into a meaningful temperature reading
  Serial.print("Converting into temperature...");
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

  // Construct the temperature string
  String temperature = "";
  if (SignBit)
  {
    temperature += "-"; // If temperature is less than zero, add the negative sign
  }
  temperature += Whole; // Add the whole part
  temperature += "."; // Add the decimal point
  if (Fract < 10)
  {
    temperature += "0"; // If the fraction part is less than 10, add a leading zero
  }
  temperature += Fract; // Add the fraction part

  // Print the temperature
  lcd.setCursor(15, 0); // Set the cursor to column 15, line 0
  lcd.print(" "); // Clear activity symbol
  lcd.setCursor(0, 1); // Set the cursor to column 0, line 1
  lcd.print(temperature + char(223) + "C   ");
  Serial.println(temperature + byte(186) + "C");

  // Log the temperature
  lcd.setCursor(15, 0); // Set the cursor to column 15, line 0
  lcd.write(1); // Print the SD symbol
  Serial.print("Opening datalog.txt...");
  File dataFile = SD.open("datalog.txt", FILE_WRITE); // Open the file
  if (dataFile)
  {
    Serial.println("done.");
    Serial.print("Writing to datalog.txt...");
    dataFile.print("0x");
    for (i = 0; i < 8; i++)
    {
      if (addr[i] < 0x10)
      {
        dataFile.print("0"); // If the byte value is less than 0x10, add a leading zero
      }
      dataFile.print(addr[i], HEX); // Add each byte of the address
    }
    dataFile.print(",");
    dataFile.print(lastRead);
    dataFile.print(",");
    dataFile.println(temperature);
    dataFile.close();
    Serial.println("done.");
  } else
  {
    Serial.println("failed! Skipping log.");
  }
  lcd.setCursor(15, 0); // Set the cursor to column 15, line 0
  lcd.print(" "); // Clear activity symbol
  
  ds.reset_search(); // Begin a new search (don't care if there are additional sensors at the moment)

  while (millis() - lastRead < 10000) {} // Leave 10 seconds between readings (to avoid self-heating)
}

