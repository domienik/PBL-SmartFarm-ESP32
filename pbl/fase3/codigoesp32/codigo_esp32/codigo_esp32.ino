// Código inicial que lê um botão e gera um número aleatório no Monitor Serial
const int pinoBotao = 22;
int estadoBotaoAnterior = HIGH;

void setup() {
  Serial.begin(115200);
  pinMode(pinoBotao, INPUT_PULLUP);
  randomSeed(analogRead(A0));
  Serial.println("Pressione o botao para gerar um numero.");
}

void loop() {
  if (digitalRead(pinoBotao) == LOW && estadoBotaoAnterior == HIGH) {
    long numero = random(1, 101);
    Serial.print("Numero gerado: ");
    Serial.println(numero);
    delay(50);
  }
  estadoBotaoAnterior = digitalRead(pinoBotao);
}