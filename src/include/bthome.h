#ifndef SRC_INCLUDE_BTHOME_H_
#define SRC_INCLUDE_BTHOME_H_

#define ADV_BTHOME_UUID16 0xFCD2
#define BTHOME_VERSION    0x02

typedef enum {
    device_info_encrypt_none    = 0x00,
    device_info_encrypt         = 0x01,
    device_info_version         = (BTHOME_VERSION << 5)
} bthome_device_info_t;

typedef enum {
    button_none                 = 0x00,
    button_press                = 0x01,
    button_double_press         = 0x02,
    button_triple_press         = 0x03,
    button_long_press           = 0x04,
    button_long_double_press    = 0x05,
    button_long_triple_press    = 0x06,
} bthome_events_button;

typedef enum {
    dimmer_none = 0x00,
    dimmer_rotate_left  = 0x01,     /* steps 3C0103  rotate left 3 steps    */
    dimmer_rotate_right = 0x02,     /* steps 3C020A  rotate right 10 steps  */
} bthome_events_dimmer;

typedef enum {
    /* Sensor data */
    BTHomeID_packet_id                  = 0x00,     /* uint8  (1 byte),  0..255                         */
    BTHomeID_battery                    = 0x01,     /* uint8  (1 byte),  1,     %                       */
    BTHomeID_co2                        = 0x12,     /* uint16 (2 bytes), 1,     ppm                     */
    BTHomeID_count8                     = 0x09,     /* uint   (1 bytes), 1                              */
    BTHomeID_count16                    = 0x3D,     /* uint   (2 bytes), 1                              */
    BTHomeID_count32                    = 0x3E,     /* uint   (4 bytes), 1                              */
    BTHomeID_current                    = 0x43,     /* uint16 (2 bytes), 0.001, A                       */
    BTHomeID_dewpoint                   = 0x08,     /* sint16 (2 bytes), 0.01,  °C                      */
    BTHomeID_distance_mm                = 0x40,     /* uint16 (2 bytes), 1,     mm                      */
    BTHomeID_distance_m                 = 0x41,     /* uint16 (2 bytes), 0.1,   m                       */
    BTHomeID_duration                   = 0x42,     /* uint24 (3 bytes), 0.001, s                       */
    BTHomeID_energy                     = 0X0A,     /* uint24 (3 bytes), 0.001, kWh                     */
    BTHomeID_humidity_01                = 0x03,     /* uint16 (2 bytes), 0.01,  %                       */
    BTHomeID_humidity                   = 0x2E,     /* uint8  (1 byte),  1,     %                       */
    BTHomeID_illuminace                 = 0x05,     /* uint24 (3 bytes), 0.01,  lux                     */
    BTHomeID_mass_kg                    = 0x06,     /* uint16 (2 byte),  0.01,  kg                      */
    BTHomeID_mass_lb                    = 0x07,     /* uint16 (2 byte),  0.01,  lb                      */
    BTHomeID_moisture_01                = 0x14,     /* uint16 (2 bytes), 0.01,  %                       */
    BTHomeID_moisture                   = 0x2F,     /* uint8  (1 byte),  1,     %                       */
    BTHomeID_pm25                       = 0x0D,     /* uint16 (2 bytes), 1,     ug/m3                   */
    BTHomeID_pm10                       = 0x0E,     /* uint16 (2 bytes), 1,     ug/m3                   */
    BTHomeID_power                      = 0x0B,     /* uint24 (3 bytes), 0.01,  W                       */
    BTHomeID_pressure                   = 0x04,     /* uint24 (3 bytes), 0.01,  hPa                     */
    BTHomeID_rotation                   = 0x3F,     /* sint16 (2 bytes), 0.1,   °                       */
    BTHomeID_speed                      = 0x44,     /* uint16 (2 bytes), 0.01,  m/s                     */
    BTHomeID_temperature                = 0x45,     /* sint16 (2 bytes), 0.1,   °C                      */
    BTHomeID_temperature_01             = 0x02,     /* sint16 (2 bytes), 0.01,  °C                      */
    BTHomeID_tvoc                       = 0x13,     /* uint16 (2 bytes), 1,     ug/m3                   */
    BTHomeID_voltage_001                = 0x0C,     /* uint16 (2 bytes), 0.001, V                       */
    BTHomeID_voltage                    = 0x4A,     /* uint16 (2 bytes), 0.1,   V                       */
    BTHomeID_UV_index                   = 0x46,     /* uint8  (1 byte),  0.1                            */
    /* binary sensors */
    BTHomeID_binary_battery             = 0x15,     /* uint8 (1 byte), 1500, 0 (False = Low)            */
    BTHomeID_binary_battery_charging    = 0x16,     /* uint8 (1 byte), 1601, 1 (True = Charging)        */
    BTHomeID_binary_carbon_monoxide     = 0x17,     /* uint8 (1 byte), 1700, 0 (False = Not detected)   */
    BTHomeID_binary_cold                = 0x18,     /* uint8 (1 byte), 1801, 1 (True = Cold)            */
    BTHomeID_binary_connectivity        = 0x19,     /* uint8 (1 byte), 1900, 0 (False = Disconnected)   */
    BTHomeID_binary_door                = 0x1A,     /* uint8 (1 byte), 1A00, 0 (False = Closed)         */
    BTHomeID_binary_garage_door         = 0x1B,     /* uint8 (1 byte), 1B01, 1 (True = Open)            */
    BTHomeID_binary_gas                 = 0x1C,     /* uint8 (1 byte), 1C01, 1 (True = Detected)        */
    BTHomeID_binary_generic_boolean     = 0x0F,     /* uint8 (1 byte), 0F01, 1 (True = On)              */
    BTHomeID_binary_heat                = 0x1D,     /* uint8 (1 byte), 1D00, 0 (False = Normal)         */
    BTHomeID_binary_light               = 0x1E,     /* uint8 (1 byte), 1E01, 1 (True = Light detected)  */
    BTHomeID_binary_lock                = 0x1F,     /* uint8 (1 byte), 1F01, 1 (True = Unlocked)        */
    BTHomeID_binary_moisture            = 0x20,     /* uint8 (1 byte), 2001, 1 (True = Wet)             */
    BTHomeID_binary_motion              = 0x21,     /* uint8 (1 byte), 2100, 0 (False = Clear)          */
    BTHomeID_binary_moving              = 0x22,     /* uint8 (1 byte), 2201, 1 (True = Moving)          */
    BTHomeID_binary_occupancy           = 0x23,     /* uint8 (1 byte), 2301, 1 (True = Detected)        */
    BTHomeID_binary_opening             = 0x11,     /* uint8 (1 byte), 1100, 0 (False = Closed)         */
    BTHomeID_binary_plug                = 0x24,     /* uint8 (1 byte), 2400, 0 (False = Unplugged)      */
    BTHomeID_binary_power               = 0x10,     /* uint8 (1 byte), 1001, 1 (True = On)              */
    BTHomeID_binary_presence            = 0x25,     /* uint8 (1 byte), 2500, 0 (False = Away)           */
    BTHomeID_binary_problem             = 0x26,     /* uint8 (1 byte), 2601, 1 (True = Problem)         */
    BTHomeID_binary_running             = 0x27,     /* uint8 (1 byte), 2701, 1 (True = Running)         */
    BTHomeID_binary_safety              = 0x28,     /* uint8 (1 byte), 2800, 0 (False = Unsafe)         */
    BTHomeID_binary_smoke               = 0x29,     /* uint8 (1 byte), 2901, 1 (True = Detected)        */
    BTHomeID_binary_sound               = 0x2A,     /* uint8 (1 byte), 2A00, 0 (False = Clear)          */
    BTHomeID_binary_tamper              = 0x2B,     /* uint8 (1 byte), 2B00, 0 (False = Off)            */
    BTHomeID_binary_vibration           = 0x2C,     /* uint8 (1 byte), 2C01, 1 (True = Detected)        */
    BTHomeID_binary_window              = 0x2D,     /* uint8 (1 byte), 2D01, 1 (True = Closed)          */
    /* Events */
    BTHomeID_evets_button               = 0x3A,     /* uint8 (1 byte), 3A00                             */
    BTHomeID_evets_dimmer               = 0x3C,     /* uint8 (1 byte), 3C00                             */
} bthome_id_t;

