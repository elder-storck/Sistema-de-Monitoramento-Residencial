#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "name";
const char* password = "password";

// Initialize Telegram BOT
#define BOTtoken "-"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "-"

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int botRequestDelay = 500;
long lastTimeBotRan;

const int motionSensor = 13; // PIR Motion Sensor
bool motionDetected = false;
bool motionDetectEnable = false;

const int door1 = 12;
bool door1stat = true;
bool doorLockMonitor = false;

// Indicates when motion is detected
void ICACHE_RAM_ATTR detectsMovement() {
  Serial.println("MOTION DETECTED!!!");
  motionDetected = true;
}

void handleNewMessages(int numNewMessages);

void setup() {
  //Serial.begin(115200);
  configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
  client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org

  pinMode(motionSensor, INPUT);
  pinMode(door1, INPUT);

  // Attempt to connect to Wifi network:
  //Serial.print("Connecting Wifi: ");
  //Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    //Serial.print(".");
    delay(500);
  }
/*
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
*/
  bot.sendMessage(CHAT_ID, "Sistema de monitoramento inicializado!", "");
}

void loop() {
  motionDetected = digitalRead(motionSensor);  
  if(motionDetectEnable && motionDetected){
    bot.sendMessage(CHAT_ID, "Movimento detectado!!", "");
    //Serial.println("Motion Detected");
    motionDetected = false;
  }
  door1stat = digitalRead(door1);
  if(doorLockMonitor && !door1stat){
    bot.sendMessage(CHAT_ID, "Porta aberta!!", "");
    //Serial.println("Door opened");
    door1stat = true;
  }

  if (millis() > lastTimeBotRan + botRequestDelay){
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages){
      //Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}

void handleNewMessages(int numNewMessages){
  //Serial.print("Handle New Messages: ");
  //Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++){
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id !=  CHAT_ID){
      bot.sendMessage(chat_id, "Usuário não autorizado", "");
      continue;
    }
   
    // Print the received message
    String text = bot.messages[i].text;
    //Serial.println(text);

    String fromName = bot.messages[i].from_name;

    if (text == "/MovimentoOn") {
       motionDetectEnable = true;
       bot.sendMessage(CHAT_ID, "Sensor de movimento ativado", "");
       //Serial.println("Enable the Motion alert");
    }
   
    if (text == "/MovimentoOff") {
       motionDetectEnable = false;
       bot.sendMessage(CHAT_ID, "Sensor de movimento desativado", "");
       //Serial.println("Disable the Motion alert");
    }

    if (text == "/PortaOn") {
       doorLockMonitor = true;
       bot.sendMessage(CHAT_ID, "Sensor da porta ativado", "");
       //Serial.println("Enable the Monitor Doors");
    }
   
    if (text == "/PortaOff") {
       doorLockMonitor = false;
       bot.sendMessage(CHAT_ID, "Sensor da porta desativado", "");
       //Serial.println("Disable the Monitor Doors");
    }

    if (text == "/iniciar"){
      String welcome = " Bem vindo ao sistema de monitoramento utilizando Telegram bot.\n\n";
      welcome += "Utilize os seguintes comandos para habilitar suas funcionalidades:\n\n";
      welcome += "/MovimentoOn : Habilita o sensor de movimento\n";
      welcome += "/MovimentoOff : Desabilita o sensor de movimento\n\n";
      welcome += "/PortaOn : Habilita o sensor da porta\n";
      welcome += "/PortaOff : Desabilita o sensor da porta\n";

      bot.sendMessage(CHAT_ID, welcome, "Markdown");
    }
  }
}