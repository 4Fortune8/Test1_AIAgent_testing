/**
 * @file can_protocol.h
 * @brief CAN message definitions and protocol handling
 * 
 * V2.00.00: User request: "plan out motor control system"
 * Plan: docs/thoughtprocesses/2026-01-22_overnight-motor-control-planning.md
 * Protocol spec: protocol/motor_control_messages.md
 * 
 * PLANNING DOCUMENT - NOT YET IMPLEMENTED
 */

#ifndef CAN_PROTOCOL_H
#define CAN_PROTOCOL_H

#include <stdint.h>
#include "driver/twai.h"

// ============================================================================
// CAN MESSAGE IDS
// ============================================================================

// Command Messages (Pi → ESP32)
#define CAN_ID_SYSTEM_RESET        0x080
#define CAN_ID_MOTOR_ENABLE        0x081
#define CAN_ID_SET_VELOCITY        0x082
#define CAN_ID_SET_POSITION        0x083
#define CAN_ID_SET_PID_GAINS       0x084
#define CAN_ID_EMERGENCY_STOP      0x085
#define CAN_ID_RESET_POSITION      0x086
#define CAN_ID_SET_ACCEL_LIMIT     0x087
#define CAN_ID_SET_CURRENT_LIMIT   0x088
#define CAN_ID_CLEAR_FAULT         0x089
#define CAN_ID_SET_MODE            0x08A
#define CAN_ID_TRAJECTORY_POINT    0x08B
#define CAN_ID_TRAJECTORY_START    0x08C
#define CAN_ID_TRAJECTORY_STOP     0x08D
#define CAN_ID_SET_HOME_POSITION   0x08E
#define CAN_ID_GO_TO_HOME          0x08F
#define CAN_ID_REQUEST_STATUS      0x090
#define CAN_ID_PING                0x091

// Status Messages (ESP32 → Pi)
#define CAN_ID_HEARTBEAT           0x100
#define CAN_ID_MOTOR_STATUS        0x101
#define CAN_ID_POSITION_REPORT     0x102
#define CAN_ID_VELOCITY_REPORT     0x103
#define CAN_ID_FAULT_REPORT        0x104
#define CAN_ID_PID_DEBUG           0x105
#define CAN_ID_CURRENT_REPORT      0x106
#define CAN_ID_TEMPERATURE_REPORT  0x107
#define CAN_ID_BUFFER_STATUS       0x108
#define CAN_ID_LIMIT_SWITCH_STATUS 0x109
#define CAN_ID_ERROR_LOG           0x10A
#define CAN_ID_PERFORMANCE_METRICS 0x10B
#define CAN_ID_PONG                0x111

// Broadcast Messages
#define CAN_ID_GLOBAL_ESTOP        0x200
#define CAN_ID_WATCHDOG_ALERT      0x201
#define CAN_ID_SYNC_PULSE          0x202

// ============================================================================
// SYSTEM STATES
// ============================================================================

typedef enum {
    STATE_INIT       = 0x00,
    STATE_IDLE       = 0x01,
    STATE_ENABLED    = 0x02,
    STATE_RUNNING    = 0x03,
    STATE_FAULT      = 0x04,
    STATE_ESTOP      = 0x05,
    STATE_HOMING     = 0x06,
    STATE_TRAJECTORY = 0x07
} system_state_t;

// ============================================================================
// CONTROL MODES
// ============================================================================

typedef enum {
    MODE_VELOCITY   = 0x00,
    MODE_POSITION   = 0x01,
    MODE_TRAJECTORY = 0x02,
    MODE_TORQUE     = 0x03  // Future
} control_mode_t;

// ============================================================================
// FAULT FLAGS
// ============================================================================

#define FAULT_MOTOR1_OVERCURRENT    (1 << 0)
#define FAULT_MOTOR2_OVERCURRENT    (1 << 1)
#define FAULT_MOTOR1_OVERTEMP       (1 << 2)
#define FAULT_MOTOR2_OVERTEMP       (1 << 3)
#define FAULT_ENCODER1_ERROR        (1 << 4)
#define FAULT_ENCODER2_ERROR        (1 << 5)
#define FAULT_CAN_WATCHDOG          (1 << 6)
#define FAULT_POSITION_LIMIT        (1 << 7)
#define FAULT_VELOCITY_LIMIT        (1 << 8)
#define FAULT_ACCEL_LIMIT           (1 << 9)
#define FAULT_LOOP_OVERRUN          (1 << 10)
#define FAULT_MEMORY_ERROR          (1 << 11)
#define FAULT_CONFIG_ERROR          (1 << 12)

// ============================================================================
// MESSAGE STRUCTURES
// ============================================================================

