/**
 * @file pid_controller.h
 * @brief PID controller implementation for motor control
 * 
 * V2.00.00: User request: "plan out motor control system"
 * Plan: docs/thoughtprocesses/2026-01-22_overnight-motor-control-planning.md
 * 
 * PLANNING DOCUMENT - NOT YET IMPLEMENTED
 * 
 * Implements discrete PID control with:
 * - Anti-windup (integral clamping)
 * - Derivative filtering
 * - Setpoint ramping
 * - Output limiting
 */

#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// CONFIGURATION
// ============================================================================

#define PID_OUTPUT_MIN          -1.0f
#define PID_OUTPUT_MAX          1.0f

#define PID_INTEGRAL_MAX        100.0f  // Anti-windup limit

#define PID_DERIVATIVE_FILTER   0.1f    // Low-pass filter coefficient

// ============================================================================
// PID CONTROLLER STATE
// ============================================================================

typedef struct {
    // Gains
    float kp;
    float ki;
    float kd;
    
    // State
    float setpoint;
    float previous_error;
    float integral;
    float derivative_filtered;
    
    // Output
    float output;
    
    // Terms (for debugging)
    float p_term;
    float i_term;
    float d_term;
    
    // Configuration
    float output_min;
    float output_max;
    float integral_max;
    
    // Flags
    bool enabled;
    bool integral_clamping;
    
    // Statistics
    uint32_t update_count;
    
} pid_controller_t;

// ============================================================================
// API
// ============================================================================

/**
 * @brief Initialize PID controller
 * @param pid Controller instance
 * @param kp Proportional gain
 * @param ki Integral gain
 * @param kd Derivative gain
 */
void pid_init(pid_controller_t* pid, float kp, float ki, float kd);

/**
 * @brief Reset PID controller state
 * Clears integral, derivative, and previous error
 * @param pid Controller instance
 */
void pid_reset(pid_controller_t* pid);

/**
 * @brief Update PID controller
 * 
 * Call at fixed sample rate (e.g., 2kHz control loop)
 * 
 * @param pid Controller instance
 * @param setpoint Target value
 * @param measurement Current value
 * @param dt Time step in seconds
 * @return Controller output (within output_min to output_max)
 */
float pid_update(pid_controller_t* pid, float setpoint, float measurement, float dt);

/**
 * @brief Set PID gains
 * @param pid Controller instance
 * @param kp Proportional gain
 * @param ki Integral gain
 * @param kd Derivative gain
 */
void pid_set_gains(pid_controller_t* pid, float kp, float ki, float kd);

/**
 * @brief Set output limits
 * @param pid Controller instance
 * @param min Minimum output
 * @param max Maximum output
 */
void pid_set_limits(pid_controller_t* pid, float min, float max);

/**
 * @brief Enable/disable integral clamping (anti-windup)
 * @param pid Controller instance
 * @param enable True to enable clamping
 */
void pid_set_integral_clamping(pid_controller_t* pid, bool enable);

/**
 * @brief Get current controller output
 * @param pid Controller instance
 * @return Output value
 */
static inline float pid_get_output(const pid_controller_t* pid) {
    return pid->output;
}

/**
 * @brief Get P, I, D terms for debugging
 * @param pid Controller instance
 * @param p_term Output: P term
 * @param i_term Output: I term
 * @param d_term Output: D term
 */
void pid_get_terms(const pid_controller_t* pid, float* p_term, float* i_term, float* d_term);

/**
 * @brief Enable/disable controller
 * @param pid Controller instance
 * @param enable True to enable
 */
void pid_set_enabled(pid_controller_t* pid, bool enable);

/**
 * @brief Check if controller is enabled
 * @param pid Controller instance
 * @return True if enabled
 */
static inline bool pid_is_enabled(const pid_controller_t* pid) {
    return pid->enabled;
}

// ============================================================================
// ADVANCED FEATURES
// ============================================================================

/**
 * @brief Set feedforward term
 * Adds a feedforward component to the output
 * Useful for velocity control with known system dynamics
 * 
 * @param pid Controller instance
 * @param feedforward Feedforward value
 */
void pid_set_feedforward(pid_controller_t* pid, float feedforward);

/**
 * @brief Compute velocity-form PID
 * Alternative PID formulation that computes change in output
 * Eliminates derivative kick on setpoint changes
 * 
 * @param pid Controller instance
 * @param setpoint Target value
 * @param measurement Current value
 * @param dt Time step in seconds
 * @return Change in controller output
 */
float pid_update_velocity_form(pid_controller_t* pid, float setpoint, float measurement, float dt);

/**
 * @brief Auto-tune PID gains using Ziegler-Nichols method
 * 
 * Procedure:
 * 1. Set Ki=0, Kd=0
 * 2. Increase Kp until sustained oscillation
 * 3. Measure oscillation period Tu and critical gain Ku
 * 4. Compute gains: Kp=0.6*Ku, Ki=2*Kp/Tu, Kd=Kp*Tu/8
 * 
 * @param pid Controller instance
 * @param ku Critical gain (from manual tuning)
 * @param tu Oscillation period in seconds (from manual tuning)
 */
void pid_autotune_ziegler_nichols(pid_controller_t* pid, float ku, float tu);

#endif // PID_CONTROLLER_H
