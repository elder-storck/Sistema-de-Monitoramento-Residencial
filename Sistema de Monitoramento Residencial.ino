#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Nome e senha do Wi-Fi a ser utilizado
const char* ssid = "-";
const char* password = "456@Lan123";

#define BOTtoken "5793271201:AAHtwCaEOaWW3aZMoLCFCp77XMXPvE068ak"  //Bot Token que é gerado ao criar o bot no telegram

// ID individual do telegram.
#define CHAT_ID "5535200911"

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int botRequestDelay = 500;
long lastTimeBotRan;

// Configurações do sensor de movimento
const int motionSensor = 13; // Pino utilizado
bool motionDetected = false; // Indica se houve ou não movimento
bool motionDetectEnable = false; // Indica se a opção do sensor de movimento está ativado.

//Configuração do sensor magnético da porta
const int door1 = 12;//Pino utilizado
bool door1stat = true;//Indica se a porta está aberta
bool doorLockMonitor = false;// Indica se a opção do sensor da porta está ativado.

void handleNewMessages(int numNewMessages);

void setup() {
  configTime(0, 0, "pool.ntp.org"); // Pega o tempo UTC via NTP
  client.setTrustAnchors(&cert); // Adiciona certificação de root para api.telegram.org

//Configuração dos pinos dos sensores.
  pinMode(motionSensor, INPUT);
  pinMode(door1, INPUT);

// Realiza a configuração Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    //Serial.print(".");
    delay(500);
  }

// Envia a mensagem de inicialização do sistema de monitoramento para o usuário com o ID "CHAT_ID"
  bot.sendMessage(CHAT_ID, "Sistema de monitoramento inicializado!", "");
  String welcome = " Bem vindo ao sistema de monitoramento utilizando Telegram bot.\n\n";
      welcome += "Utilize os seguintes comandos para habilitar suas funcionalidades:\n\n";
      welcome += "/MovimentoOn : Habilita o sensor de movimento\n";
      welcome += "/MovimentoOff : Desabilita o sensor de movimento\n\n";
      welcome += "/PortaOn : Habilita o sensor da porta\n";
      welcome += "/PortaOff : Desabilita o sensor da porta\n";

      bot.sendMessage(CHAT_ID, welcome, "Markdown");
}

void loop() {
  //Verifica a entrada no sensor de movimento
  motionDetected = digitalRead(motionSensor); 
  //Verifica se houve movimento e se o sensor de movimento está ativado no bot do telegram e nesse caso avisa sobre o movimento
  if(motionDetectEnable && motionDetected){
    bot.sendMessage(CHAT_ID, "Movimento detectado!!", "");
    motionDetected = false;
  }
  //Verifica a entrada no sensor magnético de porta.
  door1stat = digitalRead(door1);
  //Verifica se a porta está aberta se o sensor magnético de porta está ativado no bot do telegram e nesse caso avisa que a porta está aberta
  if(doorLockMonitor && !door1stat){
    bot.sendMessage(CHAT_ID, "Porta aberta!!", "");
    door1stat = true;
  }
  //Verifica se novas mensagens foram enviadas pelo usuário no chat do telegram
  if (millis() > lastTimeBotRan + botRequestDelay){
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages){
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}

// Altera variáveis de acordo com o que o usuário enviou no chat do telegram
void handleNewMessages(int numNewMessages){

  for (int i = 0; i < numNewMessages; i++){
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id !=  CHAT_ID){
      bot.sendMessage(chat_id, "Usuário não autorizado", "");
      continue;
    }
  
    String text = bot.messages[i].text;

    String fromName = bot.messages[i].from_name;

  //Habilita o sensor de movimento e avisa no chat do telegram que ele está ativado
    if (text == "/MovimentoOn") {
       motionDetectEnable = true;
       bot.sendMessage(CHAT_ID, "Sensor de movimento ativado", "");
    }
   //Desabilita o sensor de movimento e avisa no chat do telegram que ele está desativado
    if (text == "/MovimentoOff") {
       motionDetectEnable = false;
       bot.sendMessage(CHAT_ID, "Sensor de movimento desativado", "");
    }
  //Habilita o sensor magnético de porta e avisa no chat do telegram que ele está ativado
    if (text == "/PortaOn") {
       doorLockMonitor = true;
       bot.sendMessage(CHAT_ID, "Sensor da porta ativado", "");
    }
   //Desabilita o sensor magnético de porta e avisa no chat do telegram que ele está desativado
    if (text == "/PortaOff") {
       doorLockMonitor = false;
       bot.sendMessage(CHAT_ID, "Sensor da porta desativado", "");
    }
  }
}
