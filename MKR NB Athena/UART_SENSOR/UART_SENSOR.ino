const char* IR_ID = "B3122115";

void setup() {
  Serial.begin(38400);
  delay(100); // Wait for serial communication to stabilize
  Serial.println(" "); // Print a newline for clarity
  sendData('S');
  sendData('A');
}

void loop() {
  String response = receiveData();
  if (response.length() > 0) {
    Serial.println(response);
  }
  delay(400);
}

void sendData(char commandLetter) {
  char package[14];
  sprintf(package, "[%c%s]", commandLetter, IR_ID);
  Serial.println(package);
  delay(100); // Adjust delay as needed for the sensor to process the command
}

String receiveData() {
  String completedData;
  while (Serial.available()) {
    char c = Serial.read();
    completedData += c;
    if (c == ']') {
      break; // Exit the loop when end character is received
    }
  }
  return completedData;
}
