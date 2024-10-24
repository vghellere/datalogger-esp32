#include "utils.h"

void print(const char message[]) {
  #if DEBUG
  Serial.print(message);
  #endif
}

void print(const String &s) {
  #if DEBUG
  Serial.print(s);
  #endif
}

void println(const char message[]) {
  #if DEBUG
  Serial.println(message);
  #endif
}

void println(const String &s) {
  #if DEBUG
  Serial.println(s);
  #endif
}

void set_time_epoch(long epoch) {
  struct timeval tv;
  tv.tv_sec = epoch;
  tv.tv_usec = 0;
  settimeofday(&tv, NULL);
}

long get_time_epoch() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec;
}