// ----------------------------------------------------------------------------
// Copyright 2016-2018 ARM Ltd.
//
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------------------------------------------------------

#include "simplem2mclient.h"
#ifdef TARGET_LIKE_MBED
#include "mbed.h"
#endif
#include "application_init.h"
#include "mcc_common_button_and_led.h"
#include "blinky.h"
#ifndef MBED_CONF_MBED_CLOUD_CLIENT_DISABLE_CERTIFICATE_ENROLLMENT
#include "certificate_enrollment_user_cb.h"
#endif

#include "SeggerRTT.h"
#include "nrf_gpio.h"
#include "WINC1500Interface.h"

static SeggerRTT rtt;

// #if defined(EMBIGGEN_2)
FileHandle* mbed::mbed_override_console(int fd) {

    return &rtt;
}
// #endif

// event based LED blinker, controlled via pattern_resource
static Blinky blinky;

static void main_application(void);


#include "BlockDevice.h"
#include "FileSystem.h"
#include "FATFileSystem.h"
#include "LittleFileSystem.h"

using namespace mbed;

#if 0
FileSystem *FileSystem::get_default_instance()
{
#if COMPONENT_SPIF 
    static LittleFileSystem flash("flash", BlockDevice::get_default_instance());
    flash.set_as_default();
    return &flash;
#elif COMPONENT_SD
    static FATFileSystem sdcard("sd", BlockDevice::get_default_instance());
    sdcard.set_as_default();
    return &sdcard;
#else
    return NULL;
#endif
}
#endif

static NetworkInterface* network_interface=NULL;
// static DigitalOut lp_pin(P0_23, 1);
int err_spi = 0;

int main(void)
{       
    rtt.write("\r\n\r\nHello world!\r\n", strlen("\r\n\r\nHello world!\r\n"));
    wait(1);

    printf("\r\n\r\nHello world!\r\n");
    wait(0.5);
    // DigitalOut lp_pin(P0_23, 1);
    // lp_pin.write(1);
    err_spi = 0;

    // network_interface = NetworkInterface::get_default_instance();
    // if(network_interface == NULL) {
    //     printf("ERROR: No NetworkInterface found!\n");
    //     return -1;
    // }

    // printf("Erasing SPI flash...\n");
    // BlockDevice *bd = BlockDevice::get_default_instance();
    // int err = bd->init();
    // printf("Init %s\n", (err ? "Fail :(" : "OK"));
    // if (!err) {
    //     bd_size_t fl_size = bd->size();
    //     err = bd->erase(0, fl_size);
    //     printf("%s\n", (err ? "Fail :(" : "OK"));
    //     err_spi =1; 
    // }
    // bd->deinit();               

    // wait(1);
    // WiFiInterface* test_wifi = &WINC1500Interface::getInstance();
    // wait(1);
    // while(1);

    // wait(0.5);
    // network_interface = NetworkInterface::get_default_instance();
    // if(network_interface == NULL) {
    //     printf("ERROR: No NetworkInterface found!\n");
    //     return -1;
    // }

// #if (defined(TARGET_NRF52840_DK) && EMBIGGEN_2)
//     // Set WINC1500 CS to high
//     static DigitalOut winc1500_cs_pin(WINC1500_CS, 1);

//     // Config "High drive '0', high drive '1'" for spif-driver.SPI_CLK pin
//     nrf_gpio_cfg(MBED_CONF_SPIF_DRIVER_SPI_CLK,
//                  NRF_GPIO_PIN_DIR_INPUT,
//                  NRF_GPIO_PIN_INPUT_CONNECT,
//                  NRF_GPIO_PIN_NOPULL,
// 				 NRF_GPIO_PIN_H0H1,
//                  NRF_GPIO_PIN_NOSENSE);

//     // Config "High drive '0', high drive '1'" for spif-driver.SPI_MOSI pin
//     nrf_gpio_cfg(MBED_CONF_SPIF_DRIVER_SPI_MOSI,
//                  NRF_GPIO_PIN_DIR_INPUT,
//                  NRF_GPIO_PIN_INPUT_CONNECT,
//                  NRF_GPIO_PIN_NOPULL,
// 				 NRF_GPIO_PIN_H0H1,
//                  NRF_GPIO_PIN_NOSENSE);

#if 1
    printf("Erasing SPI flash...\n");
    BlockDevice *bd = BlockDevice::get_default_instance();
    int err = bd->init();
    printf("Init %s\n", (err ? "Fail :(" : "OK"));
    if (!err) {
        bd_size_t fl_size = bd->size();
        err = bd->erase(0, fl_size);
        printf("%s\n", (err ? "Fail :(" : "OK"));
    }
    bd->deinit();                 
#endif

// #endif

// #if 0    
//     wait(0.5);    
//     for (;;) {
//         printf("tick!\r\n");
//         wait(0.5);        
//     }
// #endif   

#ifdef MBED_STACK_STATS_ENABLED
    print_stack_statistics();
#endif
#ifdef MBED_HEAP_STATS_ENABLED
    print_m2mobject_stats();
#endif

    mcc_platform_run_program(main_application);
    while(1);
}

