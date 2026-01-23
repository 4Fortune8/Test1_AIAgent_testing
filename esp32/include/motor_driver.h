/**
 * @file motor_driver.h
 * @brief Motor driver interface (PWM and direction control)
 * 
 * V2.00.00: User request: "plan out motor control system"
 * Plan: docs/thoughtprocesses/2026-01-22_overnight-motor-control-planning.md
 * 
 * PLANNING DOCUMENT - NOT YET IMPLEMENTED
 * 
 * Supports common motor drivers like:
 * - L298N (IN1/IN2 for direction, ENA for PWM)
 * - TB6612FNG (similar interface)
 * - DRV8833 (dual H-bridge)
 */

#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/ledc.h"

// ============================================================================
// CONFIGURATION
// ============================================================================

#define MOTOR_COUNT             2

// PWM configuration
#define PWM_FREQUENCY_HZ        20000   // 20kHz (above audible range)
#define PWM_RESOLUTION          LEDC_TIMER_10_BIT  // 0-1023
#define PWM_MAX_DUTY            1023

// Motor 1 pins (example - adjust based on actual wiring)
#define MOTOR1_PWM_PIN          GPIO_NUM_16
#define MOTOR1_DIR_PIN1         GPIO_NUM_17
#define MOTOR1_DIR_PIN2         GPIO_NUM_18
#define MOTOR1_PWM_CHANNEL      LEDC_CHANNEL_0

// Motor 2 pins
#define MOTOR2_PWM_PIN          GPIO_NUM_19
#define MOTOR2_DIR_PIN1         GPIO_NUM_21
#define MOTOR2_DIR_PIN2         GPIO_NUM_22
#define MOTOR2_PWM_CHANNEL      LEDC_CHANNEL_1

// Current sensing (optional, if available)
#define MOTOR1_CURRENT_ADC      ADC1_CHANNEL_6  // GPIO34
#define MOTOR2_CURRENT_ADC      ADC1_CHANNEL_7  // GPIO35
#define CURRENT_SENSE_ENABLED   1

// Current sensor calibration
#define CURRENT_SENSE_OHMS      0.1f    // Shunt resistor value
#define CURRENT_SENSE_GAIN      20.0f   // Op-amp gain
#define ADC_VREF                3.3f    // ADC reference voltage
#define ADC_MAX_VALUE           4095    // 12-bit ADC

// ============================================================================
// TYPES
// ============================================================================

typedef enum {
    MOTOR_DIR_FORWARD = 0,
    MOTOR_DIR_REVERSE = 1,
    MOTOR_DIR_BRAKE   = 2,
    MOTOR_DIR_COAST   = 3
} motor_direction_t;

typedef struct {
    bool enabled;
    motor_direction_t direction;
    float duty_cycle;       // 0.0 to 1.0
    float current_amps;
    bool overcurrent;
} motor_driver_state_t;

// ============================================================================
// API
// ============================================================================

/**
 * @brief Initialize motor drivers
 * @return 0 on success, negative on error
 */
int motor_driver_init(void);

/**
 * @brief Set motor PWM duty cycle and direction
 * 
 * @param motor_id Motor index (0 or 1)
 * @param duty Duty cycle from -1.0 (full reverse) to +1.0 (full forward)
 * @return 0 on success, negative on error
 * 
 * Examples:
 *   duty = +0.5  → 50% forward
 *   duty = -0.75 → 75% reverse
 *   duty = 0.0   → stopped (brake)
 */
int motor_driver_set_duty(uint8_t motor_id, float duty);

/**
 * @brief Enable motor driver output
 * @param motor_id Motor index
 * @param enable True to enable, false to disable (coast)
 * @return 0 on success, negative on error
 */
int motor_driver_enable(uint8_t motor_id, bool enable);

/**
 * @brief Emergency stop - immediately set all motors to brake
 * Can be called from ISR context
 */
void motor_driver_emergency_stop(void);

/**
 * @brief Read motor current (if current sensing enabled)
 * @param motor_id Motor index
 * @return Current in Amps, or 0.0 if sensing not available
 */
float motor_driver_get_current(uint8_t motor_id);

/**
 * @brief Check if motor is in overcurrent condition
 * @param motor_id Motor index
 * @param threshold Current threshold in Amps
 * @return True if current exceeds threshold
 */
bool motor_driver_check_overcurrent(uint8_t motor_id, float threshold);

/**
 * @brief Get motor driver state
 * @param motor_id Motor index
 * @param state Output state structure
 * @return 0 on success, negative on error
 */
int motor_driver_get_state(uint8_t motor_id, motor_driver_state_t* state);

/**
 * @brief Set motor brake (short motor terminals)
 * @param motor_id Motor index
 * @return 0 on success, negative on error
 */
int motor_driver_brake(uint8_t motor_id);

/**
 * @brief Set motor coast (open motor terminals)
 * @param motor_id Motor index
 * @return 0 on success, negative on error
 */
int motor_driver_coast(uint8_t motor_id);

/**
 * @brief Calibrate current sensor zero point
 * Must be called with motors disabled and not moving
 * @param motor_id Motor index
 * @return 0 on success, negative on error
 */
int motor_driver_calibrate_current_sensor(uint8_t motor_id);

// ============================================================================
// LOW-LEVEL HARDWARE CONTROL (Internal use)
// ============================================================================

/**
 * @brief Set raw PWM duty cycle (0-PWM_MAX_DUTY)
 * @param motor_id Motor index
 * @param duty Raw duty cycle value
 * @return 0 on success, negative on error
 */
int motor_driver_set_raw_pwm(uint8_t motor_id, uint16_t duty);

/**
 * @brief Set motor direction pins directly
 * @param motor_id Motor index
 * @param dir Direction enum
 * @return 0 on success, negative on error
 */
int motor_driver_set_direction(uint8_t motor_id, motor_direction_t dir);

/**
 * @brief Read raw ADC value for current sensor
 * @param motor_id Motor index
 * @return Raw ADC value (0-4095)
 */
uint16_t motor_driver_read_current_adc(uint8_t motor_id);

/**
 * @brief Convert ADC value to current in Amps
 * @param adc_value Raw ADC value
 * @return Current in Amps
 */
float motor_driver_adc_to_current(uint16_t adc_value);

#endif // MOTOR_DRIVER_H
