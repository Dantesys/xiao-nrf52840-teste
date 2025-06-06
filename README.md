# Xiao nrf52840 Teste
É recomendável usar a Seeed nRF52 mbed-enabled Boards<br/>
Navegue até Arquivo > Preferências e preencha "URLs adicionais do gerenciador de placas" com a URL abaixo: https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json<br/>
Bibliotecas: 
 - Seeed Arduino LSM6DS3 https://github.com/Seeed-Studio/Seeed_Arduino_LSM6DS3
 - ArduinoBLE
 - TensorFlow Lite Exemplos https://github.com/lakshanthad/tflite-micro-arduino-examples<br/>
O Xiao nrf52840 Master é o responsavel por fazer o reconhecimento das movimentações e acessar via bluetooth o serviço de movimento do Slave<br/>
O Xiao nrf52840 Slave é o responsavel por fornecer o serviço de movimento e tem um atributo de serviço que o Master altera, com isso os led's dele muda com base no movimento<br/>
Lista de movimentos:<br/>
- Cima
- Baixo
- Direita
- Esquerda
