const int numReadings = 100;    // the number of samples to average

int readings[numReadings];      // the readings from the analog input
int index = 0;                  // the index of the current reading
int total = 0;                  // the running total
float average = 0;              // the average
int fullset = 0;                // flag for a full set being available

int inputPin = A0;              // pin being used (analogue input 0)

void setup()
{
  // initialize serial communication with computer
  Serial.begin(9600);
  // initialize all the readings to 0
  for (int thisReading = 0; thisReading < numReadings; thisReading++)
    readings[thisReading] = 0;
  // set the LED on (all systems are go!)
  digitalWrite(13, HIGH);
}

void loop()
{
  // subtract the last reading
  total = total - readings[index];
  // read voltage from analogue input 0 (TMP36)
  readings[index] = analogRead(inputPin);
  // add the reading to the total
  total = total + readings[index];
  // advance to the next position in the array
  index = index + 1;

  // if we're at the end of the array...
  if (index >= numReadings)
  {
    // ...flag that a full set is now available to print and...
    fullset = 1;
    // ...wrap around to the beginning
    index = 0;
  }

  // if we have a full set to calculate an everage with...
  if (fullset)
  {
    // calculate the average
    average = total / numReadings;
    // send it to the computer as a temperature
    Serial.println(int(average * 0.4883 - 50 + 0.5), DEC);
  }
}


