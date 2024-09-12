#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "sdkconfig.h"

char *TAG = "BLE-Server";
uint8_t ble_addr_type;
void ble_app_advertise(void);

static int device_write(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt,void *arg)
{   
    printf("Data recieved from client: %.*s\n", ctxt->om->om_len, ctxt->om->om_data);
    return 0;
}


static int device_read(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    os_mbuf_append(ctxt->om, "Data from Server", strlen("Data from Server"));
    return 0;
}





//services and characteristics read/write data functions
static const struct ble_gatt_svc_def gatt_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(0x180),
        .characteristics = (struct ble_gatt_chr_def[]){
            {.uuid = BLE_UUID16_DECLARE(0xFEF4),
            .flags = BLE_GATT_CHR_F_READ,
            .access_cb = device_read}, // function called when accessed
            {.uuid = BLE_UUID16_DECLARE(0xDEAD),
            .flags = BLE_GATT_CHR_F_WRITE,
            .access_cb = device_write}, // function called when accessed
            {0}
        }
    }
{0}
};

void app_main(void)
{

    nvs_flash_init();                           //initialize nvs flash
    //esp_nimble_hci_and_controller_init();     //initialize controller but no longer REQ
    
    //initialize Nimble Config:
    nimble_port_init();                         //initialize nimble host
    ble_svc_gap_device_name_set("BleServer");   //set device name
    ble_svc_gap_init();                         //initialize gap service
    ble_svc_gatt_init();                        //initialize gatt service
    ble_gatts_count_cfg(gatt_svcs);




}