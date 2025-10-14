#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "DHT.h" 
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// --- CONFIGURAÇÕES DOS DISPLAYS I2C ---
// Display 1: Nutrientes e Status (Endereço 0x27)
LiquidCrystal_I2C lcd_nutrientes(0x27, 20, 4); 

// Display 2: Dados do Clima (Endereço 0x3F - *** VERIFIQUE O ENDEREÇO DO SEU SEGUNDO MÓDULO! ***)
LiquidCrystal_I2C lcd_clima(0x3F, 20, 4); 

// --- Configurações dos Componentes ---
#define DHTPIN 25
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// --- Pinos dos Componentes ---
const int pinoFosforo = 19;
const int pinoPotassio = 23;
const int pinoNitrogenio = 18;
const int pinoLDR = 34;
const int pinoRele = 26;

// --- Variáveis Globais de Estado (Nutrientes e Sensores) ---
long valorN = 0, valorP = 0, valorK = 0; 
float valorPH = 7.0;
float valorUmidade = 0.0; // Umidade do Solo/Ambiente
bool bombaLigada = false;

int estadoFosforoAnterior = HIGH, estadoPotassioAnterior = HIGH, estadoNitrogenioAnterior = HIGH;

// --- Parâmetros da Cultura (Exemplo: Tomate) ---
const float UMIDADE_MINIMA = 50.0;
const float PH_IDEAL_MIN = 6.0;
const float PH_IDEAL_MAX = 6.8;
const int N_IDEAL_MIN = 15;
const int N_IDEAL_MAX = 35;
const int P_IDEAL_MIN = 8;
const int P_IDEAL_MAX = 25;
const int K_IDEAL_MIN = 35;
const int K_IDEAL_MAX = 90;

// ------------------- CONFIGURAÇÕES API E VARIÁVEIS GLOBAIS DE CLIMA -------------------
const char* ssid = "Wokwi-GUEST";
const char* password = "";

String apiKey = "bb7a21b166a22b56f4dabde2071925b3"; 
String cidadeCodificada = "Sao%20Paulo,br"; 
String openWeatherMapUrl = "http://api.openweathermap.org/data/2.5/weather?q=" + cidadeCodificada + "&appid=" + apiKey + "&units=metric&lang=pt_br";

// VARIÁVEIS GLOBAIS PARA O CLIMA
String nomeCidadeGlobal = "Carregando..."; 
float temperaturaGlobal = 0.0;
float sensacaoTermicaGlobal = 0.0;
String descricaoClimaGlobal = "Buscando...";
float velocidadeVentoGlobal = 0.0;
float umidadeArGlobal = 0.0; // Umidade do ar

// -----------------------------------------------------

// Função para buscar e salvar o clima (executada apenas 1 vez no setup)
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
        // Extrai e salva os dados globalmente
        nomeCidadeGlobal = String(doc["name"].as<const char*>());
        temperaturaGlobal = doc["main"]["temp"];
        sensacaoTermicaGlobal = doc["main"]["feels_like"];
        umidadeArGlobal = doc["main"]["humidity"]; // Umidade do ar
        descricaoClimaGlobal = String(doc["weather"][0]["description"].as<const char*>());
        velocidadeVentoGlobal = (float)doc["wind"]["speed"] * 3.6; // m/s para km/h
        
        Serial.println(" OK.");

      } else {
        Serial.print("Erro JSON: ");
        Serial.println(error.c_str());
        descricaoClimaGlobal = "Erro JSON";
      }
    } else {
      Serial.print("Erro HTTP: ");
      Serial.println(httpResponseCode);
      descricaoClimaGlobal = "Erro HTTP";
    }
    http.end();
  } else {
    Serial.println(" Wi-Fi Desconectado. Nao foi possivel buscar o clima.");
    descricaoClimaGlobal = "Wi-Fi OFF";
  }
}

// ===================================================================
// FUNÇÃO PARA ATUALIZAR O DISPLAY 1: Nutrientes e Status (SÓ SENSORES)
// ===================================================================
void atualizarDisplayNutrientes() {
  lcd_nutrientes.clear();
  
  // Linha 1: Título
  lcd_nutrientes.setCursor(0, 0);
  lcd_nutrientes.print("--- SENSORES/SOLO ---");
  
  // Linha 2: Nutrientes
  lcd_nutrientes.setCursor(0, 1);
  lcd_nutrientes.print("N:" + String(valorN) + " P:" + String(valorP) + " K:" + String(valorK));
  
  // Linha 3: Condições do Solo
  lcd_nutrientes.setCursor(0, 2);
  lcd_nutrientes.print("Solo(%) " + String(valorUmidade, 0));
  lcd_nutrientes.setCursor(13, 2);
  lcd_nutrientes.print("pH " + String(valorPH, 1));
  
  // Linha 4: Status do Sistema
  lcd_nutrientes.setCursor(0, 3);
  lcd_nutrientes.print("Bomba: ");
  lcd_nutrientes.print(bombaLigada ? "LIGADA" : "DESLIGADA");
}

