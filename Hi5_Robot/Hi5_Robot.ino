
#include <Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <QueueList.h>



#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32


// Wifi and Broker config.
const char* ssid = "XXXXXXXX";
const char* password = "XXXXXXX";
const char* mqtt_server = "broker.mqttdashboard.com";
const char* sub_topic = "hi5robot/sendHi5";
const char* pub_topic = "hi5robot/reply";



Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Servo myservo;  // create servo object to control a servo
WiFiClient wifiClient;
PubSubClient client(wifiClient);
QueueList <String> queue;



// defines pins numbers
const int trigPin = 12;  //D4
const int echoPin = 11;  //D3
const int servoPin = 10;  //D3

// defines variables
long duration;
int distance;
long lastMsg = 0;
char msg[50];
//int hi5counter = 0;
String senderName = "";




void setup() {
  Wire.begin();  
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  Serial.begin(9600); // Starts the serial communication
  myservo.attach(servoPin);  // attaches the servo on pin 9 to the servo object
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  displayWelcomeMessage();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  queue.setPrinter (display);
  myservo.write(160);
}

void loop() {

  if (!client.connected()) {
    reconnect();
    displayIntialScreen();
  }
  client.loop();
  
  
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculating the distance
  distance= duration*0.034/2;
  // Prints the distance on the Serial Monitor
  
  
  if (distance<60)
  {  
    if (!queue.isEmpty()){
      displayCollectingHi5();
      myservo.write(40);
      delay(500);
      myservo.write(90);
      delay(1000);
     }
     else{
      displayIntialScreen();
     }
    
  }
  if (queue.isEmpty()){
    myservo.write(160);
  }
 
  delay(500);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0); 
  display.println("Connecting to ");
  display.println(ssid);
  display.display();
  display.println("");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  display.println("");
  display.println("WiFi connected");
  display.println("IP address: ");
  display.println(WiFi.localIP());
  display.display();
  delay(2000);
}

void displayWelcomeMessage(){
  display.clearDisplay();
  delay(1000);
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println("Hi5 Robot");
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.print("initialising");
  for (int i=0;i<9;i++)
  {
    display.print(".");
    delay(100);
    display.display();
  }
}

void displayIntialScreen(){
  
  display.clearDisplay();
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println("Hi5 Robot");
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.print(queue.count());
  display.print(" Hi5's waiting ");
  display.display();
}

void callback(char* topic, byte* payload, unsigned int length) {
    char tempPayload[15] = "";
    if (length > 15){
      length = 15;
    }
    for (int i = 0; i < length; i++) {
      tempPayload[i] = (char)payload[i];
  }
  if (length == 15){
    tempPayload[15] = '\0';
  }
  queue.push(String(tempPayload));
  display.clearDisplay();
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println("Hi5 Robot");
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.print(queue.count());
  display.println(" Hi5's waiting ");
  display.println("");
  display.print(String(tempPayload));
  display.println(" sent a Hi5");
  display.display();
  char msg[50];
  strcpy(msg,"Hi5 Robot: Got a Hi5 from ");
  strcat(msg,tempPayload);
  Serial.println("Went Here");
  client.publish(pub_topic, msg);
  myservo.write(90);
  delay(100);
  myservo.write(110);
  delay(100);
  myservo.write(90);
}

void reconnect() {


  // Loop until we're reconnected
  while (!client.connected()) {
    display.clearDisplay();
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setCursor(0,0);             // Start at top-left corner
    display.println("Attempting MQTT");
    display.println("connection...");
    display.display();
    if (client.connect("Hi5Robot")) {
      display.println();
      display.println("CONNECTED!");
      display.println("");
      client.subscribe(sub_topic);
      display.println("Subscribed to :");
      display.println(sub_topic);
      display.display();
      delay(2000);

    } else {
      display.print("failed, rc=");
      display.print(client.state());
      display.println(" try again in 5 seconds");
      display.display();
      delay(5000);
    }
  }
}

void displayCollectingHi5(){
  display.clearDisplay();
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println("Hi5 Robot");
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.print(queue.count()-1);
  display.println(" Hi5's waiting ");
  display.setCursor(0,32);   
  display.println("You took ");
  display.print(queue.pop());
  display.println("'s Hi5");
  display.display();
}
