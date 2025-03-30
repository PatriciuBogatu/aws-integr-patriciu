cmake_minimum_required(VERSION 3.5)

# path to esp-iot repo
if(DEFINED ENV{AWSIOT_PATH})
  set(AWSIOT_PATH $ENV{AWSIOT_PATH})
else()
  set(AWSIOT_PATH ${CMAKE_CURRENT_LIST_DIR}/../common/esp-aws-iot)
endif(DEFINED ENV{AWSIOT_PATH})

# contained in start.sh
# $env:AWS_ENDPOINT = "a21os9k1lkh5wn-ats.iot.eu-central-1.amazonaws.com"
# $env:AWSIOT_PATH = "C:/Users/Patriciu/Documents/esp32/esp-aws-iot"
if(DEFINED ENV{AWS_ENDPOINT})
    message("AWS IoT endpoint: " $ENV{AWS_ENDPOINT})
    add_compile_options(-DAWS_ENDPOINT="\"$ENV{AWS_ENDPOINT}\"")
else()
    message(FATAL_ERROR "Define AWS_ENDPOINT")
endif(DEFINED ENV{AWS_ENDPOINT})


set(EXTRA_COMPONENT_DIRS 
						 "${AWSIOT_PATH}/libraries/coreMQTT"
						 "${AWSIOT_PATH}/libraries/common/posix_compat"
   )

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(aws_iot)

target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "tmp/root_cert_auth.crt" TEXT)
target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "tmp/client.crt" TEXT)
target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "tmp/client.key" TEXT)