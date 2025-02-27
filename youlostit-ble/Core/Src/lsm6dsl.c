#include "lsm6dsl.h"
#include "i2c.h"

#define LSM6DSL_ADDR 0x6A // I2C address of the LSM6DSL accelerometer
#define CTRL1_XL 0x10     // Control register for accelerometer
#define INT1_CTRL 0x0D    // Interrupt control register

#define OUTX_L_XL 0x28    // Output register for X-axis acceleration (low byte)

// #define OUTX_H_XL 0x29    // Output register for X-axis acceleration (high byte)
// #define OUTY_L_XL 0x2A    // Output register for Y-axis acceleration (low byte)
// #define OUTY_H_XL 0x2B    // Output register for Y-axis acceleration (high byte)
// #define OUTZ_L_XL 0x2C    // Output register for Z-axis acceleration (low byte)
// #define OUTZ_H_XL 0x2D    // Output register for Z-axis acceleration (high byte)

#define write 0
#define read 1

void lsm6dsl_init() {
    uint8_t data[2];

    // Configure accelerometer: 416 Hz, High-Performance mode
    data[0] = CTRL1_XL;  // Register address
    data[1] = 0x60;       // Configuration value
    i2c_transaction(LSM6DSL_ADDR, write, data, 2); // Send register address + data
    
    // Enable accelerometer data-ready interrupt on INT1
    data[0] = INT1_CTRL;  // Register address
    data[1] = 0x01;       // Enable data-ready interrupt
    i2c_transaction(LSM6DSL_ADDR, write, data, 2); // Send register address + data
}

void lsm6dsl_read_xyz(int16_t* x, int16_t* y, int16_t* z) {

    uint8_t reg = OUTX_L_XL; 
    uint8_t data[6];
    
    // Set register address for reading
    // set write
    i2c_transaction(LSM6DSL_ADDR, write, &reg, 1);
    
    // Read 6 bytes of acceleration data (X, Y, Z - Low and High bytes)
    i2c_transaction(LSM6DSL_ADDR, read, data, 6);

    
    
    // Convert raw data into 16-bit values
    *x = (int16_t)((data[1] << 8) | data[0]);
    *y = (int16_t)((data[3] << 8) | data[2]);
    *z = (int16_t)((data[5] << 8) | data[4]);
}