// Pointers to the resources that will be created in main_application().
static M2MResource* button_res;
static M2MResource* pattern_res;
static M2MResource* blink_res;

// Pointer to mbedClient, used for calling close function.
static SimpleM2MClient *client;

void pattern_updated(const char *)
{
    printf("PUT received, new value: %s\n", pattern_res->get_value_string().c_str());
}

void blinky_completed(void)
{
    printf("Blinky completed \n");

    // Send response to backend
    blink_res->send_delayed_post_response();
}

void blink_callback(void *)
{
    String pattern_string = pattern_res->get_value_string();
    const char *pattern = pattern_string.c_str();
    printf("LED pattern = %s\n", pattern);

    // The pattern is something like 500:200:500, so parse that.
    // LED blinking is done while parsing.
    const bool restart_pattern = false;
    if (blinky.start((char*)pattern_res->value(), pattern_res->value_length(), restart_pattern, blinky_completed) == false) {
        printf("out of memory error\n");
    }
}

void button_status_callback(const M2MBase& object,
                            const M2MBase::MessageDeliveryStatus status,
                            const M2MBase::MessageType /*type*/)
{
#ifdef MBED_HEAP_STATS_ENABLED
    print_m2mobject_stats();
#endif
#ifdef MBED_HEAP_STATS_ENABLED
    print_heap_stats();
#endif
#ifdef MBED_STACK_STATS_ENABLED
    print_stack_statistics();
#endif
    
    switch(status) {
        case M2MBase::MESSAGE_STATUS_BUILD_ERROR:
            printf("Message status callback: (%s) error when building CoAP message\n", object.uri_path());
            break;
        case M2MBase::MESSAGE_STATUS_RESEND_QUEUE_FULL:
            printf("Message status callback: (%s) CoAP resend queue full\n", object.uri_path());
            break;
        case M2MBase::MESSAGE_STATUS_SENT:
            printf("Message status callback: (%s) Message sent to server\n", object.uri_path());
            break;
        case M2MBase::MESSAGE_STATUS_DELIVERED:
            printf("Message status callback: (%s) Message delivered\n", object.uri_path());
            break;
        case M2MBase::MESSAGE_STATUS_SEND_FAILED:
            printf("Message status callback: (%s) Message sending failed\n", object.uri_path());
            break;
        case M2MBase::MESSAGE_STATUS_SUBSCRIBED:
            printf("Message status callback: (%s) subscribed\n", object.uri_path());
            break;
        case M2MBase::MESSAGE_STATUS_UNSUBSCRIBED:
            printf("Message status callback: (%s) subscription removed\n", object.uri_path());
            break;
        case M2MBase::MESSAGE_STATUS_REJECTED:
            printf("Message status callback: (%s) server has rejected the message\n", object.uri_path());
            break;
        default:
            break;
    }
}

// This function is called when a POST request is received for resource 5000/0/1.
void unregister(void *)
{
    printf("Unregister resource executed\n");
    client->close();
}

// This function is called when a POST request is received for resource 5000/0/2.
void factory_reset(void *)
{
    printf("Factory reset resource executed\n");
    client->close();
    kcm_status_e kcm_status = kcm_factory_reset();
    if (kcm_status != KCM_STATUS_SUCCESS) {
        printf("Failed to do factory reset - %d\n", kcm_status);
    } else {
        printf("Factory reset completed. Now restart the device\n");
    }
}

