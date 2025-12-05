#include <esp_now.h>
#include <WiFi.h>

#define ADC_VREF_mV    5300.0 // Valor em milivolt : 5.3V ( variação da bateria ? )
#define ADC_RESOLUTION 4096.0
#define PIN_LM35       34 // GPIO34 no Wemos 


// MAC Do esp8266 
uint8_t macESP8266[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; 

typedef struct __attribute__((packed)) {
  float temperatura;
  uint16_t adc;
} payload_t;

payload_t pacote;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.channel(1);
 analogSetAttenuation(ADC_11db);
  if (esp_now_init() != ESP_OK) while(1);

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, macESP8266, 6);
  peer.channel = 1;
  esp_now_add_peer(&peer);
}

void loop() {

  float adcVal = 0;


  // Func pra reduzir os erros
  for(int x =0;x<10;x++){
    adcVal += analogRead(34);

    Serial.printf("→ %f", x);
    delay(20);
  }
  adcVal = adcVal/10;
 
  Serial.printf("→ %f", adcVal);
  float milliVolt = adcVal * (ADC_VREF_mV / ADC_RESOLUTION);
  float tempC = milliVolt / 10;

  pacote.temperatura = tempC;
  //ADC = 0 só pra manter o size do pacote
  pacote.adc = 0;

  // Envia o pacote pro ESP8266
  esp_now_send(macESP8266, (uint8_t*)&pacote, sizeof(pacote));

  Serial.printf("→ %.2f °C | %.3f V | ADC:%d\n", tempC);

  delay(2000);
}