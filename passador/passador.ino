//Acelerometro e Giroscopio
#include <LSM6DS3.h>
#include <Wire.h>
//Bluetooth
#include <ArduinoBLE.h>
//Tensorflow
#include <TensorFlowLite.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>
//Modelo de Movimentos
#include "model.h"
// limiar de significativo em G's
const float limiteAcelerecao = 1.5;
const int numSamples = 119;

int samplesRead = numSamples;

LSM6DS3 myIMU(I2C_MODE, 0x6A);  

// Variaveis globais para o TensorFlow Lite Micro (TFLM)
tflite::MicroErrorReporter tflErrorReporter;

// Extrai todas as operações do TFLM
// Você pode remover esta linha e extrair apenas as operações do TFLM necessárias
// se quiser reduzir o tamanho compilado do esboço.
tflite::AllOpsResolver tflOpsResolver;

const tflite::Model* tflModel = nullptr;
tflite::MicroInterpreter* tflInterpreter = nullptr;
TfLiteTensor* tflInputTensor = nullptr;
TfLiteTensor* tflOutputTensor = nullptr;

// Crie um buffer de memória estática para TFLM, o tamanho pode precisar
// ser ajustado com base no modelo que você está usando
constexpr int tensorArenaSize = 8 * 1024;
byte tensorArena[tensorArenaSize] __attribute__((aligned(16)));

// Lista de movimentos reconhecidos
const char* GESTURES[] = {
  "cima",
  "baixo",
  "direita",
  "esquerda"
};
#define NUM_GESTURES (sizeof(GESTURES) / sizeof(GESTURES[0]))

//Definindo o uuid e a caracteristica do serviço de acesso do bluetooth
const char* deviceServiceUuid = "19b10000-e8f2-537e-4f6c-d104768a1214";
const char* deviceServiceCharacteristicUuid = "19b10001-e8f2-537e-4f6c-d104768a1214";
BLEService gestureService(deviceServiceUuid); 
BLEIntCharacteristic gestureCharacteristic(deviceServiceCharacteristicUuid, BLERead | BLENotify);
int movimento = -1;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  //Verificando IMU
  if (myIMU.begin() != 0) {
    Serial.println("Erro no dispositivo");
  } else {
    Serial.println("Dispositivo OK!");
  }

  Serial.println();

  // obtem a representação TFL da matriz de bytes do modelo
  tflModel = tflite::GetModel(model);
  if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Incompatibilidade de esquema do modelo!");
    while (1);
  }
  // Cria um intérprete para executar o modelo
  tflInterpreter = new tflite::MicroInterpreter(tflModel, tflOpsResolver, tensorArena, tensorArenaSize, &tflErrorReporter);

  // Alocar memória para os tensores de entrada e saída do modelo
  tflInterpreter->AllocateTensors();

  // Obtenha ponteiros para os tensores de entrada e saída do modelo
  tflInputTensor = tflInterpreter->input(0);
  tflOutputTensor = tflInterpreter->output(0);

  //Preparando Bluetooth
  if (!BLE.begin()) {
    Serial.println("* Iniciando Bluetooth® Low Energy - FALHA!");
    while (1);
  }
  //Definindo nome
  BLE.setLocalName("XIAO (SLIDE)");
  BLE.setAdvertisedService(gestureService);
  gestureService.addCharacteristic(gestureCharacteristic);
  BLE.addService(gestureService);
  gestureCharacteristic.writeValue(-1); 

  BLE.advertise();

  Serial.println("XIAO (SLIDE)");
  Serial.println(" ");
}

void loop() {
  conectar();
}
//Funcao para procurar o slave
void conectar(){
  BLEDevice central = BLE.central();
  Serial.println("- Procurando dispositivo...");
  if (central) {
    Serial.println("* Conectado ao dispositivo!");
    Serial.print("* MAC: ");
    Serial.println(central.address());
    Serial.println(" ");
    //Conecta e pega o valor do movimento
    while (central.connected()) {
      movimento = detectarMovimento();
      //Atualiza a caracteristica
      if(movimento!=-1){
        Serial.print("* Escrevendo valor para característica movimento: ");
        Serial.println(movimento);
        gestureCharacteristic.writeValue((byte) movimento);
        Serial.println("* Escrita de valor para característica movimento concluída!");
        Serial.println(" ");
      }
    }
    Serial.println("* Desconectado do dispositivo!");
  }
}
//Funcao que usa o tensorflow a o IMU para reconhecer o movimento
int detectarMovimento() {
  float aX, aY, aZ, gX, gY, gZ;
  // Espera por um movimento significativo
  while (samplesRead == numSamples) {
      // Leia os dados de aceleração
      aX = myIMU.readFloatAccelX();
      aY = myIMU.readFloatAccelY();
      aZ = myIMU.readFloatAccelZ();
      // Resumir os absolutos
      float aSum = fabs(aX) + fabs(aY) + fabs(aZ);
      // Verifique se está acima do limite
      if (aSum >= limiteAcelerecao) {
        // Redefinir a contagem de leitura da amostra
        samplesRead = 0;
        break;
      }
  }
  // Verifica se todas as amostras necessárias foram lidas desde
  // a última vez que o movimento significativo foi detectado
  while (samplesRead < numSamples) {
      // Verificar se novos dados de aceleração E giroscópio estão disponíveis
      // ler os dados de aceleração e giroscópio
      aX = myIMU.readFloatAccelX();
      aY = myIMU.readFloatAccelY();
      aZ = myIMU.readFloatAccelZ();

      gX = myIMU.readFloatGyroX();
      gY = myIMU.readFloatGyroY();
      gZ = myIMU.readFloatGyroZ();

      // Normalizar os dados da IMU entre 0 e 1 e armazená-los no tensor de entrada do modelo
      tflInputTensor->data.f[samplesRead * 6 + 0] = (aX + 4.0) / 8.0;
      tflInputTensor->data.f[samplesRead * 6 + 1] = (aY + 4.0) / 8.0;
      tflInputTensor->data.f[samplesRead * 6 + 2] = (aZ + 4.0) / 8.0;
      tflInputTensor->data.f[samplesRead * 6 + 3] = (gX + 2000.0) / 4000.0;
      tflInputTensor->data.f[samplesRead * 6 + 4] = (gY + 2000.0) / 4000.0;
      tflInputTensor->data.f[samplesRead * 6 + 5] = (gZ + 2000.0) / 4000.0;
      samplesRead++;
      if (samplesRead == numSamples) {
        // Executa a inferência
        TfLiteStatus invokeStatus = tflInterpreter->Invoke();
        if (invokeStatus != kTfLiteOk) {
          Serial.println("Invoke failed!");
          while (1);
          return -1;
        }
        // Percorrer os valores do tensor de saída do modelo
        for (int i = 0; i < NUM_GESTURES; i++) {
          //Verifica se o movimento é 80% equivalente e o seleciona para o envio
          if(tflOutputTensor->data.f[i]>0.8){
            Serial.print(GESTURES[i]);
            Serial.print(": ");
            Serial.println(tflOutputTensor->data.f[i], 6);
            return i;
          }
        }
      }else{
        return -1;
      }
  }
  return -1;
}