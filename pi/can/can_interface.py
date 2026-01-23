"""
can_interface.py - CAN communication layer for Raspberry Pi

V2.00.00: User request: "plan out motor control system"
Plan: docs/thoughtprocesses/2026-01-22_overnight-motor-control-planning.md
Protocol: protocol/motor_control_messages.md

PLANNING DOCUMENT - NOT YET IMPLEMENTED
"""

import can
import struct
import threading
import time
from enum import IntEnum
from dataclasses import dataclass
from typing import Optional, Callable, Dict
from queue import Queue

# ============================================================================
# CAN MESSAGE IDS
# ============================================================================

class CANCommand(IntEnum):
    """Command messages (Pi → ESP32)"""
    SYSTEM_RESET = 0x080
    MOTOR_ENABLE = 0x081
    SET_VELOCITY = 0x082
    SET_POSITION = 0x083
    SET_PID_GAINS = 0x084
    EMERGENCY_STOP = 0x085
    RESET_POSITION = 0x086
    SET_ACCEL_LIMIT = 0x087
    SET_CURRENT_LIMIT = 0x088
    CLEAR_FAULT = 0x089
    SET_MODE = 0x08A
    TRAJECTORY_POINT = 0x08B
    TRAJECTORY_START = 0x08C
    TRAJECTORY_STOP = 0x08D
    SET_HOME_POSITION = 0x08E
    GO_TO_HOME = 0x08F
    REQUEST_STATUS = 0x090
    PING = 0x091

class CANStatus(IntEnum):
    """Status messages (ESP32 → Pi)"""
    HEARTBEAT = 0x100
    MOTOR_STATUS = 0x101
    POSITION_REPORT = 0x102
    VELOCITY_REPORT = 0x103
    FAULT_REPORT = 0x104
    PID_DEBUG = 0x105
    CURRENT_REPORT = 0x106
    TEMPERATURE_REPORT = 0x107
    BUFFER_STATUS = 0x108
    LIMIT_SWITCH_STATUS = 0x109
    ERROR_LOG = 0x10A
    PERFORMANCE_METRICS = 0x10B
    PONG = 0x111

# ============================================================================
# SYSTEM STATES
# ============================================================================

class SystemState(IntEnum):
    """ESP32 system states"""
    INIT = 0x00
    IDLE = 0x01
    ENABLED = 0x02
    RUNNING = 0x03
    FAULT = 0x04
    ESTOP = 0x05
    HOMING = 0x06
    TRAJECTORY = 0x07

class ControlMode(IntEnum):
    """Control modes"""
    VELOCITY = 0x00
    POSITION = 0x01
    TRAJECTORY = 0x02
    TORQUE = 0x03

# ============================================================================
# DATA STRUCTURES
# ============================================================================

@dataclass
class MotorPosition:
    """Position report from ESP32"""
    motor1_pos: int
    motor2_pos: int
    timestamp: float

@dataclass
class MotorVelocity:
    """Velocity report from ESP32"""
    motor1_vel: float
    motor2_vel: float
    timestamp: float

@dataclass
class MotorStatus:
    """Motor status report"""
    status_flags: int
    motor1_errors: int
    motor2_errors: int
    uptime_seconds: int
    timestamp: float

@dataclass
class FaultReport:
    """Fault flags"""
    fault_flags: int
    timestamp: float

@dataclass
class CurrentReport:
    """Current measurements"""
    motor1_current: float
    motor2_current: float
    timestamp: float

# ============================================================================
# CAN INTERFACE
# ============================================================================

