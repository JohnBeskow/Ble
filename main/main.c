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
    },
{0}
};


static int ble_gap_event(struct ble_gap_event *event , void *arg)
{
    switch (event->type)
    {
        //advertise if connected
        case BLE_GAP_EVENT_CONNECT:
            ESP_LOGI("GAP", "BLE GAP EVENT CONNECT %s ", event->connect.status == 0 ? "OK!" : "FAILED!");
            if (event->connect.status != 0)
            {
                ble_app_advertise();
            }
            break;
        //advertise again after disconnect
        case BLE_GAP_EVENT_DISCONNECT:
            ESP_LOGI("GAP", "BLE GAP EVENT DISCONNECTED");
            break;
        case BLE_GAP_EVENT_ADV_COMPLETE:
            ESP_LOGI("GAP", "BLE GAP EVENT ADV COMPLETE");
            break;
        default:    
            break;
    }
    return 0;
}

// defines connections
void ble_app_advertise(void)
{
    // GAP device name definition 
    struct ble_hs_adv_fields fields;            //fields for advertising
    const char *device_name;
    memset(&fields, 0, sizeof(fields));         //sets fields to 0
    device_name = ble_svc_gap_device_name();    //get device name
    fields.name = (uint8_t *)device_name;       //set name
    fields.name_len = strlen(device_name);      //set name length
    ble_gap_adv_set_fields(&fields);            //set fields

    // GAP connectivity definition (advertising)
    struct ble_gap_adv_params adv_params;       //advertising parameters
    memset(&adv_params, 0, sizeof(adv_params)); //sets adv_params to 0
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;   //undirected connectable mode
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;   //general discoverable mode
    ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL); //start advertising

}


void ble_app_on_sync(void)
{
    ble_hs_id_infer_auto(0, &ble_addr_type);    //determines best address auto
    ble_app_advertise();                        //defines connection
}


void host_task(void *param){
    nimble_port_run();
}

void app_main(void)
{

    nvs_flash_init();                           //initialize nvs flash
    //esp_nimble_hci_and_controller_init();     //initialize controller but no longer REQ
    
    //initialize Nimble Config:
    nimble_port_init();                         //initialize nimble host
    ble_svc_gap_device_name_set("BleServer");   //set device name
    ble_svc_gap_init();                         //comms
    ble_svc_gatt_init();                        //data organization
    ble_gatts_count_cfg(gatt_svcs);             //count the number of services
    ble_gatts_add_svcs(gatt_svcs);              //add services
    ble_hs_cfg.sync_cb = ble_app_on_sync;       //sync callback
    nimble_port_freertos_init(host_task);


}