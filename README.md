# Projeto FarmTech Solutions - Fase 3: Irrigação Inteligente

Este projeto, desenvolvido para a disciplina de TIAOR, simula um sistema de irrigação inteligente para a cultura de Tomate, utilizando um microcontrolador ESP32 e diversos sensores simulados na plataforma Wokwi.com.

## Descrição do Funcionamento

O sistema monitora em tempo real em um display de `led (wokwi-lcd2004)` cinco parâmetros cruciais para a saúde da lavoura:


- **Níveis de Nutrientes (NPK):** Simulados por três botões que geram valores em PPM.

![botoes](./assets/botoes.png)

Usammos a técnica do `PULLUP` para evitar ruidos aos botões, então eles tem a pinagem de saida sempre conectada ao `GND`, já a pinagem de cada um está configurada como:

```cpp
// --- Pinos dos Componentes ---
const int pinoFosforo = 19;
const int pinoPotassio = 23;
const int pinoNitrogenio = 18;
....
```

Ao pressionar cada botão, adicionaremos ao lcd (display) o valores de cada nutriente. É importante ressaltar que a bomba será ligada apenas se os três botões mostrarem seus valores.

![botoes](./assets/lcdBotoes.png)


- **pH do Solo:** Simulado por um sensor LDR, mapeado para a escala de 0 a 14.

Sua pinagem está em:

```
const int pinoLDR = 34;
```

![ldr](./assets/ldr.png)


- **Umidade do Solo:** Simulada por um sensor DHT22.

Usaremos apenas a funcionalidade `Humidity` para configurar a simulaçao dos níveis de umidade.

![ldr](./assets/DHT.png)

```c++
// --- Configurações dos Componentes ---
#define DHTPIN 25
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
```

A lógica de irrigação "inteligente" só aciona a bomba d'água (representada por um relé) quando **todas as condições ideais** para a cultura de tomate são atendidas simultaneamente, otimizando o uso de água.

![ldr](./assets/bomba.png)

## 🔌 Integração com a API OpenWeatherMap

Para enriquecer os dados dos sensores locais, o sistema busca informações de clima em tempo real na cidade de São Paulo através da API OpenWeatherMap.

![api](./assets/api.png)

O fluxo de funcionamento é o seguinte:

1.  **Conexão Wi-Fi:** Ao iniciar, o ESP32 se conecta à rede Wi-Fi configurada no arquivo `secrets.h`.
2.  **Requisição HTTP:** Uma requisição `GET` é enviada para a URL da API, que inclui a cidade, a chave de API e parâmetros para unidades (métrica) e língua (português).
3.  **Decodificação (Parsing) do JSON:** A resposta da API, que vem em formato de texto JSON, é decodificada pela biblioteca `ArduinoJson` para extrair os dados relevantes.
4.  **Exibição dos Dados:** Informações como temperatura, sensação térmica, umidade do ar e descrição do clima são salvas em variáveis globais e exibidas no segundo display LCD.

## ☁️ Armazenamento em Nuvem para Análise de Dados em R

Para viabilizar a análise de dados e o treinamento contínuo do modelo preditivo em R, o sistema está configurado para **transferir as leituras dos sensores para um serviço de armazenamento de dados na nuvem** (utilizando a plataforma ThingSpeak).

![dados](./assets/dados.png)

ste processo cria um *dataset* histórico em formato de série temporal, que pode ser posteriormente exportado e utilizado diretamente no ambiente R para:

* Validar a eficácia do modelo preditivo (`modelo_bomba.rds`).
* Retreinar o modelo com novos dados para melhorar sua precisão.
* Realizar análises exploratórias para descobrir novas correlações entre as variáveis.


* Nível de pH do solo
* Umidade do solo (%)
* Níveis de N, P e K
* Status da bomba de irrigação (Ligada/Desligada)

![apisalvo](./assets/apisalvo.png)

Os dados são enviados periodicamente para a plataforma, permitindo o monitoramento remoto do sistema.

## 📊 Análise de Dados e Modelo Preditivo em R

Para evoluir de uma lógica de irrigação baseada em regras fixas para um sistema verdadeiramente inteligente, foi conduzida uma análise de dados utilizando a linguagem R. O objetivo foi criar um modelo de Machine Learning que servisse como um "cérebro" analítico para as decisões de irrigação, validando e aprimorando as regras implementadas no ESP32.

### Metodologia

O fluxo de trabalho seguiu as etapas padrão de um projeto de ciência de dados:

