void setup_wifi_manager();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  // Initialise wifi connection
  Serial.println("Booting");
  setup_wifi_manager();
}

void loop() {
  // put your main code here, to run repeatedly:
  
}
