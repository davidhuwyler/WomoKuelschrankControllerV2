#pragma once

struct BM6Data {
  uint16_t voltage;
  int16_t temperature;
};

void batmon_init();
void get_batmon_data(struct BM6Data* data);
void get_batmon_data_and_store(struct BM6Data* data);

BM6Data* batmonRing_getValue(uint16_t i);
uint32_t batmonRing_getFillLevel();