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

FileHandle* mbed::mbed_override_console(int fd) {
    static SeggerRTT rtt;
    return &rtt;
}

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

void stress_test1_recv() {
    //testing recv functionality    
    WiFiInterface* wifi = WiFiInterface::get_default_instance();
    if (!wifi) {
        printf("ERROR: No WiFiInterface found.\n");
        // return -1;
    }

    printf("\nConnecting to %s...\n", MBED_CONF_NSAPI_DEFAULT_WIFI_SSID);
    int ret = wifi->connect(MBED_CONF_NSAPI_DEFAULT_WIFI_SSID, MBED_CONF_NSAPI_DEFAULT_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2, M2M_WIFI_CH_ALL);
    if (ret != 0) {
        printf("\nConnection error: %d\n", ret);
        // return -1;
    }

    printf("Success\n\n");
    printf("MAC: %s\n", wifi->get_mac_address());
    printf("IP: %s\n", wifi->get_ip_address());
    printf("Netmask: %s\n", wifi->get_netmask());
    printf("Gateway: %s\n", wifi->get_gateway());
    printf("RSSI: %d\n\n", wifi->get_rssi());

    TCPSocket socket;
    nsapi_error_t response;

    // Open a socket on the network interface, and create a TCP connection to www.arm.com
    response = socket.open(wifi);
    if(0 != response) {
        printf("socket.open() failed: %d\n", response);
        // return;
    }

    response = socket.connect("192.168.1.164", 8080);
    if(0 != response) {
        printf("Error connecting: %d\n", response);
        socket.close();
        // return;
    }

    printf("Connected! \n");


    //Send a simple http request
    char sbuffer[] = "GET / HTTP/1.1\r\nHost: api.ipify.org\r\nConnection: close\r\n\r\n";
    nsapi_size_t size = strlen(sbuffer);

    // Loop until whole request send
    while(size) {
        response = socket.send(sbuffer+response, size);
        if (response < 0) {
            printf("Error sending data: %d\n", response);
            socket.close();
            // return;
        }
    
        size -= response;
        printf("sent %d [%.*s]\n", response, strstr(sbuffer, "\r\n")-sbuffer, sbuffer);
    }

    //Receieve a simple http response and print out the response line
    char rbuffer[2048];
    char* rbuff_ptr = rbuffer;
    uint16_t received_bytes = 0;
    uint16_t needed_to_receive = sizeof(rbuffer);
    int chunk_size = 16;

    while(received_bytes < needed_to_receive)
    {
        uint16_t result = socket.recv(rbuff_ptr, chunk_size);
        if (result < 0) {
        printf("Error receiving data: %d\n", result);
        } else {
            char dummy_string[100];
            sprintf(dummy_string, "main.cpp: Received: (%.*s)\n", result, rbuff_ptr);
            printf(dummy_string);

            received_bytes += result;
            rbuff_ptr += result;
        }
    }    
    //todo: add datacheck

    printf("\r\n\r\nReceived buffer contents: (%.*s)\r\n", needed_to_receive, &rbuffer[0]);

    //Close the socket to return its memory and bring down the network interface
    printf("\r\n\r\nEntering endless loop without closing socket\r\n");
    while(1);
    // socket.close();
}

int main(void)
{       
    printf("\r\n\r\nHello world!\r\n");
    wait(0.6);

    // DigitalOut lp_pin(P0_23, 1);
    // lp_pin.write(1);

    //testing recv functitnality
    stress_test1_recv();
    while(1);

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

    wait(2);

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

// #if 1
//     printf("Erasing SPI flash...\n");
//     BlockDevice *bd = BlockDevice::get_default_instance();
//     int err = bd->init();
//     printf("Init %s\n", (err ? "Fail :(" : "OK"));
//     if (!err) {
//         bd_size_t fl_size = bd->size();
//         err = bd->erase(0, fl_size);
//         printf("%s\n", (err ? "Fail :(" : "OK"));
//     }
//     bd->deinit();                 
// #endif

// #endif

// #if 0    
//     wait(0.5);    
//     for (;;) {
//         printf("tick!\r\n");
//         wait(0.5);        
//     }
// #endif   
    
    mcc_platform_run_program(main_application);
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

    // Initialize trace-library first
    if (application_init_mbed_trace() != 0) {
        printf("Failed initializing mbed trace\n" );
        return;
    }

    // Initialize storage
    if (mcc_platform_storage_init() != 0) {
        printf("Failed to initialize storage\n" );
        return;
    }

    // Initialize platform-specific components
    if(mcc_platform_init() != 0) {
        printf("ERROR - platform_init() failed!\n");
        return;
    }

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

            
    // Save pointer to mbedClient so that other functions can access it.
    client = &mbedClient;

    printf("Client initialized\r\n");
#ifdef MBED_HEAP_STATS_ENABLED
    print_heap_stats();
#endif
#ifdef MBED_STACK_STATS_ENABLED
    print_stack_statistics();
#endif

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

    mbedClient.register_and_connect();

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
