#include <LiquidCrystal.h>
#include <avr/pgmspace.h>

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

#define DO4   262
#define RE4   294
#define MI4   330
#define FA4   349
#define SOL4  392
#define LA4   440

const int totalMusicas = 5;
String nomesMusicas[totalMusicas] = {
  "Cai Cai Balao",
  "Brilha Brilha",
  "Dona Aranha",
  "Tema do Gas",
  "Ciranda"
};

int musicaSelecionada = 0;

const unsigned int notasCaiCaiBalao[] PROGMEM = { SOL4, SOL4, LA4, SOL4, MI4, SOL4, LA4, SOL4, MI4, RE4, MI4, DO4 };
const byte temposCaiCaiBalao[] PROGMEM = { 4, 4, 4, 4, 2, 4, 4, 4, 2, 4, 4, 2 };

int notaAtualIndex = 0;
int totalNotasDaMusica = sizeof(notasCaiCaiBalao) / sizeof(notasCaiCaiBalao[0]);
unsigned int frequenciaAtual = 0;

unsigned long inicioNotaMs = 0;
unsigned long duracaoNotaMs = 0;
unsigned long restoNotaMs = 0;

int leituraAnteriorCima = HIGH;
int leituraAnteriorBaixo = HIGH;
int leituraAnteriorPlay = HIGH;
int leituraAnteriorStop = HIGH;

unsigned long ultimoCliqueCima = 0;
unsigned long ultimoCliqueBaixo = 0;
unsigned long ultimoCliquePlay = 0;
unsigned long ultimoCliqueStop = 0;

const unsigned long debounceMs = 180;

unsigned int lerNotaAtual(int posicao) {
  return pgm_read_word(&notasCaiCaiBalao[posicao]);
}

byte lerTempoAtual(int posicao) {
  return pgm_read_byte(&temposCaiCaiBalao[posicao]);
}

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

  if (estadoAtual == MENU) {
    lcd.setCursor(0, 0);
    lcd.print("Selecione:");
    lcd.setCursor(0, 1);
    lcd.print(nomesMusicas[musicaSelecionada]);
  } else if (estadoAtual == TOCANDO) {
    lcd.setCursor(0, 0);
    lcd.print("Tocando:");
    lcd.setCursor(0, 1);
    lcd.print(nomesMusicas[musicaSelecionada]);
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Pausado:");
    lcd.setCursor(0, 1);
    lcd.print(nomesMusicas[musicaSelecionada]);
  }
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

void voltarParaMenu() {
  estadoAtual = MENU;
  noTone(PINO_BUZZER);
  notaAtualIndex = 0;
  frequenciaAtual = 0;
  duracaoNotaMs = 0;
  restoNotaMs = 0;
  atualizarLeds();
  atualizarTela();
}

void tocarNotaAtual() {
  if (notaAtualIndex >= totalNotasDaMusica) {
    voltarParaMenu();
    return;
  }

  frequenciaAtual = lerNotaAtual(notaAtualIndex);
  byte divisorTempo = lerTempoAtual(notaAtualIndex);

  duracaoNotaMs = 1000UL / divisorTempo;
  restoNotaMs = duracaoNotaMs;

  if (frequenciaAtual == 0) noTone(PINO_BUZZER);
  else tone(PINO_BUZZER, frequenciaAtual);

  inicioNotaMs = millis();
}

void iniciarMusica() {
  estadoAtual = TOCANDO;
  notaAtualIndex = 0;
  atualizarLeds();
  atualizarTela();
  tocarNotaAtual();
}

void pausarMusica() {
  unsigned long tempoGasto = millis() - inicioNotaMs;
  if (tempoGasto < duracaoNotaMs) restoNotaMs = duracaoNotaMs - tempoGasto;
  else restoNotaMs = 1;

  estadoAtual = PAUSADO;
  noTone(PINO_BUZZER);
  atualizarLeds();
  atualizarTela();
}

void retomarMusica() {
  estadoAtual = TOCANDO;
  duracaoNotaMs = restoNotaMs;

  if (frequenciaAtual == 0) noTone(PINO_BUZZER);
  else tone(PINO_BUZZER, frequenciaAtual);

  inicioNotaMs = millis();
  atualizarLeds();
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
  int leituraPlay = digitalRead(BOTAO_PLAY);
  int leituraStop = digitalRead(BOTAO_STOP);

  unsigned long agora = millis();

  if (leituraAnteriorCima == HIGH && leituraCima == LOW && (agora - ultimoCliqueCima > debounceMs)) {
    ultimoCliqueCima = agora;
    if (estadoAtual == MENU) proximaMusica();
  }

  if (leituraAnteriorBaixo == HIGH && leituraBaixo == LOW && (agora - ultimoCliqueBaixo > debounceMs)) {
    ultimoCliqueBaixo = agora;
    if (estadoAtual == MENU) musicaAnterior();
  }

  if (leituraAnteriorPlay == HIGH && leituraPlay == LOW && (agora - ultimoCliquePlay > debounceMs)) {
    ultimoCliquePlay = agora;

    if (estadoAtual == MENU) iniciarMusica();
    else if (estadoAtual == TOCANDO) pausarMusica();
    else if (estadoAtual == PAUSADO) retomarMusica();
  }

  if (leituraAnteriorStop == HIGH && leituraStop == LOW && (agora - ultimoCliqueStop > debounceMs)) {
    ultimoCliqueStop = agora;
    if (estadoAtual == TOCANDO || estadoAtual == PAUSADO) voltarParaMenu();
  }

  if (estadoAtual == TOCANDO) {
    if (millis() - inicioNotaMs >= duracaoNotaMs) {
      notaAtualIndex++;
      tocarNotaAtual();
    }
  }

  leituraAnteriorCima = leituraCima;
  leituraAnteriorBaixo = leituraBaixo;
  leituraAnteriorPlay = leituraPlay;
  leituraAnteriorStop = leituraStop;
}