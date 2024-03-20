#include <Wire.h>               // Biblioteca para comunicação I2C
#include <LiquidCrystal_I2C.h>  // Biblioteca para o LCD I2C
#include <DHT.h>                // Biblioteca para o sensor DHT11
#include "DHTesp.h"
#include <RTClib.h> //INCLUSÃO DA BIBLIOTECA
#include <EEPROM.h>
#include <Crypto.h>
#include <SHA256.H>

const int PIN_SWITCH1 = 6;
const int PIN_SWITCH2 = 8;
const int PIN_DHT = 13;
const int PIN_BZR = 9;
const int PIN_LDR = A0;
const int BTN_LOG = 12;

struct LogEntry {
  byte day;
  byte month;
  byte year;
  byte hour;
  byte minute;
  char message[5];
};

const int EEPROM_SIZE = 5; // Tamanho total para armazenar os registros
const int EEPROM_START_ADDR = 0; // Endereço inicial na EEPROM para armazenamento

int currentAddress = EEPROM_START_ADDR; // Endereço atual na EEPROM


RTC_DS3231 rtc; //OBJETO DO TIPO RTC_DS3231
//DECLARAÇÃO DOS DIAS DA SEMANA
char daysOfTheWeek[7][12] = {"Domingo", "Segunda", "Terça", "Quarta", 
"Quinta", "Sexta", "Sábado"};

DHTesp dhtSensor;
LiquidCrystal_I2C lcd(0x27,16,2);  
const float GAMMA = 0.7;
const float RL10 = 50;

unsigned long lastLogTime = 0;
const unsigned long logInterval = 10000;
unsigned long lastMinute = 0;
const long maxLux = 10000;
float totalTemperature = 0.0;
float totalHumidity = 0.0;
float totalLux = 0.0;
int readingsCount = 0;

void setup() {
  // Inicializa o monitor serial
  Serial.begin(115200);
  if(! rtc.begin()) { // SE O RTC NÃO FOR INICIALIZADO, FAZ
    Serial.println("DS3231 não encontrado"); //IMPRIME O TEXTO NO MONITOR SERIAL
    while(1); //SEMPRE ENTRE NO LOOP
  }
  if(rtc.lostPower()){ //SE RTC FOI LIGADO PELA PRIMEIRA VEZ / FICOU SEM 
    //ENERGIA / ESGOTOU A BATERIA, FAZ
    Serial.println("DS3231 OK!"); //IMPRIME O TEXTO NO MONITOR SERIAL
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //CAPTURA A DATA E HORA EM QUE O SKETCH É COMPILADO
    //rtc.adjust(DateTime(2018, 9, 29, 15, 00, 45)); //(ANO), (MÊS), (DIA), (HORA), (MINUTOS), (SEGUNDOS)
 }
  dhtSensor.setup(PIN_DHT, DHTesp::DHT11);
  pinMode(PIN_BZR, OUTPUT);
  pinMode(BTN_LOG, INPUT_PULLUP);
  pinMode(PIN_LDR, INPUT);
  lcd.init();
  lcd.backlight();
  // Exibe o logo/slogan da empresa no LCD
  //lcd.setCursor(0, 0);
  //lcd.print("Empresa XYZ");
  //lcd.setCursor(0, 1);
  //lcd.print("Slogan aqui");

  delay(2000); // Aguarda 2 segundos

  // Limpa o LCD
  lcd.clear();
}

