/**
 * @file motor_control.h
 * @brief Core 0 motor control interface
 * 
 * V2.00.00: User request: "plan out motor control system"
 * Plan: docs/thoughtprocesses/2026-01-22_overnight-motor-control-planning.md
 * 
 * PLANNING DOCUMENT - NOT YET IMPLEMENTED
 * 
 * CRITICAL: All functions here execute in Core 0 real-time context
 * DO NOT call CAN, Serial, or any blocking operations
 */

#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <stdint.h>
#include "can_protocol.h"

// ============================================================================
// CONFIGURATION
// ============================================================================

#define CONTROL_LOOP_FREQ_HZ    2000
#define CONTROL_LOOP_PERIOD_US  (1000000 / CONTROL_LOOP_FREQ_HZ)

#define MOTOR_COUNT             2

// Physical limits
#define MAX_VELOCITY_RPM        120.0f
#define MAX_ACCEL_RPM_S         1000.0f
#define MAX_CURRENT_AMPS        5.0f
#define MAX_POSITION_COUNTS     1000000

// Default PID gains (conservative starting point)
#define DEFAULT_KP              1.0f
#define DEFAULT_KI              0.1f
#define DEFAULT_KD              0.01f

// ============================================================================
// MOTOR STATE
// ============================================================================

typedef struct {
    // Actual state (from sensors)
    int32_t position_counts;
    float velocity_rpm;
    float current_amps;
    
    // Target state (from commands)
    float target_velocity_rpm;
    int32_t target_position_counts;
    
    // Control state
    float pwm_duty;  // -1.0 to +1.0
    
    // PID state
    float error_integral;
    float prev_error;
    
    // Gains
    float kp;
    float ki;
    float kd;
    
    // Flags
    bool enabled;
    bool at_target;
    
    // Error tracking
    uint16_t error_count;
    uint32_t fault_flags;
    
} motor_state_t;

// ============================================================================
// SHARED DATA (Accessed from both cores via lock-free mechanism)
// ============================================================================

typedef struct {
    // System state
    system_state_t system_state;
    control_mode_t control_mode;
    
    // Motor states
    motor_state_t motors[MOTOR_COUNT];
    
    // Configuration
    float accel_limit_rpm_s;
    float current_limit_amps;
    
    // Timing
    uint32_t loop_count;
    uint32_t loop_time_us;
    uint32_t max_loop_time_us;
    uint8_t loop_overruns;
    
    // Watchdog
    uint32_t last_command_time_ms;
    
    // Home position
    int32_t home_position[MOTOR_COUNT];
    bool home_set;
    
} control_shared_data_t;

// ============================================================================
// CORE 0 API (Real-time control loop)
// ============================================================================

/**
 * @brief Initialize motor control system
 * @return 0 on success, negative on error
 */
int motor_control_init(void);

/**
 * @brief Main control loop (runs on Core 0 at 2kHz)
 * 
 * Call sequence:
 * 1. Read sensors (encoders, current)
 * 2. Compute velocity (filtered derivative)
 * 3. Check safety limits
 * 4. Run PID controllers
 * 5. Apply outputs (PWM)
 * 6. Update shared data
 * 
 * CRITICAL: Must complete in < 500us
 * DO NOT call any blocking functions
 */
void motor_control_loop(void);

/**
 * @brief Emergency stop (called from any context)
 * Sets PWM to 0, disables motors, transitions to ESTOP
 */
void motor_control_emergency_stop(void);

// ============================================================================
// COMMAND INTERFACE (Called from Core 1, thread-safe)
// ============================================================================

/**
 * @brief Enable/disable motors
 * @param motor_id Motor index (0 or 1)
 * @param enable True to enable, false to disable
 * @return 0 on success, negative on error
 */
int motor_control_set_enable(uint8_t motor_id, bool enable);

/**
 * @brief Set target velocity (velocity mode)
 * @param motor_id Motor index
 * @param velocity_rpm Target velocity in RPM
 * @return 0 on success, negative on error
 */
int motor_control_set_velocity(uint8_t motor_id, float velocity_rpm);

/**
 * @brief Set target position (position mode)
 * @param motor_id Motor index
 * @param position_counts Target position in encoder counts
 * @return 0 on success, negative on error
 */
int motor_control_set_position(uint8_t motor_id, int32_t position_counts);

/**
 * @brief Update PID gains
 * @param motor_id Motor index (2 = both motors)
 * @param kp Proportional gain
 * @param ki Integral gain
 * @param kd Derivative gain
 * @return 0 on success, negative on error
 */
int motor_control_set_pid_gains(uint8_t motor_id, float kp, float ki, float kd);

/**
 * @brief Zero encoder positions
 * @return 0 on success, negative on error
 */
int motor_control_reset_position(void);

/**
 * @brief Set control mode
 * @param mode New control mode
 * @return 0 on success, negative on error
 */
int motor_control_set_mode(control_mode_t mode);

/**
 * @brief Set acceleration limit
 * @param limit Acceleration limit in RPM/s
 * @return 0 on success, negative on error
 */
int motor_control_set_accel_limit(float limit);

/**
 * @brief Set current limit
 * @param limit Current limit in Amps
 * @return 0 on success, negative on error
 */
int motor_control_set_current_limit(float limit);

/**
 * @brief Clear fault state
 * @return 0 on success, negative on error
 */
int motor_control_clear_fault(void);

/**
 * @brief Set home position
 * @param motor_id Motor index
 * @param position Home position in counts
 * @return 0 on success, negative on error
 */
int motor_control_set_home(uint8_t motor_id, int32_t position);

/**
 * @brief Go to home position
 * @return 0 on success, negative on error
 */
int motor_control_go_to_home(void);

// ============================================================================
// STATUS QUERY (Called from Core 1, thread-safe)
// ============================================================================

/**
 * @brief Get current system state
 * @return System state
 */
system_state_t motor_control_get_state(void);

/**
 * @brief Get motor position
 * @param motor_id Motor index
 * @return Position in encoder counts
 */
int32_t motor_control_get_position(uint8_t motor_id);

/**
 * @brief Get motor velocity
 * @param motor_id Motor index
 * @return Velocity in RPM
 */
float motor_control_get_velocity(uint8_t motor_id);

/**
 * @brief Get motor current
 * @param motor_id Motor index
 * @return Current in Amps
 */
float motor_control_get_current(uint8_t motor_id);

/**
 * @brief Get fault flags
 * @return Bitfield of active faults
 */
uint32_t motor_control_get_faults(void);

/**
 * @brief Get loop timing metrics
 * @param avg_us Average loop time in microseconds (output)
 * @param max_us Maximum loop time in microseconds (output)
 * @param overruns Number of loop overruns (output)
 */
void motor_control_get_timing(uint32_t* avg_us, uint32_t* max_us, uint8_t* overruns);

/**
 * @brief Check if motor is at target position
 * @param motor_id Motor index
 * @return True if at target (within tolerance)
 */
bool motor_control_at_target(uint8_t motor_id);

// ============================================================================
// SAFETY
// ============================================================================

/**
 * @brief Check all safety limits (called in control loop)
 * @return True if all limits OK, false if fault detected
 */
bool motor_control_check_limits(void);

/**
 * @brief Watchdog check (call periodically from Core 1)
 * Transitions to safe state if no commands received
 */
void motor_control_watchdog_check(void);

#endif // MOTOR_CONTROL_H
