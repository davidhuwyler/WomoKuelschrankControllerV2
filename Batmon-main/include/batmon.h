#pragma once

struct BM6Data {
  uint16_t voltage;
  int16_t temperature;
};

void batmon_init();
void get_batmon_data(struct BM6Data* data);