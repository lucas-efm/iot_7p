#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int IRemitterPin = 34;  // Pino D34 para o emissor IR
const int buttonPin18 = 5;  // Pino do botão D18
const int buttonPin19 = 18;  // Pino do botão D19
const int buttonPin21 = 19;  // Pino do botão D21
const int relayPin = 23;      // Pino do relé
const int receiverPin = 25;  // Pino D22 para o receptor IR
bool projectorOn = false;
bool soundbarOn = false;
int lightingMode = 0;  // 0: Desligado, 1: Modo 1, 2: Modo 2
int button21Clicks = 0;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 500;

IRsend irsend(IRemitterPin);
IRrecv irrecv(receiverPin);
decode_results results;

LiquidCrystal_I2C lcd(0x27, 16, 2); // Substitua pelo endereço I2C correto do seu display

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin18, INPUT);
  pinMode(buttonPin19, INPUT);
  pinMode(buttonPin21, INPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);  // Inicialmente, o relé está desligado
  irsend.begin();  // Inicializa o emissor IR
  irrecv.enableIRIn();  // Inicializa o receptor IR
  Wire.begin();
  lcd.begin(16, 2); // Inicialize o display com 16 co
  lcd.setBacklight(LOW); // Desligue o backlight
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Aguardando sinal IR...");
}

void printToSerialAndLCD(const char* message) {
  // Imprime na porta serial
  Serial.println(message);

  // Imprime no display LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message);
}

void loop() {
  // Botão D18
  if (digitalRead(buttonPin18) == HIGH && (millis() - lastDebounceTime) > debounceDelay) {
    if (projectorOn) {
      printToSerialAndLCD("Desligando Projetor");
      irsend.sendNEC(0xE4780854, 32); // Substitua o código com o código IR correto
      projectorOn = false;
    } else {
      printToSerialAndLCD("Ligando Projetor");
      irsend.sendNEC(0xE1BA5140, 32); // Substitua o código com o código IR correto
      projectorOn = true;
    }
    lastDebounceTime = millis();
  }

  // Botão D19
  if (digitalRead(buttonPin19) == HIGH && (millis() - lastDebounceTime) > debounceDelay) {
    if (soundbarOn) {
      printToSerialAndLCD("Desligando Soundbar");
      soundbarOn = false;
    } else {
      printToSerialAndLCD("Ligando Soundbar");
      soundbarOn = true;
    }
    lastDebounceTime = millis();
  }

  // Botão D21
  if (digitalRead(buttonPin21) == HIGH && (millis() - lastDebounceTime) > debounceDelay) {
    button21Clicks++;
    if (button21Clicks == 1) {
      printToSerialAndLCD("Ligando Iluminação (Modo 1)");
      lightingMode = 1;
      digitalWrite(relayPin, HIGH);  // Liga o relé
    } else if (button21Clicks == 2) {
      printToSerialAndLCD("Ligando Iluminação (Modo 2)");
      lightingMode = 2;
    } else if (button21Clicks == 3) {
      printToSerialAndLCD("Desligando Iluminação");
      lightingMode = 0;
      digitalWrite(relayPin, LOW);  // Desliga o relé
      button21Clicks = 0;
    }
    lastDebounceTime = millis();
  }

  // Receptor IR
  if (irrecv.decode(&results)) {
    // Imprime o código IR capturado na porta serial
    Serial.print("Código IR: 0x");
    Serial.println(results.value, HEX);

    // Envie o código IR capturado (retransmita) com o emissor IR (opcional)
    irsend.sendNEC(results.value, 32);

    irrecv.resume();  // Continue a aguardar outros códigos IR
  }
}