#include <Wire.h>               // Biblioteca para comunicação I2C
#include <LiquidCrystal_I2C.h>  // Biblioteca para o LCD I2C
#include <DHT.h>                // Biblioteca para o sensor DHT11
#include "DHTesp.h"             // Biblioteca para o sensor DHT11
#include <RTClib.h>             //Biblioteca para o RTC
#include <EEPROM.h>             //Biblioteca para gravar infos na log
#include <Crypto.h>             //Biblioteca para criptografia hash
#include "SHA256.h"/            //Biblioteca para criptografia hash

// Definições de notas musicais
// Aqui estão definidas várias notas musicais com suas frequências em Hz
// A partir destas definições, as notas são usadas para reproduzir melodias
// em um buzzer
// Exemplo: NOTE_C4 representa a nota Dó (C4)
#define NOTE_B0 31
#define NOTE_C1 33
#define NOTE_CS1 35
#define NOTE_D1 37
#define NOTE_DS1 39
#define NOTE_E1 41
#define NOTE_F1 44
#define NOTE_FS1 46
#define NOTE_G1 49
#define NOTE_GS1 52
#define NOTE_A1 55
#define NOTE_AS1 58
#define NOTE_B1 62
#define NOTE_C2 65
#define NOTE_CS2 69
#define NOTE_D2 73
#define NOTE_DS2 78
#define NOTE_E2 82
#define NOTE_F2 87
#define NOTE_FS2 93
#define NOTE_G2 98
#define NOTE_GS2 104
#define NOTE_A2 110
#define NOTE_AS2 117
#define NOTE_B2 123
#define NOTE_C3 131
#define NOTE_CS3 139
#define NOTE_D3 147
#define NOTE_DS3 156
#define NOTE_E3 165
#define NOTE_F3 175
#define NOTE_FS3 185
#define NOTE_G3 196
#define NOTE_GS3 208
#define NOTE_A3 220
#define NOTE_AS3 233
#define NOTE_B3 247
#define NOTE_C4 262
#define NOTE_CS4 277
#define NOTE_D4 294
#define NOTE_DS4 311
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_FS4 370
#define NOTE_G4 392
#define NOTE_GS4 415
#define NOTE_A4 440
#define NOTE_AS4 466
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_CS5 554
#define NOTE_D5 587
#define NOTE_DS5 622
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_FS5 740
#define NOTE_G5 784
#define NOTE_GS5 831
#define NOTE_A5 880
#define NOTE_AS5 932
#define NOTE_B5 988
#define NOTE_C6 1047
#define NOTE_CS6 1109
#define NOTE_D6 1175
#define NOTE_DS6 1245
#define NOTE_E6 1319
#define NOTE_F6 1397
#define NOTE_FS6 1480
#define NOTE_G6 1568
#define NOTE_GS6 1661
#define NOTE_A6 1760
#define NOTE_AS6 1865
#define NOTE_B6 1976
#define NOTE_C7 2093
#define NOTE_CS7 2217
#define NOTE_D7 2349
#define NOTE_DS7 2489
#define NOTE_E7 2637
#define NOTE_F7 2794
#define NOTE_FS7 2960
#define NOTE_G7 3136
#define NOTE_GS7 3322
#define NOTE_A7 3520
#define NOTE_AS7 3729
#define NOTE_B7 3951
#define NOTE_C8 4186
#define NOTE_CS8 4435
#define NOTE_D8 4699
#define NOTE_DS8 4978

// Definição dos pinos dos componentes conectados ao Arduino
const int PIN_SWITCH1 = 6;
const int PIN_SWITCH2 = 8;
const int PIN_DHT = 13;
const int PIN_BZR = 9;
const int PIN_LDR = A0;
const int BTN_LOG = 12;

// Definição da estrutura de dados para cada entrada de log
struct LogEntry {
  byte day;
  byte month;
  byte year;
  byte hour;
  byte minute;
  char message[5];
};

// Constantes relacionadas à EEPROM
const int EEPROM_SIZE = 5; // Tamanho total para armazenar os registros
const int EEPROM_START_ADDR = 0; // Endereço inicial na EEPROM para armazenamento

int currentAddress = EEPROM_START_ADDR; // Endereço atual na EEPROM


