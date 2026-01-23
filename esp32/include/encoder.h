/**
 * @file encoder.h
 * @brief Quadrature encoder interface using ESP32 PCNT
 * 
 * V2.00.00: User request: "plan out motor control system"
 * Plan: docs/thoughtprocesses/2026-01-22_overnight-motor-control-planning.md
 * 
 * PLANNING DOCUMENT - NOT YET IMPLEMENTED
 */

#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// CONFIGURATION
// ============================================================================

#define ENCODER_COUNT           2

// Hardware pins (example - adjust based on actual wiring)
#define ENCODER1_PIN_A          GPIO_NUM_25
#define ENCODER1_PIN_B          GPIO_NUM_26

#define ENCODER2_PIN_A          GPIO_NUM_27
#define ENCODER2_PIN_B          GPIO_NUM_14

// PCNT configuration
#define ENCODER_PCNT_UNIT_0     PCNT_UNIT_0
#define ENCODER_PCNT_UNIT_1     PCNT_UNIT_1

#define ENCODER_FILTER_VALUE    100  // Filter pulses shorter than 100 APB_CLK cycles

// Encoder specifications
#define ENCODER_CPR             4096  // Counts per revolution (example)
#define ENCODER_MAX_RPM         200   // Maximum measurable RPM

// ============================================================================
// TYPES
// ============================================================================

typedef struct {
    int32_t position;       // Cumulative position in counts
    int32_t velocity;       // Velocity in counts/sec
    bool error;             // Encoder error flag
    uint16_t overflow_count; // Number of counter overflows
} encoder_state_t;

// ============================================================================
// API
// ============================================================================

/**
 * @brief Initialize encoders
 * @return 0 on success, negative on error
 */
int encoder_init(void);

/**
 * @brief Read encoder position (thread-safe, can call from any core)
 * @param encoder_id Encoder index (0 or 1)
 * @return Current position in counts
 */
int32_t encoder_get_position(uint8_t encoder_id);

/**
 * @brief Read encoder velocity
 * @param encoder_id Encoder index
 * @return Velocity in counts/sec
 */
int32_t encoder_get_velocity(uint8_t encoder_id);

/**
 * @brief Reset encoder position to zero
 * @param encoder_id Encoder index
 * @return 0 on success, negative on error
 */
int encoder_reset_position(uint8_t encoder_id);

/**
 * @brief Set encoder position to specific value
 * @param encoder_id Encoder index
 * @param position New position value
 * @return 0 on success, negative on error
 */
int encoder_set_position(uint8_t encoder_id, int32_t position);

/**
 * @brief Check for encoder errors
 * @param encoder_id Encoder index
 * @return True if error detected
 */
bool encoder_has_error(uint8_t encoder_id);

/**
 * @brief Clear encoder error flag
 * @param encoder_id Encoder index
 */
void encoder_clear_error(uint8_t encoder_id);

/**
 * @brief Update velocity measurement (call from control loop at fixed rate)
 * 
 * Computes velocity using position difference method:
 * velocity = (position_now - position_last) / dt
 * 
 * @param encoder_id Encoder index
 * @param dt_us Time since last update in microseconds
 */
void encoder_update_velocity(uint8_t encoder_id, uint32_t dt_us);

/**
 * @brief Get raw PCNT counter value (for debugging)
 * @param encoder_id Encoder index
 * @return Raw counter value (-32768 to +32767)
 */
int16_t encoder_get_raw_count(uint8_t encoder_id);

/**
 * @brief Convert encoder counts to RPM
 * @param counts_per_sec Velocity in counts/sec
 * @return Velocity in RPM
 */
static inline float encoder_counts_to_rpm(int32_t counts_per_sec) {
    return (counts_per_sec * 60.0f) / ENCODER_CPR;
}

/**
 * @brief Convert RPM to encoder counts/sec
 * @param rpm Velocity in RPM
 * @return Velocity in counts/sec
 */
static inline int32_t encoder_rpm_to_counts(float rpm) {
    return (int32_t)((rpm * ENCODER_CPR) / 60.0f);
}

/**
 * @brief Convert encoder counts to revolutions
 * @param counts Position in encoder counts
 * @return Position in revolutions
 */
static inline float encoder_counts_to_revs(int32_t counts) {
    return (float)counts / ENCODER_CPR;
}

/**
 * @brief Convert revolutions to encoder counts
 * @param revs Position in revolutions
 * @return Position in counts
 */
static inline int32_t encoder_revs_to_counts(float revs) {
    return (int32_t)(revs * ENCODER_CPR);
}

// ============================================================================
// ISR HANDLERS (Internal - do not call directly)
// ============================================================================

/**
 * @brief PCNT overflow interrupt handler
 * Increments/decrements overflow counter to extend 16-bit PCNT to 32-bit
 */
void IRAM_ATTR encoder_pcnt_overflow_isr(void* arg);

#endif // ENCODER_H
