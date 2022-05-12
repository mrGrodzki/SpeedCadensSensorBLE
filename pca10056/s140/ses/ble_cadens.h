

#include  <stdint.h> 
#include  <stdbool.h> 
#include  "ble.h" 
#include  "ble_srv_common.h"

typedef struct ble_cus_s ble_cus_t;

#define BLE_CUS_DEF(_name)                                                                          \
static ble_cus_t _name;                                                                             \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                 \
                     BLE_HRS_BLE_OBSERVER_PRIO,                                                     \
                     ble_cus_on_ble_evt, &_name)                                                                             \


//#define CUSTOM_SERVICE_UUID_BASE        {0xBC, 0x8A, 0xBF, 0x45, 0xCA, 0x05, 0x50, 0xBA, \
 //                                         0x40, 0x42, 0xB0, 0x00, 0xC9, 0xAD, 0x64, 0xF3}
//#define CUSTOM_SERVICE_UUID               0x1400
//#define CUSTOM_VALUE_CHAR_UUID            0x1401

#define BLE_UUID_CYCLING_SPEED_AND_CADENCE                       0x1816     /**< Heart Rate service UUID. */
#define BLE_UUID_CSC_MEASUREMENT_CHAR                            0x2A5B     /**< Cycling Speed and Cadence Measurement characteristic UUID. */
#define MAX_CSCM_LEN                                             20         /**< Maximum size of a transmitted Cycling Speed and Cadence Measurement. */

#define BLE_UUID_CSC_FEATURE_CHAR                                0x2A5C     /**< Cycling Speed and Cadence Feature characteristic UUID. */

#define CSC_MEAS_FLAG_MASK_WHEEL_REV_DATA_PRESENT (0x01 << 0)               /**< Wheel revolution data present flag bit. */
#define CSC_MEAS_FLAG_MASK_CRANK_REV_DATA_PRESENT (0x01 << 1)               /**< Crank revolution data present flag bit. */


typedef struct
{
    uint8_t                       initial_custom_value;           /**< Initial custom value */
    ble_srv_cccd_security_mode_t  custom_value_char_attr_md;     /**< Initial security level for Custom characteristics attribute */
} ble_cus_init_t;

struct ble_cus_s
{
    uint16_t                      service_handle;                 /**< Handle of Custom Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t      custom_value_handles;           /**< Handles related to the Custom Value characteristic. */
    uint16_t                      conn_handle;                    /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
    uint8_t                       uuid_type; 
};

typedef struct ble_cscs_meas_s
{
    bool        is_wheel_rev_data_present;                              /**< True if Wheel Revolution Data is present in the measurement. */
    bool        is_crank_rev_data_present;                              /**< True if Crank Revolution Data is present in the measurement. */
    uint32_t    cumulative_wheel_revs;                                  /**< Cumulative Wheel Revolutions. */
    uint16_t    last_wheel_event_time;                                  /**< Last Wheel Event Time. */
    uint16_t    cumulative_crank_revs;                                  /**< Cumulative Crank Revolutions. */
    uint16_t    last_crank_event_time;                                  /**< Last Crank Event Time. */
} ble_cscs_meas_t;


uint32_t ble_cus_init(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init);

void ble_cus_on_ble_evt ( ble_evt_t const * p_ble_evt, void * p_context); 

uint32_t ble_cscs_measurement_send(ble_cus_t * p_cscs, ble_cscs_meas_t * p_measurement);

