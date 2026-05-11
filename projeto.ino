#include <LiquidCrystal.h>

LiquidCrystal lcd(9, 10, 11, 12, 13, A0);

#define BOTAO_CIMA   2
#define BOTAO_BAIXO  3
#define BOTAO_PLAY   4
#define BOTAO_STOP   5
#define PINO_BUZZER  6
#define LED_TOCANDO  7
#define LED_PAUSADO  8

enum EstadoSistema { MENU, TOCANDO, PAUSADO };
EstadoSistema estadoAtual = MENU;

const int totalMusicas = 5;
String nomesMusicas[totalMusicas] = {
  "Cai Cai Balao",
  "Brilha Brilha",
  "Dona Aranha",
  "Tema do Gas",
  "Ciranda"
};

int musicaSelecionada = 0;

int leituraAnteriorCima = HIGH;
int leituraAnteriorBaixo = HIGH;

unsigned long ultimoCliqueCima = 0;
unsigned long ultimoCliqueBaixo = 0;

const unsigned long debounceMs = 180;

void atualizarLeds() {
  if (estadoAtual == TOCANDO) {
    digitalWrite(LED_TOCANDO, HIGH);
    digitalWrite(LED_PAUSADO, LOW);
  } else if (estadoAtual == PAUSADO) {
    digitalWrite(LED_TOCANDO, LOW);
    digitalWrite(LED_PAUSADO, HIGH);
  } else {
    digitalWrite(LED_TOCANDO, LOW);
    digitalWrite(LED_PAUSADO, LOW);
  }
}

void atualizarTela() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Selecione:");
  lcd.setCursor(0, 1);
  lcd.print(nomesMusicas[musicaSelecionada]);
}

void proximaMusica() {
  musicaSelecionada++;
  if (musicaSelecionada >= totalMusicas) musicaSelecionada = 0;
  atualizarTela();
}

void musicaAnterior() {
  musicaSelecionada--;
  if (musicaSelecionada < 0) musicaSelecionada = totalMusicas - 1;
  atualizarTela();
}

void setup() {
  lcd.begin(16, 2);

  pinMode(BOTAO_CIMA, INPUT_PULLUP);
  pinMode(BOTAO_BAIXO, INPUT_PULLUP);
  pinMode(BOTAO_PLAY, INPUT_PULLUP);
  pinMode(BOTAO_STOP, INPUT_PULLUP);

  pinMode(PINO_BUZZER, OUTPUT);
  pinMode(LED_TOCANDO, OUTPUT);
  pinMode(LED_PAUSADO, OUTPUT);

  atualizarLeds();
  atualizarTela();
}

void loop() {
  int leituraCima = digitalRead(BOTAO_CIMA);
  int leituraBaixo = digitalRead(BOTAO_BAIXO);
  unsigned long agora = millis();

  if (leituraAnteriorCima == HIGH && leituraCima == LOW && (agora - ultimoCliqueCima > debounceMs)) {
    ultimoCliqueCima = agora;
    if (estadoAtual == MENU) proximaMusica();
  }

  if (leituraAnteriorBaixo == HIGH && leituraBaixo == LOW && (agora - ultimoCliqueBaixo > debounceMs)) {
    ultimoCliqueBaixo = agora;
    if (estadoAtual == MENU) musicaAnterior();
  }

  leituraAnteriorCima = leituraCima;
  leituraAnteriorBaixo = leituraBaixo;
}