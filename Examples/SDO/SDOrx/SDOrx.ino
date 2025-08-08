/**
 * CAN MREX main (Template) file 
 *
 * File:            CM_Handler.cpp
 * Oraganisation:   MREX
 * Author:          Chiara Gillam
 * Date Created:    5/08/2025
 * Last Modified:   6/08/2025
 *
 */

#include "driver/twai.h"
#include "CM_Handler.h"
#include "CM_Transmit.h"
#include "CM_ObjectDictionary.h"

#define TX_GPIO_NUM GPIO_NUM_5
#define RX_GPIO_NUM GPIO_NUM_4


// User code begin:
uint8_t nodeID = 2;  // Change this to set your device's node ID

unsigned long previousMillis = 0;
const long interval = 2000; // 2 seconds
// User code end

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("CAN Receiver (TWAI)");

  // General configuration
  twai_general_config_t g_config = {
    .mode = TWAI_MODE_NORMAL,
    .tx_io = TX_GPIO_NUM,
    .rx_io = RX_GPIO_NUM,
    .clkout_io = TWAI_IO_UNUSED,
    .bus_off_io = TWAI_IO_UNUSED,
    .tx_queue_len = 5,
    .rx_queue_len = 5,
    .clkout_divider = 0,
    .intr_flags = ESP_INTR_FLAG_LEVEL1
  };

  // Timing configuration for 500 kbps
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();

  //Accept all messages
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  // Install and start TWAI driver
  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
    Serial.println("TWAI driver install failed");
    while (true); // blink an led perhaps to show problem
  }

  if (twai_start() != ESP_OK) {
    Serial.println("TWAI driver start failed");
    while (true); // blink an led perhaps to show problem
  }

  //Debugging can be put in when debugging
  uint32_t alerts_to_enable = TWAI_ALERT_RX_QUEUE_FULL | TWAI_ALERT_TX_IDLE | TWAI_ALERT_BUS_ERROR;
  if (twai_reconfigure_alerts(alerts_to_enable, NULL) == ESP_OK) {
    Serial.println("TWAI alerts reconfigured");
  } else {
    Serial.println("Failed to reconfigure alerts");
  }

  // User code Setup Begin:

  // User code Setup end


}


void loop() {
  handleCAN(nodeID);
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
  }
}