#pragma once

// #include "bme280-new-i2c.hpp"
// #include "bsp/esp-bsp.h"  // For BSP I2C handle
// #include "driver/i2c_master.h"
#include "driver/gpio.h"
#include <vector>
#include <cstdio>
#include "esp_err.h"
#include "esp_log.h"
#include <string>
#include "wifi/json.hpp"
#include "sdkconfig.h"
#include <stdio.h>
#include <stdlib.h>  // For rand() and srand()
#include <time.h>
#include "../AppAwsClient.hpp"


static const char *TAG = "AppSensor";


namespace app {


    struct SensorReading {
        float temperature;
        float pressure;
        float humidity;
    };


    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SensorReading, temperature, pressure, humidity);

    class AppSensor {
    private:
        // i2c_bus_handle_t i2c_bus_ = NULL;
        // bme280_handle_t  bme280 = NULL;
        // i2c_master_bus_handle_t i2c_handle;
        // bme280_handle_t bme280_handle = NULL;


        nlohmann::json readings;
        bool m_enabled;

        
        AppAwsClient* awsClient;

        static void readTask(void* arg);

    public:
        bool isMock;
        AppSensor(bool isMock, AppAwsClient* awsClient)
        {
            readings["temparature"] = 0.0;
            readings["humidity"] = 0.0;
            readings["pressure"] = 0.0;
            this->isMock = isMock;
            this->awsClient = awsClient;
        }

        void init(void) {
            // const i2c_master_bus_config_t i2c_config = {
            //     .i2c_port = 0,
            //     .sda_io_num = GPIO_NUM_38,
            //     .scl_io_num = GPIO_NUM_39,
            //     .clk_source = I2C_CLK_SRC_DEFAULT,
            //     .glitch_ignore_cnt = 7,
            //     .intr_priority = 0,
            //     .trans_queue_depth = 0,
            //     .flags = 1
            // };

            // i2c_device_config_t bme280_conf = {
            //     .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            //     .device_address = 0x76,  // or 0x77
            //     .scl_speed_hz = 100000,
            // };

            // ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_config, &i2c_handle));

            // // Initialize BME280
            // bme280_default_init(bme280_handle);

            
            // if(!isMock){
            //     BME280_i2c_master_init();
            //     initializeBME280();
            // }

            xTaskCreate(readTask, "sensor", 3072, this, 5, nullptr);
        }

        

        void passToAWSReading(float reading){
            awsClient->update(reading);
        }

        
        // std::string getReadingsMQTT(void){ 
            
        //     SensorReading currentReading = read();
        //     readings["temparature"] = currentReading.temperature;
        //     readings["humidity"] = currentReading.humidity;
        //     readings["pressure"] = currentReading.pressure;

        //     return readings.dump(); 
        
        // }


        // void setState(std::string new_state)
        // {
        //     nlohmann::json val = nlohmann::json::parse(new_state, nullptr, false);
        //     if (!val.is_discarded())
        //     {
        //         readings.merge_patch(val);
        //         m_enabled = m_state["enabled"].get<bool>();
        //         handleNewState();
        //     }
        // }

    };


    void AppSensor::readTask(void* arg) { 

        AppSensor *obj = reinterpret_cast<AppSensor *>(arg);
      
        while(1){
        if(obj->isMock){
            srand(time(NULL));
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            // Generate a random number
            int random_number = rand() % 30;
            obj->passToAWSReading(random_number);
        }
        }
        
    }
}