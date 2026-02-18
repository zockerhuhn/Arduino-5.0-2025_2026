import json
import rpc
import struct

interface_arduino = rpc.rpc_uart_master(baudrate=115200, uart_port=3)

def zum_arduino_senden(value1, value2, value3, value4):
    werte_zum_senden = (value1, value2, value3, value4)
    # senden --> "<HHHH" Paket enthält 4 16bit Zahlen:
    # führt auf dem Arduino "befehl1" aus:
    result = interface_arduino.call("befehl1", struct.pack("<HHHH", *werte_zum_senden))
    # optional: Antwort vom Arduino anschauen:
    if result is not None and len(result):
        # "<HHHH" Paket enthält 4 16bit Zahlen <-- empfangen
        # empfangene_werte ist ein Array:
        empfangene_werte = struct.unpack("<HHHH", result)
        print("Arduino Antwort {} {} {} {}".format(
            empfangene_werte[0],
            empfangene_werte[1],
            empfangene_werte[2],
            empfangene_werte[3]))
    else:
        print("Keine Antwort ;-(");

# Execute remote functions in a loop.
while True:
    value1 = 12
    value2 = 34
    value3 = 56
    value4 = 78
    # diese Zeile kann eine ganze Sekunde blockieren, wenn der Arduino nicht antwortet:
    zum_arduino_senden(value1, value2, value3, value4)
