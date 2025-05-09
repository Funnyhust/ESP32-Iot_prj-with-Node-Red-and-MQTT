idf_component_register(
    SRCS "main.c" "i2c_init/i2c_init.c" "aht20/aht20.c" "bmp280/bmp280_read.c" "sensor_read/sensor_read.c"
    INCLUDE_DIRS "." "aht20" "bmp280"
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-const-variable)