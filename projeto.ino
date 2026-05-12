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
#define SI4   494
#define DO5   523

const int totalMusicas = 5;
char tituloMusica[17];
int musicaSelecionada = 0;

const unsigned int notasCaiCaiBalao[] PROGMEM = { SOL4, SOL4, LA4, SOL4, MI4, SOL4, LA4, SOL4, MI4, RE4, MI4, DO4 };
const byte temposCaiCaiBalao[] PROGMEM = { 4, 4, 4, 4, 2, 4, 4, 4, 2, 4, 4, 2 };

const unsigned int notasBrilhaBrilha[] PROGMEM = { DO4, DO4, SOL4, SOL4, LA4, LA4, SOL4, FA4, FA4, MI4, MI4, RE4, RE4, DO4 };
const byte temposBrilhaBrilha[] PROGMEM = { 4, 4, 4, 4, 4, 4, 2, 4, 4, 4, 4, 4, 4, 2 };

const unsigned int notasDonaAranha[] PROGMEM = { DO4, RE4, MI4, DO4, DO4, RE4, MI4, DO4, MI4, FA4, SOL4, MI4, FA4, SOL4 };
const byte temposDonaAranha[] PROGMEM = { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2, 4, 4, 2 };

const unsigned int notasTemaDoGas[] PROGMEM = { SOL4, MI4, SOL4, DO5, SI4, SOL4, LA4, SI4, SOL4 };
const byte temposTemaDoGas[] PROGMEM = { 8, 8, 8, 4, 8, 8, 8, 4, 2 };

const unsigned int notasCiranda[] PROGMEM = { SOL4, LA4, SOL4, FA4, MI4, FA4, SOL4, MI4, DO4, RE4, MI4, DO4 };
const byte temposCiranda[] PROGMEM = { 4, 4, 4, 4, 2, 4, 4, 4, 4, 4, 4, 2 };

int notaAtualIndex = 0;
int totalNotasDaMusica = 0;
int totalUnidadesMusica = 0;
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

void carregarTituloMusica(int indice, char *destino) {
  switch (indice) {
    case 0: strcpy_P(destino, PSTR("Cai Cai Balao")); break;
    case 1: strcpy_P(destino, PSTR("Brilha Brilha")); break;
    case 2: strcpy_P(destino, PSTR("Dona Aranha")); break;
    case 3: strcpy_P(destino, PSTR("Tema do Gas")); break;
    case 4: strcpy_P(destino, PSTR("Ciranda")); break;
    default: strcpy_P(destino, PSTR("Sem titulo")); break;
  }
}

int contarNotasMusica(int indice) {
  switch (indice) {
    case 0: return sizeof(notasCaiCaiBalao) / sizeof(notasCaiCaiBalao[0]);
    case 1: return sizeof(notasBrilhaBrilha) / sizeof(notasBrilhaBrilha[0]);
    case 2: return sizeof(notasDonaAranha) / sizeof(notasDonaAranha[0]);
    case 3: return sizeof(notasTemaDoGas) / sizeof(notasTemaDoGas[0]);
    case 4: return sizeof(notasCiranda) / sizeof(notasCiranda[0]);
    default: return 0;
  }
}

unsigned int lerNotaMusica(int indice, int posicao) {
  switch (indice) {
    case 0: return pgm_read_word(&notasCaiCaiBalao[posicao]);
    case 1: return pgm_read_word(&notasBrilhaBrilha[posicao]);
    case 2: return pgm_read_word(&notasDonaAranha[posicao]);
    case 3: return pgm_read_word(&notasTemaDoGas[posicao]);
    case 4: return pgm_read_word(&notasCiranda[posicao]);
    default: return 0;
  }
}

byte lerTempoMusica(int indice, int posicao) {
  switch (indice) {
    case 0: return pgm_read_byte(&temposCaiCaiBalao[posicao]);
    case 1: return pgm_read_byte(&temposBrilhaBrilha[posicao]);
    case 2: return pgm_read_byte(&temposDonaAranha[posicao]);
    case 3: return pgm_read_byte(&temposTemaDoGas[posicao]);
    case 4: return pgm_read_byte(&temposCiranda[posicao]);
    default: return 4;
  }
}

int contarUnidadesMusica(int indice) {
  int soma = 0;
  int totalNotas = contarNotasMusica(indice);

  for (int i = 0; i < totalNotas; i++) {
    byte divisor = lerTempoMusica(indice, i);
    soma += 8 / divisor;
  }

  return soma;
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
  carregarTituloMusica(musicaSelecionada, tituloMusica);

  lcd.clear();

  if (estadoAtual == MENU) {
    lcd.setCursor(0, 0);
    lcd.print("Selecione:");
    lcd.setCursor(0, 1);
    lcd.print(tituloMusica);
  } else if (estadoAtual == TOCANDO) {
    lcd.setCursor(0, 0);
    lcd.print("Tocando:");
    lcd.setCursor(0, 1);
    lcd.print(tituloMusica);
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Pausado:");
    lcd.setCursor(0, 1);
    lcd.print(tituloMusica);
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
  totalNotasDaMusica = 0;
  totalUnidadesMusica = 0;
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

  frequenciaAtual = lerNotaMusica(musicaSelecionada, notaAtualIndex);
  byte divisorTempo = lerTempoMusica(musicaSelecionada, notaAtualIndex);

  int unidadesDaNota = 8 / divisorTempo;
  duracaoNotaMs = (5000UL * unidadesDaNota) / totalUnidadesMusica;

  if (duracaoNotaMs < 1) duracaoNotaMs = 1;

  restoNotaMs = duracaoNotaMs;

  if (frequenciaAtual == 0) noTone(PINO_BUZZER);
  else tone(PINO_BUZZER, frequenciaAtual);

  inicioNotaMs = millis();
}

void iniciarMusica() {
  estadoAtual = TOCANDO;
  notaAtualIndex = 0;
  totalNotasDaMusica = contarNotasMusica(musicaSelecionada);
  totalUnidadesMusica = contarUnidadesMusica(musicaSelecionada);

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