RTC_DS3231 rtc; //OBJETO DO TIPO RTC_DS3231
//DECLARAÇÃO DOS DIAS DA SEMANA
char daysOfTheWeek[7][12] = {"Domingo", "Segunda", "Terça", "Quarta", 
"Quinta", "Sexta", "Sábado"};


// Constantes e variáveis para cálculos relacionados à luz, temperatura e umidade
DHTesp dhtSensor;
LiquidCrystal_I2C lcd(0x27,16,2);  
const float GAMMA = 0.7;
const float RL10 = 50;
unsigned long lastLogTime = 0;
const unsigned long logInterval = 60000;
unsigned long lastMinute = 0;
const long maxLux = 10000;
float totalTemperature = 0.0;
float totalHumidity = 0.0;
float totalLux = 0.0;
int readingsCount = 0;


// Matrizes para definição de caracteres personalizados no LCD
uint8_t pacman[8] = {
  0b00000,
  0b00000,
  0b01110,
  0b11011,
  0b11111,
  0b01110,
  0b00000,
  0b00000
};

uint8_t pacmanOpen[] = {
  0b00000,
  0b00000,
  0b01110,
  0b11011,
  0b11100,
  0b01110,
  0b00000,
  0b00000
};

uint8_t dot[] = {
  0b00000,
  0b00000,
  0b00000,
  0b00110,
  0b00110,
  0b00000,
  0b00000,
  0b00000
};



// Melodia e durações de notas musicais para o buzzer
int melody[] = {
  NOTE_B4, NOTE_B5, NOTE_FS5, NOTE_DS5,
  NOTE_B5, NOTE_FS5, NOTE_DS5, NOTE_C5,
  NOTE_C6, NOTE_G6, NOTE_E6, NOTE_C6, NOTE_G6, NOTE_E6,
  
  NOTE_B4, NOTE_B5, NOTE_FS5, NOTE_DS5, NOTE_B5,
  NOTE_FS5, NOTE_DS5, NOTE_DS5, NOTE_E5, NOTE_F5,
  NOTE_F5, NOTE_FS5, NOTE_G5, NOTE_G5, NOTE_GS5, NOTE_A5, NOTE_B5
};

