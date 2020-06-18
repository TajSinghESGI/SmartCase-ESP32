//https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Register-Map1.pdf => DATASHEET MPU6050

#include "Wire.h" // Librairie pour les periphs I2C
#include <WiFi.h> //Librairie pour le Wifi
#include <WiFiManager.h>

const int MPU_ADDR = 0x68; // adresse I2C du MPU.

//int16_t permet de définir un int sur 16bits
int16_t accelerometer_x, accelerometer_y, accelerometer_z; // variables pour l'acceloremetre
int16_t gyro_x, gyro_y, gyro_z; // variable pour le gyro 

/*const char* ssidPhone = "AndroidTaj";
const char* passwordPhone = "";*/

WiFiManager wm;

const char* ssid = "SmartCase-WiFi";
const char* password = "SmartCase"; //TODO - Voir pour pouvoir se connecter directement depuis le tel ? - Fait !

void setup() {
  Serial.begin(9600);
  Wire.begin(33, 32); // Initialise l'I2C. 
  Wire.beginTransmission(MPU_ADDR); // Initialise la transmission à destination de la GY-521
  Wire.write(0x6B); // Registre d'alimentation
  Wire.write(0); // Mis a 0 permet d'allumer le MPU
  Wire.endTransmission(true);

  //Connexion au wifi
 /*WiFi.begin(ssidPhone, passwordPhone);
  Serial.print("Je me connecte...");
  
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }
  
  Serial.println("\n");
  Serial.println("Connexion etablie!");
  Serial.print("Adresse IP: ");
  Serial.println(WiFi.localIP());*/

  /*Serial.println();
  Serial.print("Configuring access point...");
  // You can remove the password parameter if you want the AP to be open. 
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);*/

  WiFi.mode(WIFI_STA);
  if(!wm.autoConnect(ssid, password))
    Serial.println("Erreur de connexion.");
  else
    Serial.println("Connexion etablie!");
  
}
void loop() {
  
  //Logique accéleromètre 
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); //Envoi de données vers la carte maitre
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 7*2, true); //La carte I2C demande 14 registres au GY-521.
  
  // "Wire.read()<<8 | Wire.read();" means two registers are read and stored in the same variable
  // On divise par 16384 pour avoir une mesure à +-2g (gravité).
 
  accelerometer_x = Wire.read()<<8 | Wire.read() / 16384;
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
  
  /*Serial.print("gX = "); Serial.print(gyro_x);
  Serial.print(" | gY = "); Serial.print(gyro_y);
  Serial.print(" | gZ = "); Serial.print(gyro_z);
  Serial.println();*/

  //Logique puissance du WiFi
  long rssi = WiFi.RSSI();
  
  Serial.print("Puissance du WiFi: ");
  Serial.println(rssi);
  
  // delay
  delay(5000);
}
