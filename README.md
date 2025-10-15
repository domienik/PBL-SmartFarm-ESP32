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


## 📁 Estrutura do Repositório

* `sketch.ino`: O código principal do projeto para o ESP32.
* `diagram.json`: Arquivo de configuração do Wokwi que descreve o circuito elétrico.
* `libraries.txt`: Lista as bibliotecas Arduino necessárias para o projeto.
* `.gitignore`: Especifica os arquivos que não devem ser enviados para o repositório (como `secrets.h`).

## Vídeo de Demonstração 📽

O funcionamento completo do projeto pode ser visto no vídeo abaixo:

*COLE AQUI O LINK DO SEU VÍDEO NO YOUTUBE*