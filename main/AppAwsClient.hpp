#pragma once

#include <string>
#include <sstream>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// these 3 libs come with aws iot sdk
#include "core_mqtt.h"
// provides tls-level connection to aws cloud infrastructure by using the device's cryptographic data
#include "network_transport.h"
#include "clock.h"
#include "sdkconfig.h"
#include "esp_err.h"
#include "esp_log.h"

extern "C"
{
    extern const char root_cert_auth_start[] asm("_binary_root_cert_auth_crt_start");
    extern const char root_cert_auth_end[] asm("_binary_root_cert_auth_crt_end");
    extern const char client_cert_start[] asm("_binary_client_crt_start");
    extern const char client_cert_end[] asm("_binary_client_crt_end");
    extern const char client_key_start[] asm("_binary_client_key_start");
    extern const char client_key_end[] asm("_binary_client_key_end");
}

namespace app
{

    class AppAwsClient
    {
    private:
        bool m_aws_connected{false};
        TaskHandle_t m_process_task{nullptr};

        std::string m_aws_endpoint{"a21os9k1lkh5wn-ats.iot.eu-central-1.amazonaws.com"};
        std::string m_thing_name{"my_temp_sensor"};
        std::string m_user_name{"any_user_name"};
        std::string m_topic_name{"my_temp_sensor/reading"};
        uint8_t m_local_buffer[1024];

        NetworkContext_t m_network_context;
        TransportInterface_t m_transport;
        
        MQTTConnectInfo_t m_connect_info;
        MQTTFixedBuffer_t m_network_buffer;
        MQTTPublishInfo_t m_publish_info;
        MQTTContext_t m_mqtt_context;

        static void processMqtt(void *arg)
        {
            ESP_LOGI("AppAwsClient", "Starting processMqtt task");
            AppAwsClient *obj = reinterpret_cast<AppAwsClient *>(arg);

            while (1)
            {
                vTaskDelay(100 / portTICK_PERIOD_MS);
                MQTT_ProcessLoop(&obj->m_mqtt_context);
            }
        }

        static void eventCallback(MQTTContext_t *pMqttContext,
                                  MQTTPacketInfo_t *pPacketInfo,
                                  MQTTDeserializedInfo_t *pDeserializedInfo)
        {
        }

    public:
        void init(void)
        {
            xTaskCreate(processMqtt, "pmqtt", 2048, this, 5, &m_process_task);
            vTaskSuspend(m_process_task);

            m_network_context.pcHostname = m_aws_endpoint.c_str();
            m_network_context.xPort = 8883;
            m_network_context.pxTls = NULL;
            m_network_context.xTlsContextSemaphore = xSemaphoreCreateRecursiveMutex();
            m_network_context.disableSni = 0;
            m_network_context.pcServerRootCA = root_cert_auth_start;
            m_network_context.pcServerRootCASize = root_cert_auth_end - root_cert_auth_start;
            m_network_context.pcClientCert = client_cert_start;
            m_network_context.pcClientCertSize = client_cert_end - client_cert_start;
            m_network_context.pcClientKey = client_key_start;
            m_network_context.pcClientKeySize = client_key_end - client_key_start;
            m_network_context.pAlpnProtos = NULL;

            m_transport.pNetworkContext = &m_network_context;
            m_transport.send = espTlsTransportSend;
            m_transport.recv = espTlsTransportRecv;
            m_transport.writev = nullptr;

            m_network_buffer.pBuffer = m_local_buffer;
            m_network_buffer.size = sizeof(m_local_buffer);

            m_connect_info.cleanSession = true;
            m_connect_info.pClientIdentifier = m_thing_name.c_str();
            m_connect_info.clientIdentifierLength = m_thing_name.length();
            m_connect_info.keepAliveSeconds = 60;
            m_connect_info.pUserName = m_user_name.c_str();
            m_connect_info.userNameLength = m_user_name.length();

            m_publish_info.qos = MQTTQoS0;
            m_publish_info.pTopicName = m_topic_name.c_str();
            m_publish_info.topicNameLength = m_topic_name.length();

            MQTT_Init(&m_mqtt_context, &m_transport, Clock_GetTimeMs, eventCallback, &m_network_buffer);
        }

        void setWiFiStatus(bool connected)
        {
            if (connected)
            {   
                const char* taskName = pcTaskGetName(xTaskGetCurrentTaskHandle());
                ESP_LOGI("AppAwsClient", "Setting WifiStatus to true from task: %s", taskName);
                
                if (xTlsConnect(&m_network_context) == TLS_TRANSPORT_SUCCESS)
                {   
                    ESP_LOGI("AppAwsClient", "TSL_TRANSPORT_SUCCESS");
                    
                    bool sess_present;
                    m_aws_connected = MQTT_Connect(&m_mqtt_context, &m_connect_info, nullptr, 1000, &sess_present) == MQTTSuccess;
                    if (!m_aws_connected)
                    {   ESP_LOGI("MAIN", "NOT ABLE TO CONNECT TO AWS");
                        vTaskSuspend(m_process_task);
                    }else{
                        vTaskResume(m_process_task);
                    }
                }
            }
            else
            {
                m_aws_connected = false;
                vTaskSuspend(m_process_task);
            }
        }

        void update(uint32_t temp_level) 
        {
            if (!m_aws_connected)
            {
                return;
            }

            ESP_LOGI("AppAwsClient", "Publishing temperature: %lu", temp_level);
            std::stringstream ss_mqtt_message;
            ss_mqtt_message << "{\"temp_level\":" << temp_level << "}";

            std::string payload = ss_mqtt_message.str();
            m_publish_info.pPayload = payload.c_str();
            m_publish_info.payloadLength = payload.length();

            uint16_t packet_id = MQTT_GetPacketId(&m_mqtt_context);
            MQTT_Publish(&m_mqtt_context, &m_publish_info, packet_id);
        }
    };

} // namespace app