void loop() {
  unsigned long currentMillis = millis();
  
  if(digitalRead(BTN_LOG) == LOW){
    readFromEEPROM();
  }

  delay(10);
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  float temperature = data.temperature;
  float humidity = data.humidity;
  int ldrValue = analogRead(PIN_LDR);

  // Incrementa somente se as leituras forem válidas
  if (!isnan(temperature) && !isnan(humidity)) {
    totalTemperature += temperature;
    totalHumidity += humidity;
    readingsCount++;
  }

  // Read light intensity (lux)
  int analogValue = analogRead(PIN_LDR);
  int intensidadeLuz = map(analogValue, 0, 1023, 100, 0);
  
  totalLux += intensidadeLuz;

  if (temperature > 25 || temperature < 15 || humidity > 50 || humidity < 30 || intensidadeLuz > 30 || intensidadeLuz < 0) {
    tone(PIN_BZR, 1000);
    pinMode(PIN_SWITCH1, OUTPUT);
    digitalWrite(PIN_SWITCH1, HIGH);
    pinMode(PIN_SWITCH2, OUTPUT);
    digitalWrite(PIN_SWITCH2, LOW);
    if (currentMillis - lastLogTime >= logInterval) {
      String message = "";
      if (temperature > 25 || temperature < 15){
        message += "T";
      }
      if(humidity > 50 || humidity < 30 ){
        message += "H";
      }
      if(intensidadeLuz > 30 || intensidadeLuz < 0){
        message += "L";
      }
      logToEEPROM(message.c_str());
      lastLogTime = currentMillis;
    } 
  } else {
    noTone(PIN_BZR);
    pinMode(PIN_SWITCH2, OUTPUT);
    digitalWrite(PIN_SWITCH2, HIGH);
    pinMode(PIN_SWITCH1, OUTPUT);
    digitalWrite(PIN_SWITCH1, LOW);
  }

  if(currentMillis < 60000){
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(temperature);
    lcd.print((char)223);
    lcd.print("C ");

    lcd.print("L:");
    lcd.print(intensidadeLuz);
    lcd.print("%           ");

    lcd.setCursor(0, 1);
    lcd.print("U:");
    lcd.print(humidity);
    lcd.print("%");

  }

  if (currentMillis - lastMinute >= 60000) { // Se passou 1 minuto
    lastMinute = currentMillis;

    // Calcula média ponderada
    float averageTemperature = totalTemperature / readingsCount;
    float averageHumidity = totalHumidity / readingsCount;
    float averageLux = totalLux / readingsCount;

    // Exibe médias no LCD
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(averageTemperature);
    lcd.print((char)223);
    lcd.print("C ");

    lcd.print("L:");
    lcd.print(averageLux);
    lcd.print("%           ");

    lcd.setCursor(0, 1);
    lcd.print("U:");
    lcd.print(averageHumidity);
    lcd.print("%");

    // Reinicia contadores
    totalTemperature = 0.0;
    totalHumidity = 0.0;
    totalLux = 0.0;
    readingsCount = 0;
  }

  SHA256 sha256;
  uint8_t hash[32];
  sha256.update((const uint8_t*)&temperature, sizeof(temperature));
  sha256.update((const uint8_t*)&humidity, sizeof(humidity));
  sha256.finalize(hash, sizeof(hash));

  // Imprime o hash calculado
  Serial.println("Hash SHA-256:");
  for (int i = 0; i < sizeof(hash); i++) {
    Serial.print(hash[i], HEX);
  }
  
  delay(1000);
}

void logToEEPROM(const char* message) {
  // Lê a data e hora atual do RTC
  DateTime now = rtc.now();

  // Cria um novo registro de log
  LogEntry entry;
  entry.day = now.day();
  entry.month = now.month();
  entry.year = now.year() - 2000; // Ajusta para armazenar apenas os dois últimos dígitos do ano
  entry.hour = now.hour();
  entry.minute = now.minute();

  strncpy(entry.message, message, sizeof(entry.message));
  entry.message[sizeof(entry.message) - 1] = '\0';

  // Calcula o endereço de memória para o próximo registro
  int nextAddress = currentAddress + sizeof(entry);

  // Se atingiu o limite máximo da EEPROM, reinicia para o início
  if (nextAddress >= EEPROM_SIZE * sizeof(entry)) {
    currentAddress = EEPROM_START_ADDR;
  }

  // Desloca os registros para abrir espaço para um novo registro
  for (int i = EEPROM_SIZE - 1; i > 0; --i) {
    int currentAddr = EEPROM_START_ADDR + i * sizeof(entry);
    int prevAddr = EEPROM_START_ADDR + (i - 1) * sizeof(entry);
    LogEntry tempEntry;
    EEPROM.get(prevAddr, tempEntry);
    EEPROM.put(currentAddr, tempEntry);
  }

  // Adiciona o novo registro no início da fila na EEPROM
  EEPROM.put(EEPROM_START_ADDR, entry);

  // Atualiza o endereço atual para o próximo registro
  currentAddress = nextAddress;
}

// Função para ler os registros da EEPROM
void readFromEEPROM() {
  for (int i = 0; i < EEPROM_SIZE; i++) {
    LogEntry entry;
    EEPROM.get(EEPROM_START_ADDR + i * sizeof(entry), entry);

    // Imprime a data e hora do registro
    Serial.print("Data: ");
    Serial.print(entry.day);
    Serial.print("/");
    Serial.print(entry.month);
    Serial.print("/");
    Serial.print(entry.year + 2000); // Ajusta para exibir o ano completo
    Serial.print(" Hora: ");
    Serial.print(entry.hour);
    Serial.print(":");
    Serial.print(entry.minute);
    Serial.print(" - Erro:");
    Serial.println(entry.message);
  }
  Serial.println("-------------------");
}