// ===================================================================
// FUNÇÃO PARA ATUALIZAR O DISPLAY 2: Dados do Clima (SÓ API)
// ===================================================================
void atualizarDisplayClima() {
  lcd_clima.clear();
  
  // Linha 1: Cidade e Condição
  lcd_clima.setCursor(0, 0);
  lcd_clima.print(nomeCidadeGlobal.substring(0, 20)); // Cidade
  
  // Linha 2: Temperatura e Sensação
  lcd_clima.setCursor(0, 1);
  lcd_clima.print("Temp: " + String(temperaturaGlobal, 1) + (char)223 + "C"); // (char)223 é o símbolo de grau
  lcd_clima.setCursor(10, 1);
  lcd_clima.print("Sens: " + String(sensacaoTermicaGlobal, 1) + (char)223 + "C");
  
  // Linha 3: Umidade do Ar e Vento
  lcd_clima.setCursor(0, 2);
  lcd_clima.print("Ar(%) " + String(umidadeArGlobal, 0));
  lcd_clima.setCursor(10, 2);
  lcd_clima.print("Vento: " + String(velocidadeVentoGlobal, 1) + "km/h");

  // Linha 4: Descrição do Clima
  lcd_clima.setCursor(0, 3);
  lcd_clima.print(descricaoClimaGlobal.substring(0, 20)); 
}


// Função com a Lógica de Decisão para Irrigação
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
  
  // Inicializa os DOIS displays
  lcd_nutrientes.init();
  lcd_nutrientes.backlight();
  lcd_clima.init();
  lcd_clima.backlight();
  
  // Mensagem inicial de espera
  lcd_nutrientes.setCursor(0, 0);
  lcd_nutrientes.print("--- SMART FARM ---");
  lcd_nutrientes.setCursor(0, 1);
  lcd_nutrientes.print("Iniciando Sensores...");
  
  lcd_clima.setCursor(0, 0);
  lcd_clima.print("--- CLIMA API ---");
  lcd_clima.setCursor(0, 1);
  lcd_clima.print("Aguardando Wi-Fi...");
  
  dht.begin();
  randomSeed(analogRead(pinoLDR)); 
  
  pinMode(pinoRele, OUTPUT);
  digitalWrite(pinoRele, LOW);
  
  pinMode(pinoFosforo, INPUT_PULLUP);
  pinMode(pinoPotassio, INPUT_PULLUP);
  pinMode(pinoNitrogenio, INPUT_PULLUP);

  // Conecta ao Wi-Fi e espera a conexão
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
      delay(500);
      Serial.print(".");
      tentativas++;
  }
  
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println(" Conectado!");
    // EXECUTA O CLIMA APENAS UMA VEZ
    atualizarClima();
  } else {
    Serial.println(" Falha na conexao Wi-Fi.");
  }

  // Atualiza os displays com os dados iniciais
  atualizarDisplayNutrientes();
  atualizarDisplayClima(); // Garante que o display de clima mostre os dados da API
}

void loop() {
  
  bool algoMudou = false;
  bool estadoBombaAntes = bombaLigada;

  // Leitura dos Sensores (a cada loop)
  float novoPH = (analogRead(pinoLDR) / 4095.0) * 14.0; 
  if (abs(novoPH - valorPH) > 0.1) {
    valorPH = novoPH;
    algoMudou = true;
  }

  // Leitura do DHT (Umidade do Solo/Ambiente)
  float novaUmidade = dht.readHumidity();
  if (!isnan(novaUmidade) && abs(novaUmidade - valorUmidade) > 1.0) {
    valorUmidade = novaUmidade;
    algoMudou = true;
  }
  
  // Lógica dos "Botões" para simulação de nutrientes
  if (digitalRead(pinoFosforo) == LOW && estadoFosforoAnterior == HIGH) {
    valorP = random(P_IDEAL_MIN, P_IDEAL_MAX + 1); 
    algoMudou = true;
    delay(50);
  }
  estadoFosforoAnterior = digitalRead(pinoFosforo);

  if (digitalRead(pinoPotassio) == LOW && estadoPotassioAnterior == HIGH) {
    valorK = random(K_IDEAL_MIN, K_IDEAL_MAX + 1);
    algoMudou = true;
    delay(50);
  }
  estadoPotassioAnterior = digitalRead(pinoPotassio);

  if (digitalRead(pinoNitrogenio) == LOW && estadoNitrogenioAnterior == HIGH) {
    valorN = random(N_IDEAL_MIN, N_IDEAL_MAX + 1);
    algoMudou = true;
    delay(50);
  }
  estadoNitrogenioAnterior = digitalRead(pinoNitrogenio);
  
  // Tomada de Decisão
  verificarIrrigacao();

  // Atualização do Display de Nutrientes (somente se houver mudança nos sensores ou na bomba)
  if (algoMudou || (bombaLigada != estadoBombaAntes)) {
    atualizarDisplayNutrientes();
  }
  
  delay(100);
}