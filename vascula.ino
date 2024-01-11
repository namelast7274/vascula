#include "HX711.h"
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

byte DT = 3;
byte CLK = 2;
byte modo = 7;
byte tara = 6;
int peso_conocido[4] = {500, 1000, 3000, 5000};
long escala;

LiquidCrystal_I2C lcd(0x27, 16, 2);
HX711 balanza;

void anti_debounce(byte boton) {
  delay(100);
  while (digitalRead(boton)); // Anti-Rebote
  delay(100);
}

float precio_por_g = 0.015;  // Tasa de precio por gramo (ajústala según tus necesidades)

float calcularGanancias(float peso_g) {
  return peso_g * precio_por_g;
}

int modoContador = 0;
bool modoSecuencia = false;

void calibration() {
  int i = 0, cal = 1;
  long adc_lecture;

  lcd.setCursor(2, 0);
  lcd.print("Calibracion de");
  lcd.setCursor(4, 1);
  lcd.print("Balanza");
  delay(1500);
  balanza.read();
  balanza.set_scale();
  balanza.tare(20);

  lcd.clear();

  while (cal == 1) {
    lcd.setCursor(1, 0);
    lcd.print("Peso Conocido:");
    lcd.setCursor(1, 1);
    lcd.print(peso_conocido[i]);
    lcd.print(" g        ");

    if (digitalRead(tara)) {
      anti_debounce(tara);
      i = (i > 2) ? 0 : i + 1;
    }

    if (digitalRead(modo)) {
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("Ponga el Peso");
      lcd.setCursor(1, 1);
      lcd.print("y espere ...");
      delay(2000);

      adc_lecture = balanza.get_value(100);
      escala = adc_lecture / peso_conocido[i];

      EEPROM.put(0, escala);
      delay(100);
      cal = 0;
      lcd.clear();
    }
  }
}

void setup() {
  balanza.begin(DT, CLK);
  pinMode(modo, INPUT);
  pinMode(tara, INPUT);
  lcd.init();
  lcd.backlight();
  EEPROM.get(0, escala);

  if (digitalRead(modo) && digitalRead(tara))
    calibration();

  lcd.setCursor(1, 0);
  lcd.print("Retire el Peso");
  lcd.setCursor(1, 1);
  lcd.print("y Espere");
  delay(2000);
  balanza.set_scale(escala);
  balanza.tare(20);

  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Listo....");
  delay(1000);
  lcd.clear();
}

void loop() {
  float peso_g, ganancias;

  peso_g = balanza.get_units(10);

  if (digitalRead(modo)) {
    anti_debounce(modo);
    modoContador++;

    if (modoContador == 3) {
      modoSecuencia = !modoSecuencia;
      modoContador = 0;
    }
  }

  lcd.setCursor(1, 0);
  lcd.print("Peso: ");
  lcd.print(peso_g, 0);  // Mostrar el peso en gramos sin decimales
  lcd.setCursor(15,0);
  lcd.print("g");
  lcd.setCursor(1, 1);

  ganancias = calcularGanancias(peso_g);
  lcd.print("Ganancias: $");
  lcd.print(ganancias, 2);

  delay(5);

  if (digitalRead(tara)) {
    anti_debounce(tara);
    balanza.tare(10);
  }
}
