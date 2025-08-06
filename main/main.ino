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

#define TX_GPIO_NUM GPIO_NUM_17
#define RX_GPIO_NUM GPIO_NUM_16

uint8_t nodeID = 2;  // Change this to set your device's node ID


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
    .alerts_enabled = TWAI_ALERT_RX_DATA,
    .clkout_divider = 0,
    .intr_flags = ESP_INTR_FLAG_LEVEL1
  };

  // Timing configuration for 500 kbps
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();

  // // Filter to accept only standard ID 0x101
  // twai_filter_config_t f_config = {
  //   .acceptance_code = (0x101 << 21),  // left-align 11-bit ID
  //   .acceptance_mask = (0x7FF << 21),  // mask all 11 bits
  //   .single_filter = true
  // };

  //Accept all messages
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  // Install and start TWAI driver
  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
    Serial.println("TWAI driver install failed");
    while (true);
  }

  if (twai_start() != ESP_OK) {
    Serial.println("TWAI driver start failed");
    while (true);
  }

  Serial.println("TWAI driver started, filtering for ID 0x101");
}

void loop() {
  handleCAN(nodeID);

}