class CANInterface:
    """
    CAN communication interface for motor control system
    
    Handles:
    - Message encoding/decoding
    - Command transmission
    - Status reception
    - Watchdog monitoring
    - Callback dispatch
    """
    
    def __init__(self, interface: str = 'can0', bitrate: int = 500000):
        """
        Initialize CAN interface
        
        Args:
            interface: CAN interface name (default 'can0')
            bitrate: CAN bitrate in bps (default 500000)
        """
        self.interface = interface
        self.bitrate = bitrate
        self.bus: Optional[can.Bus] = None
        
        # Reception thread
        self.rx_thread: Optional[threading.Thread] = None
        self.running = False
        
        # Status callbacks
        self.callbacks: Dict[int, Callable] = {}
        
        # Latest status cache
        self.last_heartbeat: Optional[SystemState] = None
        self.last_position: Optional[MotorPosition] = None
        self.last_velocity: Optional[MotorVelocity] = None
        self.last_motor_status: Optional[MotorStatus] = None
        self.last_fault: Optional[FaultReport] = None
        self.last_current: Optional[CurrentReport] = None
        
        # Watchdog
        self.last_heartbeat_time = 0.0
        self.heartbeat_timeout = 0.5  # 500ms
        
        # Statistics
        self.tx_count = 0
        self.rx_count = 0
        self.error_count = 0
        
    def connect(self) -> bool:
        """
        Connect to CAN bus
        
        Returns:
            True on success, False on failure
        """
        try:
            self.bus = can.Bus(
                interface='socketcan',
                channel=self.interface,
                bitrate=self.bitrate
            )
            
            # Start reception thread
            self.running = True
            self.rx_thread = threading.Thread(target=self._rx_loop, daemon=True)
            self.rx_thread.start()
            
            print(f"[CAN] Connected to {self.interface} at {self.bitrate} bps")
            return True
            
        except Exception as e:
            print(f"[CAN] Connection failed: {e}")
            return False
    
    def disconnect(self):
        """Disconnect from CAN bus"""
        self.running = False
        if self.rx_thread:
            self.rx_thread.join(timeout=1.0)
        if self.bus:
            self.bus.shutdown()
            self.bus = None
        print("[CAN] Disconnected")
    
    def register_callback(self, msg_id: int, callback: Callable):
        """
        Register callback for specific message ID
        
        Args:
            msg_id: CAN message ID
            callback: Function to call when message received
        """
        self.callbacks[msg_id] = callback
    
    # ========================================================================
    # COMMAND TRANSMISSION
    # ========================================================================
    
    def send_motor_enable(self, motor1: bool, motor2: bool) -> bool:
        """Enable/disable motors"""
        flags = (int(motor1) << 0) | (int(motor2) << 1)
        return self._send_message(CANCommand.MOTOR_ENABLE, [flags])
    
    def send_set_velocity(self, motor1_rpm: float, motor2_rpm: float) -> bool:
        """Set target velocities"""
        data = struct.pack('<ff', motor1_rpm, motor2_rpm)
        return self._send_message(CANCommand.SET_VELOCITY, data)
    
    def send_set_position(self, motor1_pos: int, motor2_pos: int) -> bool:
        """Set target positions"""
        data = struct.pack('<ii', motor1_pos, motor2_pos)
        return self._send_message(CANCommand.SET_POSITION, data)
    
    def send_set_pid_gains(self, motor_id: int, kp: float, ki: float, kd: float) -> bool:
        """
        Set PID gains
        Note: Requires multi-frame transmission (not yet implemented)
        """
        # TODO: Implement multi-frame transmission
        print(f"[CAN] Multi-frame messages not yet implemented")
        return False
    
    def send_emergency_stop(self) -> bool:
        """Emergency stop"""
        return self._send_message(CANCommand.EMERGENCY_STOP, [])
    
    def send_reset_position(self) -> bool:
        """Reset encoder positions to zero"""
        return self._send_message(CANCommand.RESET_POSITION, [])
    
    def send_set_accel_limit(self, limit: float) -> bool:
        """Set acceleration limit"""
        data = struct.pack('<f', limit)
        return self._send_message(CANCommand.SET_ACCEL_LIMIT, data)
    
    def send_set_current_limit(self, limit: float) -> bool:
        """Set current limit"""
        data = struct.pack('<f', limit)
        return self._send_message(CANCommand.SET_CURRENT_LIMIT, data)
    
    def send_clear_fault(self) -> bool:
        """Clear fault state"""
        return self._send_message(CANCommand.CLEAR_FAULT, [])
    
    def send_set_mode(self, mode: ControlMode) -> bool:
        """Set control mode"""
        return self._send_message(CANCommand.SET_MODE, [mode])
    
    def send_ping(self, timestamp: int) -> bool:
        """Send PING message"""
        data = struct.pack('<I', timestamp)
        return self._send_message(CANCommand.PING, data)
    
    def send_request_status(self, status_id: int) -> bool:
        """Request specific status message"""
        return self._send_message(CANCommand.REQUEST_STATUS, [status_id])
    
    # ========================================================================
    # LOW-LEVEL TRANSMISSION
    # ========================================================================
    
    def _send_message(self, msg_id: int, data: bytes) -> bool:
        """
        Send CAN message
        
        Args:
            msg_id: CAN message ID
            data: Message payload (0-8 bytes)
            
        Returns:
            True on success, False on failure
        """
        if not self.bus:
            print("[CAN] Not connected")
            return False
        
        try:
            msg = can.Message(
                arbitration_id=msg_id,
                data=data,
                is_extended_id=False
            )
            self.bus.send(msg)
            self.tx_count += 1
            return True
            
        except Exception as e:
            print(f"[CAN] Send failed: {e}")
            self.error_count += 1
            return False
    
    # ========================================================================
    # MESSAGE RECEPTION
    # ========================================================================
    
    def _rx_loop(self):
        """Reception thread main loop"""
        print("[CAN] RX thread started")
        
        while self.running and self.bus:
            try:
                msg = self.bus.recv(timeout=0.1)
                if msg:
                    self._process_message(msg)
                    
            except Exception as e:
                print(f"[CAN] RX error: {e}")
                self.error_count += 1
        
        print("[CAN] RX thread stopped")
    
    def _process_message(self, msg: can.Message):
        """
        Process received CAN message
        
        Args:
            msg: Received CAN message
        """
        self.rx_count += 1
        msg_id = msg.arbitration_id
        
        # Decode based on message ID
        if msg_id == CANStatus.HEARTBEAT:
            self._handle_heartbeat(msg.data)
        elif msg_id == CANStatus.POSITION_REPORT:
            self._handle_position(msg.data)
        elif msg_id == CANStatus.VELOCITY_REPORT:
            self._handle_velocity(msg.data)
        elif msg_id == CANStatus.MOTOR_STATUS:
            self._handle_motor_status(msg.data)
        elif msg_id == CANStatus.FAULT_REPORT:
            self._handle_fault(msg.data)
        elif msg_id == CANStatus.CURRENT_REPORT:
            self._handle_current(msg.data)
        elif msg_id == CANStatus.PONG:
            self._handle_pong(msg.data)
        
        # Call registered callback if exists
        if msg_id in self.callbacks:
            self.callbacks[msg_id](msg)
    
    # ========================================================================
    # MESSAGE DECODERS
    # ========================================================================
    
    def _handle_heartbeat(self, data: bytes):
        """Decode heartbeat message"""
        state = SystemState(data[0])
        self.last_heartbeat = state
        self.last_heartbeat_time = time.time()
    
    def _handle_position(self, data: bytes):
        """Decode position report"""
        motor1_pos, motor2_pos = struct.unpack('<ii', data)
        self.last_position = MotorPosition(
            motor1_pos=motor1_pos,
            motor2_pos=motor2_pos,
            timestamp=time.time()
        )
    
    def _handle_velocity(self, data: bytes):
        """Decode velocity report"""
        motor1_vel, motor2_vel = struct.unpack('<ff', data)
        self.last_velocity = MotorVelocity(
            motor1_vel=motor1_vel,
            motor2_vel=motor2_vel,
            timestamp=time.time()
        )
    
    def _handle_motor_status(self, data: bytes):
        """Decode motor status"""
        status_flags, motor1_err, motor2_err, uptime = struct.unpack('<HHHH', data)
        self.last_motor_status = MotorStatus(
            status_flags=status_flags,
            motor1_errors=motor1_err,
            motor2_errors=motor2_err,
            uptime_seconds=uptime,
            timestamp=time.time()
        )
    
    def _handle_fault(self, data: bytes):
        """Decode fault report"""
        fault_flags, = struct.unpack('<I', data)
        self.last_fault = FaultReport(
            fault_flags=fault_flags,
            timestamp=time.time()
        )
    
    def _handle_current(self, data: bytes):
        """Decode current report"""
        motor1_cur, motor2_cur = struct.unpack('<ff', data)
        self.last_current = CurrentReport(
            motor1_current=motor1_cur,
            motor2_current=motor2_cur,
            timestamp=time.time()
        )
    
    def _handle_pong(self, data: bytes):
        """Decode PONG response"""
        sequence, = struct.unpack('<I', data)
        # Could measure RTT here
    
    # ========================================================================
    # STATUS QUERIES
    # ========================================================================
    
    def get_system_state(self) -> Optional[SystemState]:
        """Get last received system state"""
        return self.last_heartbeat
    
    def get_position(self) -> Optional[MotorPosition]:
        """Get last received position"""
        return self.last_position
    
    def get_velocity(self) -> Optional[MotorVelocity]:
        """Get last received velocity"""
        return self.last_velocity
    
    def get_motor_status(self) -> Optional[MotorStatus]:
        """Get last received motor status"""
        return self.last_motor_status
    
    def get_fault_flags(self) -> Optional[int]:
        """Get last received fault flags"""
        return self.last_fault.fault_flags if self.last_fault else None
    
    def check_watchdog(self) -> bool:
        """
        Check if heartbeat is being received
        
        Returns:
            True if heartbeat is recent, False if timeout
        """
        if self.last_heartbeat_time == 0.0:
            return False  # Never received
        
        elapsed = time.time() - self.last_heartbeat_time
        return elapsed < self.heartbeat_timeout
    
    def get_statistics(self) -> Dict[str, int]:
        """Get communication statistics"""
        return {
            'tx_count': self.tx_count,
            'rx_count': self.rx_count,
            'error_count': self.error_count
        }

# ============================================================================
# EXAMPLE USAGE
# ============================================================================

if __name__ == '__main__':
    # Example usage
    can_if = CANInterface('can0')
    
    if can_if.connect():
        # Register callback for position reports
        def on_position(msg):
            pos = can_if.get_position()
            print(f"Position: M1={pos.motor1_pos}, M2={pos.motor2_pos}")
        
        can_if.register_callback(CANStatus.POSITION_REPORT, on_position)
        
        # Enable motors
        can_if.send_motor_enable(True, True)
        
        # Set velocity
        can_if.send_set_velocity(10.0, 10.0)
        
        # Run for 10 seconds
        time.sleep(10.0)
        
        # Stop motors
        can_if.send_set_velocity(0.0, 0.0)
        
        # Disconnect
        can_if.disconnect()