void main_application(void)
{
#if defined(__linux__) && (MBED_CONF_MBED_TRACE_ENABLE == 0)
        // make sure the line buffering is on as non-trace builds do
        // not produce enough output to fill the buffer
        setlinebuf(stdout);
#endif 

    // if (err_spi) {
    //     while(1) {
    //         printf("Main Application\n" );
    //         wait(2);
    //     }
    // }
    printf("MCC main func entry point!\n" );

#ifdef MBED_HEAP_STATS_ENABLED
    print_m2mobject_stats();
#endif
#ifdef MBED_STACK_STATS_ENABLED
    print_stack_statistics();
#endif


    // Initialize trace-library first
    if (application_init_mbed_trace() != 0) {
        printf("Failed initializing mbed trace\n" );
        return;
    }
    
    printf("AFTER application_init_mbed_trace()!\n" );

    // Initialize storage
    if (mcc_platform_storage_init() != 0) {
        printf("Failed to initialize storage\n" );
        return;
    }

    printf("AFTER mcc_platform_storage_init()!\n" );

    // Initialize platform-specific components
    if(mcc_platform_init() != 0) {
        printf("ERROR - platform_init() failed!\n");
        return;
    }

    printf("AFTER mcc_platform_init()!\n" );

    // Print platform information
    mcc_platform_sw_build_info();

    // Print some statistics of the object sizes and their heap memory consumption.
    // NOTE: This *must* be done before creating MbedCloudClient, as the statistic calculation
    // creates and deletes M2MSecurity and M2MDevice singleton objects, which are also used by
    // the MbedCloudClient.
#ifdef MBED_HEAP_STATS_ENABLED
    print_m2mobject_stats();
#endif

    printf("Before instancing SimpleM2MClient\r\n");
        
    // SimpleClient is used for registering and unregistering resources to a server.
    SimpleM2MClient mbedClient;

    printf("Before application_init\r\n");
    
    // application_init() runs the following initializations:
    //  1. platform initialization
    //  2. print memory statistics if MBED_HEAP_STATS_ENABLED is defined
    //  3. FCC initialization.
    if (!application_init()) {
        printf("Initialization failed, exiting application!\n");
        return;
    }
    
    printf("AFTER application_init\r\n");

    // Save pointer to mbedClient so that other functions can access it.
    client = &mbedClient;

    printf("Client initialized\r\n");
#ifdef MBED_HEAP_STATS_ENABLED
    print_heap_stats();
#endif
#ifdef MBED_STACK_STATS_ENABLED
    print_stack_statistics();
#endif

    printf("BEFORE ADDING RESOURCES\r\n");

    // Create resource for button count. Path of this resource will be: 3200/0/5501.
    button_res = mbedClient.add_cloud_resource(3200, 0, 5501, "button_resource", M2MResourceInstance::INTEGER,
                              M2MBase::GET_ALLOWED, 0, true, NULL, (void*)button_status_callback);

    // Create resource for led blinking pattern. Path of this resource will be: 3201/0/5853.
    pattern_res = mbedClient.add_cloud_resource(3201, 0, 5853, "pattern_resource", M2MResourceInstance::STRING,
                               M2MBase::GET_PUT_ALLOWED, "500:500:500:500", false, (void*)pattern_updated, NULL);

    // Create resource for starting the led blinking. Path of this resource will be: 3201/0/5850.
    blink_res = mbedClient.add_cloud_resource(3201, 0, 5850, "blink_resource", M2MResourceInstance::STRING,
                             M2MBase::POST_ALLOWED, "", false, (void*)blink_callback, (void*)button_status_callback);
    // Use delayed response
    blink_res->set_delayed_response(true);

    // Create resource for unregistering the device. Path of this resource will be: 5000/0/1.
    mbedClient.add_cloud_resource(5000, 0, 1, "unregister", M2MResourceInstance::STRING,
                 M2MBase::POST_ALLOWED, NULL, false, (void*)unregister, NULL);

    // Create resource for running factory reset for the device. Path of this resource will be: 5000/0/2.
    mbedClient.add_cloud_resource(5000, 0, 2, "factory_reset", M2MResourceInstance::STRING,
                 M2MBase::POST_ALLOWED, NULL, false, (void*)factory_reset, NULL);

    printf("AFTER ADDING RESOURCES\r\n");

#ifdef MBED_HEAP_STATS_ENABLED
    print_heap_stats();
#endif
#ifdef MBED_STACK_STATS_ENABLED
    print_stack_statistics();
#endif

    mbedClient.register_and_connect();

    printf("AFTER mbedClient.register_and_connect()\r\n");

#ifndef MBED_CONF_MBED_CLOUD_CLIENT_DISABLE_CERTIFICATE_ENROLLMENT
    // Add certificate renewal callback
    mbedClient.get_cloud_client().on_certificate_renewal(certificate_renewal_cb);
#endif // MBED_CONF_MBED_CLOUD_CLIENT_DISABLE_CERTIFICATE_ENROLLMENT


    // Check if client is registering or registered, if true sleep and repeat.

    while (mbedClient.is_register_called()) {
        static int button_count = 0;
        mcc_platform_do_wait(100);
        if (mcc_platform_button_clicked()) {
            button_res->set_value(++button_count);
        }
    }

    // Client unregistered, exit program.
}
