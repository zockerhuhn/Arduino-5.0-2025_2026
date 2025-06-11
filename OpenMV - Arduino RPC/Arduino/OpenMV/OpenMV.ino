#include <Arduino.h>
#include <openmvrpc.h>

// Nachrichten zwischen OpenMV Cam und Arduino können maximal 256 bytes lang sein:
openmv::rpc_scratch_buffer<256> scratch_buffer;
// Wir können maximal 8 Befehle registrieren:
openmv::rpc_callback_buffer<8> callback_buffer;

openmv::rpc_hardware_serial1_uart_slave openMvCam(115200);

// Hier stehen die Daten, die uns die OpenMV Cam sendet:
struct {
  uint16_t value1 = 0;
  uint16_t value2 = 0;
  uint16_t value3 = 0;
  uint16_t value4 = 0;
} daten_empfangen;

struct {
  uint16_t value1 = 0;
  uint16_t value2 = 0;
  uint16_t value3 = 0;
  uint16_t value4 = 0;
} daten_zum_senden;

size_t daten_aktualisieren(void *in_data, size_t in_data_len, void *out_data) {
    if (in_data_len == sizeof(daten_empfangen)) {
      // Daten sind vollständig
      // Den Puffer "in_data" kopieren in die Datenstruktur "daten_empfangen":
      memcpy(&daten_empfangen, in_data, sizeof(daten_empfangen));
    }

    // Optional: Etwas zurückschicken an die OpenMV Cam:
    // Für diesen Test gibt es nur ein ECHO, d.h. der Arduino sendet genau das zurück was er bekommen hat:
    daten_zum_senden.value1 = daten_empfangen.value1;
    daten_zum_senden.value2 = daten_empfangen.value2;
    daten_zum_senden.value3 = daten_empfangen.value3;
    daten_zum_senden.value4 = daten_empfangen.value4;
    // Die Datenstruktur "daten_zum_senden" in den Puffer out_data kopieren:
    memcpy(out_data, &daten_zum_senden, sizeof(daten_zum_senden));
    return sizeof(daten_zum_senden); // so viele Bytes werden versendet
}

void setup() {
  // Debug via USB:
  Serial.begin(115200);

  // ruft oben die Funktion daten_aktualisieren() auf, wenn die OpenMV Cam das will:
  openMvCam.register_callback(F("befehl1"), daten_aktualisieren);
  // Library initialisieren:
  openMvCam.begin();
}

void loop() {
    // Nachschauen, ob die OpenMV Cam dem Arduino was gesendet hat und die Werte abspeichern:
    openMvCam.loop(); // könnte manchmal daten_aktualisieren() aufrufen

    // jetzt könnte man die Daten irgendwie verwenden, zum Beispiel: daten_empfangen.value1
    Serial.println("OpenMV Daten: " +
      String(daten_empfangen.value1) + ", " + 
      String(daten_empfangen.value2) + ", " + 
      String(daten_empfangen.value3) + ", " + 
      String(daten_empfangen.value4));
}
