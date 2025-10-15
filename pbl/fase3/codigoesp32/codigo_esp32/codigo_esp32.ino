#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "DHT.h" 
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "secrets.h"

// --- CONFIGURAÇÕES DOS DISPLAYS I2C ---
LiquidCrystal_I2C lcd_nutrientes(0x27, 20, 4); 
LiquidCrystal_I2C lcd_clima(0x3F, 20, 4); 

// --- Configurações dos Componentes ---
#define DHTPIN 25
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// --- Pinos dos Componentes ---
const int pinoFosforo = 2;
const int pinoPotassio = 4;
const int pinoNitrogenio = 15;
const int pinoLDR = 34;
const int pinoRele = 26;

// --- Variáveis Globais de Estado ---
long valorN = 0, valorP = 0, valorK = 0; 
float valorPH = 7.0;
float valorUmidade = 0.0;
bool bombaLigada = false;
int estadoFosforoAnterior = HIGH, estadoPotassioAnterior = HIGH, estadoNitrogenioAnterior = HIGH;

// --- Variáveis de Controle de Tempo para Envio de Dados ---
unsigned long tempoAnteriorEnvio = 0;
const long intervaloEnvio = 20000; // Envia dados a cada 20 segundos

// --- Parâmetros da Cultura (Exemplo: Soja) ---
const float UMIDADE_MINIMA = 50.0;
const float PH_IDEAL_MIN = 6.0;
const float PH_IDEAL_MAX = 6.8;
const int N_IDEAL_MIN = 9;
const int N_IDEAL_MAX = 15;
const int P_IDEAL_MIN = 75;
const int P_IDEAL_MAX = 95;
const int K_IDEAL_MIN = 80;
const int K_IDEAL_MAX = 100;

// --- CONFIGURAÇÕES API E VARIÁVEIS GLOBAIS DE CLIMA ---
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
String apiKeyClima = API_KEY;
String cidadeCodificada = "Sao%20Paulo,br"; 
String openWeatherMapUrl = "http://api.openweathermap.org/data/2.5/weather?q=" + cidadeCodificada + "&appid=" + apiKeyClima + "&units=metric&lang=pt_br";
String nomeCidadeGlobal = "Carregando...", descricaoClimaGlobal = "Buscando..."; 
float temperaturaGlobal = 0.0, sensacaoTermicaGlobal = 0.0, velocidadeVentoGlobal = 0.0, umidadeArGlobal = 0.0;

// --- CONFIGURAÇÕES DO THINGSPEAK ---
String thingSpeakApiKey = THINGSPEAK_API_KEY;

// --- FUNÇÕES DO PROJETO ---

void atualizarClima() {
  Serial.print("Buscando clima...");
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(openWeatherMapUrl);
    int httpResponseCode = http.GET();
    if (httpResponseCode == 200) {
      String payload = http.getString();
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, payload);
      if (!error) {
        nomeCidadeGlobal = String(doc["name"].as<const char*>());
        temperaturaGlobal = doc["main"]["temp"];
        sensacaoTermicaGlobal = doc["main"]["feels_like"];
        umidadeArGlobal = doc["main"]["humidity"];
        descricaoClimaGlobal = String(doc["weather"][0]["description"].as<const char*>());
        velocidadeVentoGlobal = (float)doc["wind"]["speed"] * 3.6; // Converte m/s para km/h
        Serial.println(" OK.");
      } else {
        Serial.print("Erro JSON: "); Serial.println(error.c_str());
        descricaoClimaGlobal = "Erro JSON";
      }
    } else {
      Serial.print("Erro HTTP: "); Serial.println(httpResponseCode);
      descricaoClimaGlobal = "Erro HTTP";
    }
    http.end();
  } else {
    Serial.println(" Wi-Fi Desconectado.");
    descricaoClimaGlobal = "Wi-Fi OFF";
  }
}

void enviarDadosThingSpeak() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "http://api.thingspeak.com/update?api_key=" + thingSpeakApiKey;
    url += "&field1=" + String(valorPH, 2);
    url += "&field2=" + String(valorUmidade, 1);
    url += "&field3=" + String(valorN);
    url += "&field4=" + String(valorP);
    url += "&field5=" + String(valorK);
    url += "&field6=" + String(bombaLigada ? 1 : 0);
    http.begin(url);
    Serial.println("Enviando dados para o ThingSpeak...");
    int httpCode = http.GET();
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.print("Resposta do ThingSpeak: ");
      Serial.println(payload);
    } else {
      Serial.print("Erro no envio para o ThingSpeak: ");
      Serial.println(http.errorToString(httpCode).c_str());
    }
    http.end();
  }
}

void atualizarDisplayNutrientes() {
  lcd_nutrientes.clear();
  lcd_nutrientes.setCursor(0, 0); lcd_nutrientes.print("--- SENSORES/SOLO ---");
  lcd_nutrientes.setCursor(0, 1); lcd_nutrientes.print("N:" + String(valorN) + " P:" + String(valorP) + " K:" + String(valorK));
  lcd_nutrientes.setCursor(0, 2); lcd_nutrientes.print("Solo(%) " + String(valorUmidade, 0));
  lcd_nutrientes.setCursor(13, 2); lcd_nutrientes.print("pH " + String(valorPH, 1));
  lcd_nutrientes.setCursor(0, 3); lcd_nutrientes.print("Bomba: "); lcd_nutrientes.print(bombaLigada ? "LIGADA" : "DESLIGADA");
}

