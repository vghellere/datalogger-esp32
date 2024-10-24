#include <sys/time.h>
#include "consts.h"
#include "HardwareSerial.h"

void print(const char message[]);
void print(const String &s);
void println(const char message[]);
void println(const String &s);
void set_time_epoch(long epoch);
long get_time_epoch();