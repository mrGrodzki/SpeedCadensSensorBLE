

#include "sdk_common.h"
#include "ble_srv_common.h"
#include "ble_cadens.h"
#include <string.h>
#include "nrf_gpio.h"
#include "boards.h"
#include "nrf_log.h"



/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_cus       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_cus_t * p_cus, ble_evt_t const * p_ble_evt)
{
    p_cus->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
  
    
}


/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_cus       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_cus_t * p_cus, ble_evt_t const * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_cus->conn_handle = BLE_CONN_HANDLE_INVALID;
 
}

void ble_cus_on_ble_evt( ble_evt_t const * p_ble_evt, void * p_context)   // handler zdarzen
{
    ble_cus_t * p_cus = (ble_cus_t *) p_context;
    
    if (p_cus == NULL || p_ble_evt == NULL)
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
        on_connect(p_cus, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
        on_disconnect(p_cus, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}


uint32_t ble_cus_init(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init)
{
    if (p_cus == NULL || p_cus_init == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t   err_code;
    ble_uuid_t ble_uuid;
    ble_add_char_params_t add_char_params;

    p_cus->conn_handle = BLE_CONN_HANDLE_INVALID;

    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_CYCLING_SPEED_AND_CADENCE);

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,     // tworzenie serwisu 
                                        &ble_uuid,
                                        &p_cus->service_handle);
    if (err_code != NRF_SUCCESS)
    {
       return err_code;
    }
    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid              = BLE_UUID_CSC_MEASUREMENT_CHAR;
    add_char_params.max_len           = MAX_CSCM_LEN;
    add_char_params.is_var_len        = false;
    add_char_params.char_props.notify = 1;
    add_char_params.cccd_write_access = 1;

    err_code = characteristic_add(p_cus->service_handle, &add_char_params, &p_cus->custom_value_handles);   // dodanie charakterystyki 
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
}
/**

przetwarzanie i zapis danych 

*/
static uint8_t csc_measurement_encode(ble_cus_t       * p_cscs,
                                      ble_cscs_meas_t * p_csc_measurement,
                                      uint8_t         * p_encoded_buffer)
{
    uint8_t flags = 0;
    uint8_t len   = 1;

   
        if (p_csc_measurement->is_wheel_rev_data_present)
        {
            flags |= CSC_MEAS_FLAG_MASK_WHEEL_REV_DATA_PRESENT;
            len += uint32_encode(p_csc_measurement->cumulative_wheel_revs, &p_encoded_buffer[len]);
            len += uint16_encode(p_csc_measurement->last_wheel_event_time, &p_encoded_buffer[len]);
        }
   
        if (p_csc_measurement->is_crank_rev_data_present)
        {
            flags |= CSC_MEAS_FLAG_MASK_CRANK_REV_DATA_PRESENT;
            len += uint16_encode(p_csc_measurement->cumulative_crank_revs, &p_encoded_buffer[len]);
            len += uint16_encode(p_csc_measurement->last_crank_event_time, &p_encoded_buffer[len]);
        }
    
    // Flags Field
    p_encoded_buffer[0] = flags;

    return len;
}


/**

Wysylanie danych

*/
uint32_t ble_cscs_measurement_send(ble_cus_t * p_cscs, ble_cscs_meas_t * p_measurement)
{
    if (p_cscs == NULL || p_measurement == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t err_code;

   
    if (p_cscs->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        uint8_t                encoded_csc_meas[MAX_CSCM_LEN];
        uint16_t               len;
        uint16_t               hvx_len;
        ble_gatts_hvx_params_t hvx_params;

        len     = csc_measurement_encode(p_cscs, p_measurement, encoded_csc_meas);
        hvx_len = len;

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_cscs-> custom_value_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = 0;
        hvx_params.p_len  = &hvx_len;
        hvx_params.p_data = encoded_csc_meas;

        err_code = sd_ble_gatts_hvx(p_cscs->conn_handle, &hvx_params);
        if ((err_code == NRF_SUCCESS) && (hvx_len != len))
        {
            err_code = NRF_ERROR_DATA_SIZE;
        }
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}

