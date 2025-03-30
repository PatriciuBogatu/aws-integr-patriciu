#include <functional>
#include "wifi/AppWifi.hpp"
#include "sensor/SensorApp.hpp"
#include "AppAwsClient.hpp"
#include "esp_log.h"

namespace
{
    app::AppWifi app_wifi;
    app::AppAwsClient app_client;
    app::AppSensor app_sensor{true, &app_client};
}

extern "C" void app_main()
{
    app_wifi.init();
    app_client.init();

    app_wifi.connect();
    app_client.setWiFiStatus(true);
    app_sensor.init();
}