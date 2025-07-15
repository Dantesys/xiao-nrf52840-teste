import asyncio
from bleak import BleakScanner

async def main():
    print("🔍 Escaneando dispositivos BLE...")
    devices = await BleakScanner.discover()
    
    for i, device in enumerate(devices):
        print(f"{i}: {device.name} [{device.address}]")

asyncio.run(main())