#pragma once

struct BM6Data {
  float voltage;
  int temperature;
  int power;
};

void batmon_init(void);
void get_batmon_data(struct BM6Data* data);