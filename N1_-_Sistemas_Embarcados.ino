#include <Wire.h>               // Biblioteca para comunicação I2C
#include <LiquidCrystal_I2C.h>  // Biblioteca para o LCD I2C
#include <DHT.h>                // Biblioteca para o sensor DHT11
#include "DHTesp.h"
#include <RTClib.h> //INCLUSÃO DA BIBLIOTECA
#include <EEPROM.h>
#include <Crypto.h>
#include "SHA256.h"

// Definições de notas musicais
// Aqui estão definidas várias notas musicais com suas frequências em Hz
// A partir destas definições, as notas são usadas para reproduzir melodias
// em um buzzer
// Exemplo: NOTE_C4 representa a nota Dó (C4)
#define NOTE_B0 31
// Outras notas omitidas para brevidade...
#define NOTE_DS8 4978

// Definição dos pinos dos componentes conectados ao Arduino
const int PIN_SWITCH1 = 6;  // Pino para controle de switch 1
const int PIN_SWITCH2 = 8;  // Pino para controle de switch 2
const int PIN_DHT = 13;     // Pino para o sensor DHT
const int PIN_BZR = 9;      // Pino para o buzzer
const int PIN_LDR = A0;     // Pino para o sensor de luminosidade LDR
const int BTN_LOG = 12;     // Pino para o botão de leitura de logs

// Definição da estrutura de dados para cada entrada de log
struct LogEntry {
  byte day;
  byte month;
  byte year;
  byte hour;
  byte minute;
  char message[5];  // Mensagem de log com tamanho máximo de 5 caracteres
};

// Constantes relacionadas à EEPROM
const int EEPROM_SIZE = 5;             // Tamanho total para armazenar os registros
const int EEPROM_START_ADDR = 0;       // Endereço inicial na EEPROM para armazenamento
int currentAddress = EEPROM_START_ADDR; // Endereço atual na EEPROM

RTC_DS3231 rtc; //OBJETO DO TIPO RTC_DS3231
char daysOfTheWeek[7][12] = {"Domingo", "Segunda", "Terça", "Quarta", 
"Quinta", "Sexta", "Sábado"};

DHTesp dhtSensor;
LiquidCrystal_I2C lcd(0x27,16,2);  

// Constantes e variáveis para cálculos relacionados à luz
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

// Outras matrizes omitidas para brevidade...

// Melodia e durações de notas musicais para o buzzer
int melody[] = {
  // Notas musicais representadas como frequências em Hz
};

int durations[] = {
  // Durações das notas musicais
};

void setup() {
  Serial.begin(115200); // Inicializa o monitor serial
  
  if (!rtc.begin()) { // Verifica se o RTC DS3231 está funcionando corretamente
    Serial.println("DS3231 não encontrado");
    while(1); // Loop infinito se o RTC não for encontrado
  }
  
  if (rtc.lostPower()) { // Se o RTC perdeu energia ou é a primeira inicialização
    Serial.println("DS3231 OK!");
    // Define a data e hora do RTC como a data e hora de compilação do código
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
  // Configuração do sensor DHT
  dhtSensor.setup(PIN_DHT, DHTesp::DHT11);
  
  // Configuração dos pinos do buzzer, botão de log e sensor de luminosidade
  pinMode(PIN_BZR, OUTPUT);
  pinMode(BTN_LOG, INPUT_PULLUP);
  pinMode(PIN_LDR, INPUT);
  
  lcd.init(); // Inicialização do LCD
  lcd.backlight(); // Ativa a retroiluminação do LCD
  
  // Exibe um logo/slogan animado no LCD
  // Este código cria e exibe caracteres personalizados
  // Representando um pacman e pontos
}

void loop() {
  unsigned long currentMillis = millis();

  // Verifica se o botão de leitura de logs foi pressionado
  if (digitalRead(BTN_LOG) == LOW) {
    readFromEEPROM();
  }

  // Lê a temperatura e umidade do sensor DHT
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  float temperature = data.temperature;
  float humidity = data.humidity;
  
  // Lê a intensidade da luz (lux) do sensor LDR
  int ldrValue = analogRead(PIN_LDR);
  int intensidadeLuz = map(ldrValue, 0, 1023, 0, 100);
  
  // Incrementa somente se as leituras forem válidas
  if (!isnan(temperature) && !isnan(humidity)) {
    totalTemperature += temperature;
    totalHumidity += humidity;
    readingsCount++;
  }

  totalLux += intensidadeLuz;

  // Verifica as condições para ativar o alarme e fazer log
  if (currentMillis < 60000) {
    // Exibe dados no LCD e verifica condições para ativar o alarme e fazer log
  } else {
    // Outras verificações e ações omitidas para brevidade...
  }

  // Verifica condições para ativar o alarme e fazer log
  if (currentMillis > 60000) {
    // Outras verificações e ações omitidas para brevidade...
  }

  delay(1000); // Delay para evitar leituras rápidas do sensor
}

// Função para fazer log na EEPROM
void logToEEPROM(const char* message) {
  // Lê a data e hora atual do RTC
  DateTime now = rtc.now();

  // Cria um novo registro de log
  LogEntry entry;
  entry.day = now.day();
  entry.month = now.month();
  entry.year = now.year() - 2000; // Ajusta para armazenar apenas os dois últimos dígitos do ano
  entry.hour = now

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
