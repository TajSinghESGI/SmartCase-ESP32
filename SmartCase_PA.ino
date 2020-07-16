//https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Register-Map1.pdf => DATASHEET MPU6050

#include "Wire.h" // Librairie pour les periphs I2C
#include <WiFi.h> //Librairie pour le Wifi
#include <WebSocketClient.h>
#include <WiFiManager.h>
#include <SocketIoClient.h>
#include <PubSubClient.h>

#define mqtt_server "0.tcp.ngrok.io"
#define user "bluecase"
#define mdp "bluecase"
#define wifi_topic "WIFI/pa"
#define topic_lastWill "WIFI/death"
#define topic_poids "POIDS/pa"
#define topic_poids_flutter "POIDS/pa/flutter"
#define port 17606
#define client_id "8ca1f20ac3564e73a6b3f836b235d505"

const int MPU_ADDR = 0x68; // adresse I2C du MPU.

//int16_t permet de définir un int sur 16bits
int16_t accelerometer_x, accelerometer_y, accelerometer_z; // variables pour l'acceloremetre
int16_t gyro_x, gyro_y, gyro_z; // variable pour le gyro 

//Permet de stocker les nouveaux ssid et password depuis la lib wifimanager.
char new_ssid[50]; 
char new_password[50];

float randNumber = 0.0;
float poidFixe = 7.0;

char message_buff[100];
long lastMsg = 0;   //Horodatage du dernier message publié sur MQTT
long lastRecu = 0;

WiFiManager wm;
WiFiClient espClient;
PubSubClient client(espClient);

//Reseau wifi en AP de l'esp utile pour le setup de l'arduino.
const char* ssid = "SmartCase";
const char* password = "SmartCase";

void setup_wifi(char* ssid, char* password) {
  delay(10);
  Serial.println();
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    Serial.print("Connexion a ");
    Serial.println(ssid);
  }
  Serial.println("");
  Serial.println("Connexion WiFi etablie ");
  Serial.print("=> Addresse IP : ");
  Serial.print(WiFi.localIP());
}

void reconnect() {
  //Boucle jusqu'à obtenir une reconnexion
  while (!client.connected()) {
    Serial.print("Connexion au serveur MQTT...");
    if (client.connect(client_id, user, mdp, topic_lastWill, 0, false, "Je suis mort")) {
      Serial.println("OK");
      client.subscribe(topic_poids_flutter);                                                      //Logique accéleromètre 
    } else {
      client.setKeepAlive(5);
      Serial.print("KO, erreur : ");
      Serial.print(client.state());
      Serial.println(" On attend 5 secondes avant de recommencer");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(33, 32); // Initialise l'I2C. 
  Wire.beginTransmission(MPU_ADDR); // Initialise la transmission à destination de la GY-521
  Wire.write(0x6B); // Registre d'alimentation
  Wire.write(0); // Mis a 0 permet d'allumer le MPU
  Wire.endTransmission(true);

  //Connexion au wifi
  WiFi.mode(WIFI_STA);
  wm.setAPStaticIPConfig(IPAddress(8,8,8,8), IPAddress(8,8,8,8), IPAddress(255,255,255,0));
  if(!wm.autoConnect(ssid, password))
    Serial.println("Erreur de connexion.");
  else {
    setup_wifi(new_ssid, new_password);
  }
  
  client.setServer(mqtt_server, port);    //Configuration de la connexion au serveur MQTT
  client.setKeepAlive(5);
  client.setCallback(callback);  //La fonction de callback qui est executée à chaque réception de message   
  //Serial.println(wm.getWiFiPass());
  //Serial.println(wm.getWiFiSSID());
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  
  client.loop();
  long now = millis();
  if (now - lastRecu > 100 ) {
    lastRecu = now;
    client.subscribe(topic_poids_flutter);                                                      //Logique accéleromètre 
  }
  /*
   *Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B); //Envoi de données vers la carte maitre
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, 7*2, true); //La carte I2C demande 14 registres au GY-521.
   */
  
  // "Wire.read()<<8 | Wire.read();" means two registers are read and stored in the same variable
  // On divise par 16384 pour avoir une mesure à +-2g (gravité).
 
  /*
   *accelerometer_x = Wire.read()<<8 | Wire.read() / 16384;
    accelerometer_y = Wire.read()<<8 | Wire.read() / 16384;
    accelerometer_z = Wire.read()<<8 | Wire.read() / 16384;

  //On divise par 131 pour avoir une portée de 250°
    gyro_x = Wire.read()<<8 | Wire.read() / 131; 
    gyro_y = Wire.read()<<8 | Wire.read() / 131; 
    gyro_z = Wire.read()<<8 | Wire.read() / 131;
  

    Serial.print("aX = "); Serial.print(accelerometer_x);
    Serial.print(" | aY = "); Serial.print(accelerometer_y);
    Serial.print(" | aZ = "); Serial.print(accelerometer_z);
  
    Serial.println();
  */
  
                                                          //Logique puissance du WiFi
  long rssi = WiFi.RSSI();
  char cstr[16];
  
  //Serial.print("Puissance du WiFi: ");
  //Serial.println(rssi);

  //Serial.println(String(rssi).c_str());
  
  client.publish(wifi_topic, String(rssi).c_str(), false);                                                      
 
  // delay
  delay(2000);
}

void callback(char* topic, byte* payload, unsigned int length) {

  int i = 0;
  String msgString = "";
  String oldMsg = "";

  randNumber = random(0, 99);
  poidFixe = poidFixe + randNumber*0.01;
  Serial.print("POIDS: ");
  Serial.println(poidFixe);
    
  // create character buffer with ending null terminator (string)
  for(i=0; i<length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';
  
  msgString = String(message_buff);
  Serial.println("Payload: " + msgString);
  if ( msgString == "ON" ) {
    client.publish(topic_poids, String(poidFixe).c_str(), false);
  }
  poidFixe = 7.0;
}