1.  **Coleta e Preparação:** Utilizou-se um dataset (`dados_sensores.csv`) contendo leituras históricas das variáveis do solo (NPK, pH, umidade) e o status da bomba. Os dados foram limpos e preparados para a modelagem.

2.  **Análise Exploratória e Modelagem:** O ambiente R foi utilizado para explorar os dados, identificar correlações entre as variáveis e, por fim, treinar um modelo de **Regressão Logística**. Este modelo foi escolhido por sua capacidade de prever um resultado binário (Bomba `LIGADA` ou `DESLIGADA`) com base nas condições dos sensores.

3.  **Exportação do Modelo:** O modelo preditivo treinado foi serializado e salvo no arquivo `modelo_bomba.rds`, permitindo que sua inteligência seja reutilizada no futuro.

### Artefatos da Análise

A pasta `analise_R/` contém todos os artefatos gerados durante este processo:

* **`dados_sensores.csv`**: O conjunto de dados (dataset) limpo, utilizado para treinar e validar o modelo.
* **`modelo_bomba.rds`**: O entregável mais importante da análise. É um objeto R que contém o modelo de Machine Learning treinado, pronto para ser carregado e fazer novas previsões.
* **`.RData` e `.Rhistory`**: Arquivos de trabalho do ambiente R. Contêm o workspace (variáveis) e o histórico de comandos, garantindo a auditoria e a reprodutibilidade do estudo.

### Conexão com o Projeto Principal

A inteligência gerada por esta análise possui duas conexões vitais com o projeto:

1.  **Validação da Lógica:** As faixas de valores ideais (pH, umidade, NPK) implementadas na função `verificarIrrigacao()` do ESP32 foram **validadas** por esta análise de dados, confirmando que as regras fixas são baseadas em correlações estatisticamente relevantes.

2.  **Ciclo de Melhoria Contínua:** Os novos dados que o sistema coleta e envia para a nuvem (ThingSpeak) podem ser usados para alimentar e **retreinar o modelo em R periodicamente**. Isso cria um ciclo virtuoso onde o sistema se torna cada vez mais inteligente e adaptado às condições específicas da lavoura ao longo do tempo.


## 📁 Estrutura do Repositório

```
/
├── 📂 datascience/
│   ├── 📄 dados_sensores.csv
│   ├── 📄 modelo_bomba.rds
│   ├── 📄 RData.R
│   └── ...
│
├── 📂 assets/
│   └── 🖼️ diagrama_circuito.png
│
├── 📄 sketch.ino
├── 📄 diagram.json
├── 📄 library.txt
├── 📄 secrets.h
└── 📄 README.md

```

### Descrição dos Arquivos e Pastas

* **`sketch.ino`**: O código-fonte principal em C++ desenvolvido para o microcontrolador ESP32. Contém toda a lógica de leitura dos sensores, controle do relé e comunicação com as APIs.
* **`diagram.json`**: Arquivo de configuração da plataforma Wokwi. Descreve todos os componentes do circuito (ESP32, sensores, displays, etc.) e como eles estão conectados eletricamente na simulação.
* **`library.txt`**: Um arquivo específico do Wokwi que lista as bibliotecas de terceiros (`LiquidCrystal_I2C`, `DHT sensor library`, `ArduinoJson`) necessárias para compilar e executar o projeto corretamente.
* **`secrets.h`**: Um arquivo local (ignorado pelo Git) onde são armazenadas informações sensíveis, como as chaves das APIs (OpenWeather, ThingSpeak) e credenciais de Wi-Fi, mantendo-as seguras e fora do controle de versão.
* **`README.md`**: Este arquivo de documentação, que explica todo o projeto, sua lógica, funcionamento e como executá-lo.
* **`📂 datascience/`**: Este diretório contém todos os artefatos do processo de análise de dados e criação do modelo preditivo em R.
    * **`dados_sensores.csv`**: O conjunto de dados (dataset) utilizado para a análise.
    * **`modelo_bomba.rds`**: O modelo de Machine Learning treinado e exportado, pronto para ser reutilizado.
    * **`RData.R`**: O script com o código em R utilizado para limpar os dados, realizar a análise e treinar o modelo.
* **`📂 assets/`**: Este diretório armazena arquivos de mídia utilizados na documentação, como imagens e esquemas do circuito.

## Vídeo de Demonstração 📽

O funcionamento completo do projeto pode ser visto no vídeo abaixo:

https://youtu.be/ZCE25_D37qg