#include <Wire.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "MAX30100_PulseOximeter.h"

#define REPORTING_PERIOD_MS 1000

// Credenciales WiFi
const char* ssid = "iPhone de Alejandra";
const char* password = "12345678";

// Crear una instancia del objeto PulseOximeter
PulseOximeter pox;
AsyncWebServer server(80); // Crear el servidor en el puerto 80

uint32_t tsLastReport = 0;

// Callback para la detección de pulsos
void onBeatDetected() {
    Serial.println("Beat!");
}

// Función para obtener datos de frecuencia cardíaca
String getHeartRate() {
    return String(pox.getHeartRate());
}

// Función para obtener datos de SpO2
String getSpO2() {
    return String(pox.getSpO2());
}

void setup() {
    Serial.begin(115200);
    
    // Inicializar la conexión WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Conectando a WiFi...");
    }
    Serial.println("Conectado a WiFi, IP: " + WiFi.localIP().toString());

    // Inicializar el sensor MAX30100
    Serial.print("Inicializando el oxímetro de pulso..");
    if (!pox.begin()) {
        Serial.println("FALLIDO");
        for (;;);
    } else {
        Serial.println("ÉXITO");
    }

    // Registrar el callback para la detección de pulsos
    pox.setOnBeatDetectedCallback(onBeatDetected);

    // Ruta para mostrar los datos en el servidor web
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = "<html><head><title>Datos del Oxímetro</title>";
        html += "<script>";
        html += "function fetchData() {";
        html += "  fetch('/data').then(response => response.text()).then(data => {";
        html += "    document.getElementById('sensorData').innerHTML = data;";
        html += "  });";
        html += "}";
        html += "setInterval(fetchData, 1000);"; // Actualizar cada segundo
        html += "</script></head>";
        html += "<body><h1>Datos del Oxímetro</h1>";
        html += "<div id='sensorData'>Cargando datos...</div>";
        html += "</body></html>";
        request->send(200, "text/html", html);
    });

    // Ruta para enviar los datos de frecuencia cardíaca y SpO2
    server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request) {
        String data = "Frecuencia cardíaca: " + getHeartRate() + " bpm<br>";
        data += "SpO2: " + getSpO2() + " %";
        request->send(200, "text/plain", data);
    });

    // Iniciar el servidor
    server.begin();
}

void loop() {
    // Actualizar el sensor MAX30100
    pox.update();

    // Reportar datos a través del Serial Monitor
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        Serial.print("Frecuencia cardíaca: ");
        Serial.print(getHeartRate());
        Serial.print(" bpm / SpO2: ");
        Serial.print(getSpO2());
        Serial.println(" %");
        tsLastReport = millis();
    }
}