void atualizarDisplayClima() {
  lcd_clima.clear();
  lcd_clima.setCursor(0, 0); lcd_clima.print(nomeCidadeGlobal.substring(0, 20));
  lcd_clima.setCursor(0, 1); lcd_clima.print("Temp: " + String(temperaturaGlobal, 1) + (char)223 + "C");
  lcd_clima.setCursor(10, 1); lcd_clima.print("Sens: " + String(sensacaoTermicaGlobal, 1) + (char)223 + "C");
  lcd_clima.setCursor(0, 2); lcd_clima.print("Ar(%) " + String(umidadeArGlobal, 0));
  lcd_clima.setCursor(10, 2); lcd_clima.print("Vento:" + String(velocidadeVentoGlobal,1) + "km/h");
  lcd_clima.setCursor(0, 3); lcd_clima.print(descricaoClimaGlobal.substring(0, 20)); 
}

void verificarIrrigacao() {
  bool umidadeBaixa = valorUmidade < UMIDADE_MINIMA;
  bool phIdeal = (valorPH >= PH_IDEAL_MIN && valorPH <= PH_IDEAL_MAX);
  bool nutrientesOk = (valorN >= N_IDEAL_MIN && valorN <= N_IDEAL_MAX) &&
                      (valorP >= P_IDEAL_MIN && valorP <= P_IDEAL_MAX) &&
                      (valorK >= K_IDEAL_MIN && valorK <= K_IDEAL_MAX);

  if (umidadeBaixa && phIdeal && nutrientesOk) {
    digitalWrite(pinoRele, HIGH);
    bombaLigada = true;
  } else {
    digitalWrite(pinoRele, LOW);
    bombaLigada = false;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("tempo_ms,ph,umidade,N,P,K,bomba");
  
  lcd_nutrientes.init(); lcd_nutrientes.backlight();
  lcd_clima.init(); lcd_clima.backlight();
  
  lcd_nutrientes.setCursor(0, 0); lcd_nutrientes.print("--- SMART FARM ---");
  lcd_nutrientes.setCursor(0, 1); lcd_nutrientes.print("Iniciando Sensores...");
  lcd_clima.setCursor(0, 0); lcd_clima.print("--- CLIMA API ---");
  lcd_clima.setCursor(0, 1); lcd_clima.print("Aguardando Wi-Fi...");
  
  dht.begin();
  randomSeed(analogRead(pinoLDR)); 
  
  pinMode(pinoRele, OUTPUT); digitalWrite(pinoRele, LOW);
  pinMode(pinoFosforo, INPUT_PULLUP);
  pinMode(pinoPotassio, INPUT_PULLUP);
  pinMode(pinoNitrogenio, INPUT_PULLUP);

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
    delay(500); Serial.print("."); tentativas++;
  }
  
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println(" Conectado!");
    atualizarClima();
  } else {
    Serial.println(" Falha na conexao Wi-Fi.");
  }

  atualizarDisplayNutrientes();
  atualizarDisplayClima();
}

void loop() {
  bool algoMudou = false;
  bool estadoBombaAntes = bombaLigada;

  // Leitura dos Sensores e Botões
  float novoPH = (analogRead(pinoLDR) / 4095.0) * 14.0; 
  if (abs(novoPH - valorPH) > 0.1) { valorPH = novoPH; algoMudou = true; }
  
  float novaUmidade = dht.readHumidity();
  if (!isnan(novaUmidade) && abs(novaUmidade - valorUmidade) > 1.0) { valorUmidade = novaUmidade; algoMudou = true; }
  
  if (digitalRead(pinoFosforo) == LOW && estadoFosforoAnterior == HIGH) { valorP = random(P_IDEAL_MIN, P_IDEAL_MAX + 1); algoMudou = true; delay(50); }
  estadoFosforoAnterior = digitalRead(pinoFosforo);

  if (digitalRead(pinoPotassio) == LOW && estadoPotassioAnterior == HIGH) { valorK = random(K_IDEAL_MIN, K_IDEAL_MAX + 1); algoMudou = true; delay(50); }
  estadoPotassioAnterior = digitalRead(pinoPotassio);

  if (digitalRead(pinoNitrogenio) == LOW && estadoNitrogenioAnterior == HIGH) { valorN = random(N_IDEAL_MIN, N_IDEAL_MAX + 1); algoMudou = true; delay(50); }
  estadoNitrogenioAnterior = digitalRead(pinoNitrogenio);
  
  verificarIrrigacao();

  if (algoMudou || (bombaLigada != estadoBombaAntes)) {
    atualizarDisplayNutrientes();
  }

  // --- TEMPORIZADOR ÚNICO PARA TODO O ENVIO DE DADOS ---
  unsigned long tempoAtual = millis();
  if (tempoAtual - tempoAnteriorEnvio >= intervaloEnvio) {
    tempoAnteriorEnvio = tempoAtual;

    // Tarefa 1: Enviar para o Monitor Serial (para o R)
    Serial.printf("%lu,%.2f,%.1f,%ld,%ld,%ld,%d\n",
                  millis(), valorPH, valorUmidade,
                  valorN, valorP, valorK,
                  bombaLigada ? 1 : 0);
                  
    // Tarefa 2: Enviar para a Nuvem (ThingSpeak)
    enviarDadosThingSpeak();
  }
}