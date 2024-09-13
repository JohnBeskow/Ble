idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       PRIV_INCLUDE_DIRS "/home/john/esp/esp-idf/components/bt/host/nimble/esp-hci/include"
                       REQUIRES bt
                       PRIV_REQUIRES nvs_flash)