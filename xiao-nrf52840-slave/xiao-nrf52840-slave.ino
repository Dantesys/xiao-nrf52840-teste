#include <ArduinoBLE.h>
//MOVIMENTOS
enum {
  NENHUM  = -1,
  CIMA    = 0,
  BAIXO  = 1,
  DIREITA  = 2,
  ESQUERDA = 3
};

int movimento = -1;
//Definindo o servico Bluetooth
const char* deviceServiceUuid = "19b10000-e8f2-537e-4f6c-d104768a1214";
const char* deviceServiceCharacteristicUuid = "19b10001-e8f2-537e-4f6c-d104768a1214";
BLEService gestureService(deviceServiceUuid); 
BLEByteCharacteristic gestureCharacteristic(deviceServiceCharacteristicUuid, BLERead | BLEWrite);

void setup() {
  Serial.begin(9600);
  while (!Serial);  
  //Leds
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BLUE, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_BUILTIN, LOW);

  
  if (!BLE.begin()) {
    Serial.println("- Iniciando Bluetooth® Low Energy - FALHA!");
    while (1);
  }
  //Definindo nome e caracteristica
  BLE.setLocalName("XIAO (SLAVE)");
  BLE.setAdvertisedService(gestureService);
  gestureService.addCharacteristic(gestureCharacteristic);
  BLE.addService(gestureService);
  gestureCharacteristic.writeValue(-1);
  BLE.advertise();

  Serial.println("XIAO (SLAVE)");
  Serial.println(" ");
}

void loop() {
  //Procurar o MASTER
  BLEDevice central = BLE.central();
  Serial.println("- Procurando um MASTER...");
  delay(500);

  if (central) {
    Serial.println("* Conectado ao dispositivo MASTER!");
    Serial.print("* MASTER MAC: ");
    Serial.println(central.address());
    Serial.println(" ");
    //Conecta e pega o valor do movimento
    while (central.connected()) {
      if (gestureCharacteristic.written()) {
         movimento = gestureCharacteristic.value();
         gravaMovimento(movimento);
       }
    }
    
    Serial.println("* Desconectado do dispositivo MASTER!");
  }
}
//Funcao para mudar a caracteristica
void gravaMovimento(int movimento) {
  Serial.println("- A característica <movimento> foi alterada!");
  
   switch (movimento) {
      case CIMA:
        Serial.println("* Valor Atual: CIMA (VERMELHO LED ON)");
        Serial.println(" ");
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_GREEN, HIGH);
        digitalWrite(LED_BLUE, HIGH);
        digitalWrite(LED_BUILTIN, LOW);
        break;
      case BAIXO:
        Serial.println("* Valor Atual: BAIXO (VERDE LED ON)");
        Serial.println(" ");
        digitalWrite(LED_RED, HIGH);
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_BLUE, HIGH);
        digitalWrite(LED_BUILTIN, LOW);
        break;
      case DIREITA:
        Serial.println("* Valor Atual: DIREITA (AZUL LED ON)");
        Serial.println(" ");
        digitalWrite(LED_RED, HIGH);
        digitalWrite(LED_GREEN, HIGH);
        digitalWrite(LED_BLUE, LOW);
        digitalWrite(LED_BUILTIN, LOW);
        break;
      case ESQUERDA:
        Serial.println("* Valor Atual: ESQUERDA (BUILTIN LED ON)");
        Serial.println(" ");
        digitalWrite(LED_RED, HIGH);
        digitalWrite(LED_GREEN, HIGH);
        digitalWrite(LED_BLUE, HIGH);
        digitalWrite(LED_BUILTIN, HIGH);
        break;
      default:
        digitalWrite(LED_RED, HIGH);
        digitalWrite(LED_GREEN, HIGH);
        digitalWrite(LED_BLUE, HIGH);
        digitalWrite(LED_BUILTIN, LOW);
        break;
    }      
}
