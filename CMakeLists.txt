cmake_minimum_required(VERSION 3.10)
project(CODEFEST_AD_ASTRA_2024 VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Buscar OpenSSL usando pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(OPENSSL REQUIRED openssl)

# Directorios de inclusión
include_directories(
        ${OPENSSL_INCLUDE_DIRS}
)

# Archivos fuente
set(SOURCE_FILES
        app.cpp
)

# Añade el ejecutable
add_executable(CODEFEST_AD_ASTRA_2024 ${SOURCE_FILES})

# Vincular OpenSSL y otras bibliotecas
target_link_libraries(CODEFEST_AD_ASTRA_2024
        ${OPENSSL_LIBRARIES}
)

# Instalar el ejecutable
install(TARGETS CODEFEST_AD_ASTRA_2024 RUNTIME DESTINATION bin)