typedef struct __attribute__((packed)) _tariff_t {
    uint8_t  pkt_id;        /* BTHomeID_packet_id   */
    uint8_t  pid;           /* 0..255               */
    uint8_t  tariff1_id;    /* BTHomeID_energy      */
    uint8_t  tariff_1[3];   /* Tariff 1             */
    uint8_t  tariff2_id;    /* BTHomeID_energy      */
    uint8_t  tariff_2[3];   /* Tariff 2             */
    uint8_t  tariff3_id;    /* BTHomeID_energy      */
    uint8_t  tariff_3[3];   /* Tariff 3             */
} tariff_t;

typedef struct __attribute__((packed)) _power_voltage_t {
    uint8_t  pkt_id;        /* BTHomeID_packet_id   */
    uint8_t  pid;           /* 0..255               */
    uint8_t  power_id;      /* BTHomeID_power       */
    uint8_t  power[3];      /* Power                */
    uint8_t  voltage_id;    /* BTHomeID_voltage     */
    uint16_t voltage;       /* Voltage 220V         */
    uint8_t  battery_id;    /* BTHomeID_battery     */
    uint8_t  battery_level; /* 0-100%               */
} power_voltage_t;

typedef struct __attribute__((packed)) _bthome_nonce_t {
    uint8_t  mac[6];
    uint16_t uuid16;
    uint8_t  device_info;
    uint32_t counter;
} bthome_nonce_t;

extern bthome_nonce_t bthome_nonce;
extern tariff_t tariff_data;
extern power_voltage_t pv_data;


void bthome_beacon_init();
void bthome_encrypt_tariff_data_beacon();
void bthome_encrypt_pv_data_beacon();

#endif /* SRC_INCLUDE_BTHOME_H_ */
