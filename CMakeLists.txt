cmake_minimum_required(VERSION 3.10)
project(cpp-mqtt CXX)

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
# Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()


set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/paho.mqtt.cpp/cmake")
set(CMAKE_CXX_STANDARD 17)

set(PAHO_BUILD_STATIC TRUE)
set(PAHO_BUILD_SHARED FALSE)
set(PAHO_WITH_SSL TRUE)
set(PAHO_ENABLE_TESTING FALSE)
set(PAHO_MQTT_C_LIBRARIES "${PROJECT_SOURCE_DIR}/build/paho.mqtt.c/src")
set(PAHO_MQTT_C_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/paho.mqtt.c/src")

include_directories("paho.mqtt.c/src" "paho.mqtt.cpp/src")

link_directories(   "${PROJECT_SOURCE_DIR}/build/paho.mqtt.cpp/src"
                    "${PROJECT_SOURCE_DIR}/build/paho.mqtt.c/src"
                    "${PROJECT_SOURCE_DIR}/build/paho.mqtt.c/src/CMakeFiles/*"
                    "${PROJECT_SOURCE_DIR}/build/paho.mqtt.cpp/src/CMakeFiles/*")


add_subdirectory(paho.mqtt.c)
add_subdirectory(paho.mqtt.cpp)

find_package(Threads)
find_package(PahoMqttC)
find_package(OpenSSL)
find_package(woof)
find_package(caplets)

# add_executable(publish publish.cpp)
# target_link_libraries( publish
#         ${CMAKE_THREAD_LIBS_INIT}
#         ${PROJECT_SOURCE_DIR}/build/paho.mqtt.cpp/src/libpaho-mqttpp3.a
#         ${PROJECT_SOURCE_DIR}/build/paho.mqtt.c/src/libpaho-mqtt3as.a
#         ${OPENSSL_SSL_LIBRARIES}
#         ${OPENSSL_CRYPTO_LIBRARIES}
#         ${CMAKE_DL_LIBS}
#         )
add_handler(laptop_handler laptop_handler.cpp)

add_executable(consume consume.cpp)
target_link_libraries( consume
        ${CMAKE_THREAD_LIBS_INIT}
        ${PROJECT_SOURCE_DIR}/build/paho.mqtt.cpp/src/libpaho-mqttpp3.a
        ${PROJECT_SOURCE_DIR}/build/paho.mqtt.c/src/libpaho-mqtt3as.a
        ${OPENSSL_SSL_LIBRARIES}
        ${OPENSSL_CRYPTO_LIBRARIES}
        ${CMAKE_DL_LIBS} woof caplets
        )
add_executable(read_log read_log.cpp) 
target_link_libraries( read_log                                                                                                                                                                           ${CMAKE_THREAD_LIBS_INIT}                                                                                                                                                                        ${PROJECT_SOURCE_DIR}/build/paho.mqtt.cpp/src/libpaho-mqttpp3.a                                                                                                                                  ${PROJECT_SOURCE_DIR}/build/paho.mqtt.c/src/libpaho-mqtt3as.a                                                                                                                                    ${OPENSSL_SSL_LIBRARIES}                                                                                                                                                                         ${OPENSSL_CRYPTO_LIBRARIES}                                                                                                                                                                      ${CMAKE_DL_LIBS} woof caplets                                                                                                                                                                    )# add_executable(multithread multithread.cpp)
# target_link_libraries( multithread
#         ${CMAKE_THREAD_LIBS_INIT}
#         ${PROJECT_SOURCE_DIR}/build/paho.mqtt.cpp/src/libpaho-mqttpp3.a
#         ${PROJECT_SOURCE_DIR}/build/paho.mqtt.c/src/libpaho-mqtt3as.a
#         ${OPENSSL_SSL_LIBRARIES}
#         ${OPENSSL_CRYPTO_LIBRARIES}
#         ${CMAKE_DL_LIBS}
#         )

# add_custom_command(
#             TARGET cpp-mqtt POST_BUILD
#             COMMAND openssl req -new -newkey rsa:2048 -days 365 -nodes -x509 -subj "/C=US/ST=Denial/L=Springfield/O=Dis/CN=localhost" -keyout tls.key  -out tls.crt
#             VERBATIM)        
