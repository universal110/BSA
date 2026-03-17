
#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <math.h>
#include "sensor.h"
#include <zephyr/sys/printk.h>

static const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));



int sensorInit(void) {

    if (!device_is_ready(i2c_dev)) {
        printk("I2C device not ready\n");
        return -EIO;
    }

    // i2c_configure() sets up the I2C bus with specific settings.

    int rv = i2c_configure(i2c_dev, I2C_MODE_CONTROLLER | I2C_SPEED_SET(I2C_SPEED_FAST));
    if (rv < 0) {
        printk("I2C configuration failed\n");
        return rv;
    }

    // The MPU-6050 starts in sleep mode by default to save power.
    // We must wake it up before it will respond to any read requests.
    // i2c_reg_write_byte() writes a single byte to a specific register.
    // It takes four arguments:
    //   - the I2C device handle
    //   - the sensor's I2C address (0x68)
    //   - the register to write to (0x6B = power management register)
    //   - the value to write (0x00 = clear all bits = wake up)
    // Writing 0x00 to register 0x6B clears the sleep bit inside the sensor
    rv = i2c_reg_write_byte(i2c_dev, sensor_ADDR, sensor_PWR_MGMT, 0x00);
    if (rv < 0) {
        printk("Failed to wake MPU-6050\n");
        return rv;
    }

    k_msleep(100);

    printk("Sensor ready\n");
    return 0;
}


static int sensorReadAccel(int16_t *x, int16_t *y, int16_t *z) {

    // A buffer to hold the 6 raw bytes coming back from the sensor.
    // The accelerometer gives 2 bytes per axis (X, Y, Z) = 6 bytes total.

    uint8_t buf[6];

    int rv = i2c_burst_read(i2c_dev, sensor_ADDR, sensor_ACCEL_XOUT_H, buf, 6);
    if (rv < 0) {
        printk("Failed to read accelerometer\n");
        return rv;
    }

    uint8_t  x_high    = buf[0];             
    uint8_t  x_low     = buf[1];             
    uint16_t x_shifted = x_high * 256;       
                                             
                                             
    int16_t  x_final   = x_shifted + x_low; 
    *x = x_final;                            

    uint8_t  y_high    = buf[2];
    uint8_t  y_low     = buf[3];
    uint16_t y_shifted = y_high * 256;
    int16_t  y_final   = y_shifted + y_low;
    *y = y_final;


    uint8_t  z_high    = buf[4];
    uint8_t  z_low     = buf[5];
    uint16_t z_shifted = z_high * 256;
    int16_t  z_final   = z_shifted + z_low;
    *z = z_final;

    return 0;
}



float sensorWristBend(void) {

    int16_t ax, ay, az;
    int rv = sensorReadAccel(&ax, &ay, &az);
    if (rv < 0) {

        return 0.0f;
    }

    float wristBend = atan2f((float)ay, (float)az) * (180.0f / (3.14159265f));

    return wristBend;
}


float sensorWristTurn(void) {

    // Same read pattern as sensorWristBend()
    int16_t ax, ay, az;
    int rv = sensorReadAccel(&ax, &ay, &az);
    if (rv < 0) {
        return 0.0f;
    }

    // For roll (wrist turn) we compare X axis to Z axis instead of Y to Z
    // Everything else is identical to the pitch calculation above
    float wristTurn = atan2f((float)ax, (float)az) * (180.0f / 3.14159265f);

    return wristTurn;
}



float sensorGetDeviation(float sidewaysTilt, float wristBend) {

    // Get the current wrist orientation from the sensor
    float currentTurn = sensorWristTurn();
    float currentBend = sensorWristBend();

    // Calculate how much each angle changed from the baseline
    // sidewaysTilt is the baseline_roll recorded at session start
    // wristBend is the baseline_pitch recorded at session start
    float diffTurn = currentTurn - sidewaysTilt;
    float diffBend = currentBend - wristBend;

    float deviation = sqrtf((diffTurn * diffTurn) + (diffBend * diffBend));

    return deviation;
}

float sensorGetMagnitude(void) {

    // Read raw accelerometer values
    int16_t ax, ay, az;
    int rv = sensorReadAccel(&ax, &ay, &az);
    if (rv < 0) {
        return 0.0f;
    }

    // Calculate total force across all three axes
    // When still this is ~16384 (just gravity)
    // A wrist flick spikes this significantly higher
    float magnitude = sqrtf(((float)ax * (float)ax) +
                            ((float)ay * (float)ay) +
                            ((float)az * (float)az));
    return magnitude;
}