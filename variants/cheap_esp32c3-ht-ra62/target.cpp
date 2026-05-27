#include <Arduino.h>
#include "target.h"

CheapC3Sx1262Board board;

RADIO_CLASS radio = new Module(P_LORA_NSS, P_LORA_DIO_1, P_LORA_RESET, P_LORA_BUSY);

WRAPPER_CLASS radio_driver(radio, board);

ESP32RTCClock fallback_clock;
AutoDiscoverRTCClock rtc_clock(fallback_clock);

#ifdef DISPLAY_CLASS
  DISPLAY_CLASS display;
#endif

#if ENV_INCLUDE_GPS
  #include <helpers/sensors/MicroNMEALocationProvider.h>
  MicroNMEALocationProvider nmea = MicroNMEALocationProvider(Serial1, &rtc_clock);
  EnvironmentSensorManager sensors = EnvironmentSensorManager(nmea);
#else
  EnvironmentSensorManager sensors;
#endif

bool radio_init() {
  fallback_clock.begin();
  rtc_clock.begin(Wire);

  Serial.println("[DEBUG] Initializing radio via global SPI...");
  Serial.print("  SCLK="); Serial.println(P_LORA_SCLK);
  Serial.print("  MISO="); Serial.println(P_LORA_MISO);
  Serial.print("  MOSI="); Serial.println(P_LORA_MOSI);
  Serial.print("  NSS="); Serial.println(P_LORA_NSS);
  Serial.print("  DIO1="); Serial.println(P_LORA_DIO_1);
  Serial.print("  BUSY="); Serial.println(P_LORA_BUSY);
  Serial.print("  RESET="); Serial.println(P_LORA_RESET);

  bool result = radio.std_init(&SPI);
  if (!result) {
    Serial.println("[DEBUG] First attempt failed, retrying after 500ms...");
    delay(500);
    result = radio.std_init(&SPI);
  }

  return result;
}

uint32_t radio_get_rng_seed() {
  return radio.random(0x7FFFFFFF);
}

void radio_set_params(float freq, float bw, uint8_t sf, uint8_t cr) {
  radio.setFrequency(freq);
  radio.setSpreadingFactor(sf);
  radio.setBandwidth(bw);
  radio.setCodingRate(cr);
}

void radio_set_tx_power(int8_t dbm) {
  radio.setOutputPower(dbm);
}

mesh::LocalIdentity radio_new_identity() {
  RadioNoiseListener rng(radio);
  return mesh::LocalIdentity(&rng);  // create new random identity
}

