
const int analogInPin = A0;
const int analogOutPin = 9;

int sensorValue = 0;
int outputValue = 0;

void setup() {
    Serial.begin(9600);
}


void loop() {

    sensorValue = analogRead(analogInPin);
    outputValue = map(sensorValue, 0, 1023, 0, 255);

    analogWrite(analogOutPin, outputValue);

    Serial.print("sensor = \n");
    Serial.print(sensorValue);
    Serial.print("\n");
    Serial.print("output = \n");
    Serial.print(outputValue);
    Serial.print("\n");

    delay(20);

    // to clean screen
    Serial.write(27);       // ESC command
    Serial.print("[2J");    // clear screen command
    Serial.write(27);
    Serial.print("[H");     // cursor to home command

    delay(2);

}


