#include <LiquidCrystal.h>

LiquidCrystal lcd(9, 10, 11, 12, 13, A0);

void setup() {
  lcd.begin(16, 2);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Teste LCD");

  lcd.setCursor(0, 1);
  lcd.print("FUNCIONANDO");
}

void loop() {
}