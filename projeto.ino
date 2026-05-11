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

void atualizarTelaInicial() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Projeto Musica");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");
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
  atualizarTelaInicial();
}

void loop() {
}