// Commands
typedef struct {
    uint8_t motor1_enable : 1;
    uint8_t motor2_enable : 1;
    uint8_t brake1        : 1;
    uint8_t brake2        : 1;
    uint8_t reserved      : 4;
} motor_enable_msg_t;

typedef struct {
    float motor1_velocity_rpm;
    float motor2_velocity_rpm;
} set_velocity_msg_t;

typedef struct {
    int32_t motor1_position;
    int32_t motor2_position;
} set_position_msg_t;

typedef struct {
    uint8_t motor_select;  // 0=Motor1, 1=Motor2, 2=Both
    float kp;
    float ki;
    float kd;
} set_pid_gains_msg_t;

typedef struct {
    uint8_t mode;  // control_mode_t
} set_mode_msg_t;

typedef struct {
    uint8_t sequence;
    uint32_t timestamp_ms;
    int16_t velocity1;
    int8_t velocity2_delta;
} trajectory_point_msg_t;

// Status
typedef struct {
    system_state_t state;
} heartbeat_msg_t;

typedef struct {
    uint16_t status_flags;
    uint16_t motor1_errors;
    uint16_t motor2_errors;
    uint16_t uptime_seconds;
} motor_status_msg_t;

typedef struct {
    int32_t motor1_position;
    int32_t motor2_position;
} position_report_msg_t;

typedef struct {
    float motor1_velocity_rpm;
    float motor2_velocity_rpm;
} velocity_report_msg_t;

typedef struct {
    uint32_t fault_flags;
} fault_report_msg_t;

typedef struct {
    float motor1_current_amps;
    float motor2_current_amps;
} current_report_msg_t;

// ============================================================================
// PROTOCOL API
// ============================================================================

/**
 * @brief Initialize CAN protocol handler
 * @return 0 on success, negative on error
 */
int can_protocol_init(void);

/**
 * @brief Process incoming CAN message (called from Core 1 RX task)
 * @param msg Received TWAI message
 * @return 0 on success, negative on error
 */
int can_protocol_process_rx(const twai_message_t* msg);

/**
 * @brief Send heartbeat message (called periodically from Core 1)
 * @param state Current system state
 * @return 0 on success, negative on error
 */
int can_protocol_send_heartbeat(system_state_t state);

/**
 * @brief Send motor status message
 * @param status Motor status data
 * @return 0 on success, negative on error
 */
int can_protocol_send_motor_status(const motor_status_msg_t* status);

/**
 * @brief Send position report
 * @param position Position data
 * @return 0 on success, negative on error
 */
int can_protocol_send_position(const position_report_msg_t* position);

/**
 * @brief Send velocity report
 * @param velocity Velocity data
 * @return 0 on success, negative on error
 */
int can_protocol_send_velocity(const velocity_report_msg_t* velocity);

/**
 * @brief Send fault report
 * @param faults Fault flags
 * @return 0 on success, negative on error
 */
int can_protocol_send_fault_report(uint32_t faults);

/**
 * @brief Send current report
 * @param current Current measurement data
 * @return 0 on success, negative on error
 */
int can_protocol_send_current(const current_report_msg_t* current);

/**
 * @brief Send PONG response to PING
 * @param sequence Sequence counter
 * @return 0 on success, negative on error
 */
int can_protocol_send_pong(uint32_t sequence);

/**
 * @brief Handle emergency stop (called from any context)
 * @return 0 on success, negative on error
 */
int can_protocol_handle_estop(void);

/**
 * @brief Get last command timestamp (for watchdog)
 * @return Timestamp in milliseconds
 */
uint32_t can_protocol_get_last_cmd_time(void);

// ============================================================================
// COMMAND CALLBACKS (Implemented in control layer)
// ============================================================================

// These functions are called by protocol handler and must be implemented
// in the control layer. They execute in Core 1 context.

extern int cmd_motor_enable(const motor_enable_msg_t* msg);
extern int cmd_set_velocity(const set_velocity_msg_t* msg);
extern int cmd_set_position(const set_position_msg_t* msg);
extern int cmd_set_pid_gains(const set_pid_gains_msg_t* msg);
extern int cmd_emergency_stop(void);
extern int cmd_reset_position(void);
extern int cmd_set_accel_limit(float limit);
extern int cmd_set_current_limit(float limit);
extern int cmd_clear_fault(void);
extern int cmd_set_mode(control_mode_t mode);
extern int cmd_trajectory_point(const trajectory_point_msg_t* msg);
extern int cmd_trajectory_start(void);
extern int cmd_trajectory_stop(void);
extern int cmd_set_home(const set_position_msg_t* msg);
extern int cmd_go_to_home(void);
extern int cmd_system_reset(void);

#endif // CAN_PROTOCOL_H
