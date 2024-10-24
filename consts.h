#define DEBUG               true
#define SAMPLE_TIME         5 * 60 * 1000 // Sample every 5 minutes
#define MAX_SAMPLES         1440 // 5 days worth of data if sampling every 5 minutes. Memory wise: we can safely store 10k samples

#define ONE_WIRE_BUS_PIN    4 // pin the sensor is connected to

#define TEMP_SERVICE_UUID             "fa7770c3-712f-488f-8eff-6ca4661be914"
#define TEMP_CHARACTERISTIC_UUID      "c707fa4f-c906-4878-bf7e-649cb06b461d"
#define DATETIME_SERVICE_UUID         "19966f5f-0de7-41fb-96e2-38e8e37949ff"
#define DATETIME_CHARACTERISTIC_UUID  "728fa1b3-9af7-4eba-b05a-471f594191e9"