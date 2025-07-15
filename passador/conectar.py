import asyncio
import pyautogui
from bleak import BleakClient

# UUIDs definidos no código Arduino
SERVICE_UUID = "19b10000-e8f2-537e-4f6c-d104768a1214"
CHARACTERISTIC_UUID = "19b10001-e8f2-537e-4f6c-d104768a1214"
#MUDE O MAC DEPEDENDO DO SEU PERIFERICO
ADRESSMAC = "5A:5A:E3:9E:5B:E2"
async def main(endereco):
    async with BleakClient(endereco) as client:
        print("✅ Conectado com sucesso!")
        
        # Leitura única (opcional, se você quiser testar)
        value = await client.read_gatt_char(CHARACTERISTIC_UUID)
        print("📦 Valor inicial byte:", value)
        print("📦 Valor inicial convertido:", int.from_bytes(value, byteorder='little', signed=True))
        # Notificações contínuas
        def callback(sender, data: bytearray):
            if len(data) == 4:
                valor = int.from_bytes(data, byteorder='little', signed=True)
                
                direcoes = {
                    -1: "❓ Movimento desconhecido",
                    0: "⬆️ Movimento para cima",
                    1: "⬇️ Movimento para baixo",
                    2: "➡️ Movimento para a direita",
                    3: "⬅️ Movimento para a esquerda"
                }
                
                print(f"📦 Valor bruto: {valor}")
                print(f"🧭 Interpretação: {direcoes.get(valor, 'Valor inválido')}")
                if(valor==0):
                    pyautogui.keyDown('shift')
                    pyautogui.press('f5')
                    pyautogui.keyUp('shift')
                elif(valor==2):
                    pyautogui.press('right')
                elif(valor==3):
                    pyautogui.press('left')
                elif(valor==1):
                    pyautogui.press('esc')
            else:
                print(f"❗ Dados inesperados: {data}")


        await client.start_notify(CHARACTERISTIC_UUID, callback)

        print("⏳ Aguardando notificações... pressione Ctrl+C para sair")
        while True:
            await asyncio.sleep(1)

asyncio.run(main(ADRESSMAC))