// CM_PDO.cpp
#include "CM_PDO.h"
#include "CM_ObjectDictionary.h"  // for findODEntry
#include <string.h>

static PdoComm rpdoComm[4];
static PdoMap  rpdoMap[4];

static PdoComm tpdoComm[4];
static PdoMap  tpdoMap[4];
static TpdoState tpdoState[4];
static bool tpdoDirty[4];

static bool operational = true; // gate with NMT state in your handler

static void setComm(PdoComm& c, uint32_t cob, uint8_t ttype, uint16_t inhibit_ms, uint16_t evt_ms) {
  c.cob_id = cob;
  c.trans_type = ttype;
  c.inhibit_time = inhibit_ms;  // stored in ms for runtime simplicity
  c.event_timer = evt_ms;
  c.enabled = ((cob & 0x80000000u) == 0);
}

void initDefaultPDOs(uint8_t nodeId) {
  // Defaults: enable TPDO1/RPDO1 only, async (255), no inhibit, 100 ms event timer
  setComm(tpdoComm[0], 0x180 + nodeId, 255, 0, 100);
  setComm(rpdoComm[0], 0x200 + nodeId, 255, 0, 0);
  for (int i=1;i<4;i++) {
    setComm(tpdoComm[i], 0x80000000u | (0x180 + (i*0x100) + nodeId), 255, 0, 0); // disabled by bit31
    setComm(rpdoComm[i], 0x80000000u | (0x200 + (i*0x100) + nodeId), 255, 0, 0);
  }

  // Example mapping:
  // TPDO1: send speed (0x0001:00, 8 bits) + deviceMode (0x1000:00, 8 bits)
  tpdoMap[0].count = 2;
  tpdoMap[0].e[0] = {0x0001, 0x00, 8};
  tpdoMap[0].e[1] = {0x1000, 0x00, 8};

  // RPDO1: receive speed (8 bits)
  rpdoMap[0].count = 1;
  rpdoMap[0].e[0] = {0x0001, 0x00, 8};

  memset(tpdoState, 0, sizeof(tpdoState));
  memset(tpdoDirty, 0, sizeof(tpdoDirty));
}

static bool readMapped(const PdoMapEntry& me, uint8_t* out, uint8_t& offsetBytes) {
  ODEntry* od = findODEntry(me.index, me.subindex);
  if (!od || (me.len_bits % 8) != 0) return false;
  uint8_t n = me.len_bits / 8;
  if (offsetBytes + n > 8) return false; // classic CAN
  // CANopen uses little-endian for basic types in mapping
  memcpy(out + offsetBytes, od->dataPtr, n);
  offsetBytes += n;
  return true;
}

static bool writeMapped(const PdoMapEntry& me, const uint8_t* in, uint8_t& offsetBytes) {
  ODEntry* od = findODEntry(me.index, me.subindex);
  if (!od || (me.len_bits % 8) != 0) return false;
  uint8_t n = me.len_bits / 8;
  if (offsetBytes + n > 8) return false;
  // Basic safety: size must match
  if (od->size != n) return false;
  memcpy(od->dataPtr, in + offsetBytes, n);
  offsetBytes += n;
  return true;
}

bool packTPDO(uint8_t pdoNum, uint8_t* outBytes, uint8_t* outLen) {
  if (pdoNum >= 4) return false;
  if (!tpdoComm[pdoNum].enabled) return false;
  uint8_t off = 0;
  for (uint8_t i=0;i<tpdoMap[pdoNum].count;i++) {
    if (!readMapped(tpdoMap[pdoNum].e[i], outBytes, off)) return false;
  }
  *outLen = off;
  return true;
}

bool unpackRPDO(uint8_t pdoNum, const uint8_t* data, uint8_t len) {
  if (pdoNum >= 4) return false;
  if (!rpdoComm[pdoNum].enabled) return false;
  // Quick length check: sum of bytes must match DLC
  uint8_t needed = 0;
  for (uint8_t i=0;i<rpdoMap[pdoNum].count;i++) needed += rpdoMap[pdoNum].e[i].len_bits/8;
  if (needed != len) return false;
  uint8_t off = 0;
  for (uint8_t i=0;i<rpdoMap[pdoNum].count;i++) {
    if (!writeMapped(rpdoMap[pdoNum].e[i], data, off)) return false;
  }
  return true;
}

void processRPDO(const twai_message_t& rx) {
  if (!operational) return;
  // Identify RPDO channel by COB-ID
  for (uint8_t i=0;i<4;i++) {
    if (rpdoComm[i].enabled && rx.identifier == (rpdoComm[i].cob_id & 0x7FF)) {
      if (!unpackRPDO(i, rx.data, rx.data_length_code)) {
        Serial.println("RPDO unpack failed");
      }
      return;
    }
  }
}

void markTpdoDirty(uint8_t pdoNum) {
  if (pdoNum < 4) tpdoDirty[pdoNum] = true;
}

void serviceTPDOs(uint8_t nodeId) {
  if (!operational) return;
  uint32_t now = millis();

  for (uint8_t i=0;i<4;i++) {
    if (!tpdoComm[i].enabled) continue;

    bool due = false;

    // Event timer
    if (tpdoComm[i].event_timer > 0) {
      if (now - tpdoState[i].last_tx_ms >= tpdoComm[i].event_timer) due = true;
    }

    // Event-driven (dirty flag)
    if (tpdoDirty[i]) due = true;

    if (!due) continue;

    // Inhibit time check
    if (tpdoComm[i].inhibit_time > 0) {
      if (now - tpdoState[i].last_tx_ms < tpdoComm[i].inhibit_time) continue;
    }

    uint8_t payload[8] = {0};
    uint8_t len = 0;
    if (!packTPDO(i, payload, &len)) continue;

    // Suppress unchanged payload (optional)
    if (tpdoState[i].last_valid && tpdoState[i].last_len == len &&
        memcmp(tpdoState[i].last_payload, payload, len) == 0) {
      tpdoDirty[i] = false;
      continue;
    }

    twai_message_t tx{};
    tx.identifier = tpdoComm[i].cob_id & 0x7FF;
    tx.data_length_code = len;
    tx.flags = TWAI_MSG_FLAG_NONE;
    memcpy(tx.data, payload, len);

    if (twai_transmit(&tx, pdMS_TO_TICKS(10)) == ESP_OK) {
      tpdoState[i].last_tx_ms = now;
      tpdoState[i].last_len = len;
      memcpy(tpdoState[i].last_payload, payload, len);
      tpdoState[i].last_valid = true;
      tpdoDirty[i] = false;
      // Serial.printf("TPDO%u sent (ID 0x%03X)\n", i+1, tx.identifier);
    } else {
      // Serial.printf("TPDO%u transmit fail\n", i+1);
    }
  }
}

// Expose a simple setter to sync NMT state from your handler
void setOperational(bool on) { operational = on; }