// Durações das notas musicais
int durations[] = {
  16, 16, 16, 16,
  32, 16, 8, 16,
  16, 16, 16, 32, 16, 8,
  
  16, 16, 16, 16, 32,
  16, 8, 32, 32, 32,
  32, 32, 32, 32, 32, 16, 8
};


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
  lcd.setCursor(6, 0);
  lcd.print("LEMA");
  lcd.setCursor(0, 1);
  //lcd.print("Slogan aqui");

  lcd.createChar(1, pacman);
  lcd.createChar(2, dot);
  for (int i = 0; i < 16; i++) {
    lcd.setCursor(i, 1);
    lcd.print("\1");
    for (int j = i + 1; j < 16; j++) {
      lcd.setCursor(j, 1);
      lcd.print("\2");
    }
    lcd.createChar(1, pacman);
    delay(200);
    lcd.createChar(1, pacmanOpen);
    delay(200);
    lcd.setCursor(i, 3);
    lcd.print(" ");
  }

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
  int intensidadeLuz = map(analogValue, 0, 1023, 0, 100);
  
  totalLux += intensidadeLuz;

  if(currentMillis < 60000){
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(temperature);
    lcd.print((char)223);
    lcd.print("C           ");

    lcd.setCursor(0, 1);
    lcd.print("U:");
    lcd.print(humidity);
    lcd.print("% ");

    lcd.print("L:");
    lcd.print(intensidadeLuz);
    lcd.print("%           ");

    if (temperature > 25 || temperature < 15 || humidity > 50 || humidity < 30 || intensidadeLuz > 30 || intensidadeLuz < 0) {

      int size = sizeof(durations) / sizeof(int);
    
      for (int note = 0; note < size; note++) {
        //to calculate the note duration, take one second divided by the note type.
        //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
        int duration = 1000 / durations[note];
        tone(PIN_BZR, melody[note], duration);
    
        //to distinguish the notes, set a minimum time between them.
        //the note's duration + 30% seems to work well:
        int pauseBetweenNotes = duration * 1.30;
        delay(pauseBetweenNotes);
        
        //stop the tone playing:
        noTone(PIN_BZR);
      }
      
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
    } 
    else {
      noTone(PIN_BZR);
      pinMode(PIN_SWITCH2, OUTPUT);
      digitalWrite(PIN_SWITCH2, HIGH);
      pinMode(PIN_SWITCH1, OUTPUT);
      digitalWrite(PIN_SWITCH1, LOW);
    }
  }
   float averageTemperature;
   float averageHumidity;
   float averageLux;
  
  if (currentMillis - lastMinute >= 60000) { // Se passou 1 minuto
    lastMinute = currentMillis;

    // Calcula média ponderada
    averageTemperature = totalTemperature / readingsCount;
    averageHumidity = totalHumidity / readingsCount;
    averageLux = totalLux / readingsCount;

    // Exibe médias no LCD
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(averageTemperature);
    lcd.print((char)223);
    lcd.print("C        ");
    
    lcd.setCursor(0, 1);
    lcd.print("U:");
    lcd.print(averageHumidity);
    lcd.print("% ");
    
    lcd.print("L:");
    lcd.print(averageLux);
    lcd.print("%           ");


    if (averageTemperature > 25 || averageTemperature < 15 || averageHumidity > 50 || averageHumidity < 30 || averageLux > 30 || averageLux < 0) {
    pinMode(PIN_SWITCH1, OUTPUT);
    digitalWrite(PIN_SWITCH1, HIGH);
    pinMode(PIN_SWITCH2, OUTPUT);
    digitalWrite(PIN_SWITCH2, LOW);
    if (currentMillis - lastLogTime >= logInterval) {
      String message = "";
      if (averageTemperature > 25 || averageTemperature < 15){
        message += "T";
      }
      if(averageHumidity > 50 || averageHumidity < 30 ){
        message += "H";
      }
      if(averageLux > 30 || averageLux < 0){
        message += "L";
      }
      logToEEPROM(message.c_str());
      lastLogTime = currentMillis;
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
    Serial.println();
  } 
  else {
    noTone(PIN_BZR);
    pinMode(PIN_SWITCH2, OUTPUT);
    digitalWrite(PIN_SWITCH2, HIGH);
    pinMode(PIN_SWITCH1, OUTPUT);
    digitalWrite(PIN_SWITCH1, LOW);
  }
    // Reinicia contadores
    totalTemperature = 0.0;
    totalHumidity = 0.0;
    totalLux = 0.0;
    readingsCount = 0;
  }

  if (currentMillis > 60000){
    if (averageTemperature > 25 || averageTemperature < 15 || averageHumidity > 50 || averageHumidity < 30 || averageLux > 30 || averageLux < 0){
     int size = sizeof(durations) / sizeof(int);
    
      for (int note = 0; note < size; note++) {
        //to calculate the note duration, take one second divided by the note type.
        //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
        int duration = 1000 / durations[note];
        tone(PIN_BZR, melody[note], duration);
    
        //to distinguish the notes, set a minimum time between them.
        //the note's duration + 30% seems to work well:
        int pauseBetweenNotes = duration * 1.30;
        delay(pauseBetweenNotes);
        
        //stop the tone playing:
        noTone(PIN_BZR);
      } 
    }
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
    replaceLetters(entry);
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
  Serial.println("---------------------------------------------");
}

void replaceLetters(LogEntry &entry) {
       // Converter a mensagem para uma String temporária
    String tempMessage = entry.message;
    
    // Substituir a letra 'H' por 'Umidade'
    for (int i = 0; tempMessage[i] != '\0'; i++) {
        if (tempMessage[i] == 'H') {
            tempMessage[i] = 'U';
            tempMessage = tempMessage.substring(0, i + 1) + "midade " + tempMessage.substring(i + 1);
        }
    }
    
    // Substituir a letra 'L' por 'Luminosidade'
    for (int i = 0; tempMessage[i] != '\0'; i++) {
        if (tempMessage[i] == 'L') {
            tempMessage[i] = 'L';
            tempMessage = tempMessage.substring(0, i + 1) + "uminosidade " + tempMessage.substring(i + 1);
        }
    }
    
    // Substituir a letra 'T' por 'Temperatura'
    for (int i = 0; tempMessage[i] != '\0'; i++) {
        if (tempMessage[i] == 'T') {
            tempMessage[i] = 'T';
            tempMessage = tempMessage.substring(0, i + 1) + "emperatura " + tempMessage.substring(i + 1);
        }
    }
    
    // Copiar a mensagem modificada de volta para a estrutura LogEntry
    strcpy(entry.message, tempMessage.c_str());
}
