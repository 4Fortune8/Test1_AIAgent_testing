"""
trajectory_planner.py - Motion trajectory generation for CNC-style movement

V2.00.00: User request: "plan out motor control system"
Plan: docs/thoughtprocesses/2026-01-22_overnight-motor-control-planning.md

PLANNING DOCUMENT - NOT YET IMPLEMENTED

Generates smooth trajectories with:
- Trapezoidal velocity profiles
- S-curve acceleration (jerk limiting)
- Multi-axis coordination
- Look-ahead planning
"""

import math
import numpy as np
from dataclasses import dataclass
from typing import List, Tuple, Optional
from enum import Enum

# ============================================================================
# TRAJECTORY TYPES
# ============================================================================

class ProfileType(Enum):
    """Velocity profile types"""
    TRAPEZOIDAL = 1      # Constant acceleration
    S_CURVE = 2           # Jerk-limited (smoother)
    POINT_TO_POINT = 3   # Simple move
    SYNCHRONIZED = 4      # Multi-axis synchronized

# ============================================================================
# DATA STRUCTURES
# ============================================================================

@dataclass
class MotionLimits:
    """Motion limits per axis"""
    max_velocity: float      # RPM
    max_acceleration: float  # RPM/s
    max_jerk: float          # RPM/s^2 (for S-curves)

@dataclass
class TrajectoryPoint:
    """Single point in trajectory"""
    timestamp: float         # Seconds from start
    position: np.ndarray     # Position per axis (counts)
    velocity: np.ndarray     # Velocity per axis (RPM)
    acceleration: np.ndarray # Acceleration per axis (RPM/s)

@dataclass
class Trajectory:
    """Complete trajectory"""
    points: List[TrajectoryPoint]
    duration: float
    profile_type: ProfileType

# ============================================================================
# TRAJECTORY PLANNER
# ============================================================================

class TrajectoryPlanner:
    """
    Generates motion trajectories for multi-axis systems
    
    Features:
    - Trapezoidal and S-curve profiles
    - Multi-axis synchronization
    - Velocity blending between segments
    - Time-optimal planning
    """
    
    def __init__(self, num_axes: int = 2, limits: Optional[MotionLimits] = None):
        """
        Initialize trajectory planner
        
        Args:
            num_axes: Number of axes (default 2)
            limits: Motion limits (default conservative values)
        """
        self.num_axes = num_axes
        
        if limits is None:
            # Default conservative limits
            self.limits = MotionLimits(
                max_velocity=60.0,      # 60 RPM
                max_acceleration=500.0, # 500 RPM/s
                max_jerk=5000.0         # 5000 RPM/s^2
            )
        else:
            self.limits = limits
    
    # ========================================================================
    # POINT-TO-POINT MOTION
    # ========================================================================
    
    def plan_point_to_point(
        self,
        start_pos: np.ndarray,
        end_pos: np.ndarray,
        max_vel: Optional[float] = None,
        profile: ProfileType = ProfileType.TRAPEZOIDAL
    ) -> Trajectory:
        """
        Plan point-to-point motion with synchronized axes
        
        Args:
            start_pos: Starting position (counts per axis)
            end_pos: Ending position (counts per axis)
            max_vel: Maximum velocity (RPM), None = use limit
            profile: Velocity profile type
            
        Returns:
            Trajectory object
        """
        if max_vel is None:
            max_vel = self.limits.max_velocity
        
        # Compute distance per axis
        distance = end_pos - start_pos
        
        # Find limiting axis (longest time)
        time_per_axis = np.abs(distance) / max_vel
        total_time = np.max(time_per_axis)
        
        # Scale velocities to synchronize axes
        velocity_scaling = time_per_axis / total_time
        axis_velocities = max_vel * velocity_scaling * np.sign(distance)
        
        # Generate profile based on type
        if profile == ProfileType.TRAPEZOIDAL:
            return self._generate_trapezoidal(
                start_pos, end_pos, axis_velocities, total_time
            )
        elif profile == ProfileType.S_CURVE:
            return self._generate_s_curve(
                start_pos, end_pos, axis_velocities, total_time
            )
        else:
            raise ValueError(f"Unsupported profile type: {profile}")
    
    # ========================================================================
    # TRAPEZOIDAL PROFILE
    # ========================================================================
    
    def _generate_trapezoidal(
        self,
        start_pos: np.ndarray,
        end_pos: np.ndarray,
        max_velocities: np.ndarray,
        total_time: float,
        dt: float = 0.02  # 50 Hz sampling
    ) -> Trajectory:
        """
        Generate trapezoidal velocity profile
        
        Profile has three phases:
        1. Acceleration phase (constant accel)
        2. Constant velocity phase
        3. Deceleration phase (constant decel)
        
        Args:
            start_pos: Start positions
            end_pos: End positions
            max_velocities: Maximum velocities per axis
            total_time: Total move time
            dt: Sampling period
            
        Returns:
            Trajectory with sample points
        """
        distance = end_pos - start_pos
        accel = self.limits.max_acceleration
        
        # Compute phase times
        # Assume symmetric accel/decel
        t_accel = np.min(max_velocities) / accel
        t_decel = t_accel
        t_const = total_time - t_accel - t_decel
        
        if t_const < 0:
            # Cannot reach max velocity - triangular profile
            t_accel = math.sqrt(total_time / 2.0)
            t_decel = t_accel
            t_const = 0
            peak_velocity = accel * t_accel
        else:
            peak_velocity = np.min(max_velocities)
        
        # Generate sample points
        num_samples = int(total_time / dt) + 1
        timestamps = np.linspace(0, total_time, num_samples)
        
        points = []
        for t in timestamps:
            if t <= t_accel:
                # Acceleration phase
                v = accel * t
                s = 0.5 * accel * t**2
                a = accel
            elif t <= t_accel + t_const:
                # Constant velocity phase
                v = peak_velocity
                s = 0.5 * accel * t_accel**2 + peak_velocity * (t - t_accel)
                a = 0
            else:
                # Deceleration phase
                t_dec = t - t_accel - t_const
                v = peak_velocity - accel * t_dec
                s = (0.5 * accel * t_accel**2 + 
                     peak_velocity * t_const + 
                     peak_velocity * t_dec - 0.5 * accel * t_dec**2)
                a = -accel
            
            # Scale to actual distance and direction
            progress = s / (np.linalg.norm(distance) if np.linalg.norm(distance) > 0 else 1)
            position = start_pos + progress * distance
            velocity = v * (distance / np.linalg.norm(distance))
            acceleration = a * (distance / np.linalg.norm(distance))
            
            point = TrajectoryPoint(
                timestamp=t,
                position=position,
                velocity=velocity,
                acceleration=acceleration
            )
            points.append(point)
        
        return Trajectory(
            points=points,
            duration=total_time,
            profile_type=ProfileType.TRAPEZOIDAL
        )
    
    # ========================================================================
    # S-CURVE PROFILE
    # ========================================================================
    
    def _generate_s_curve(
        self,
        start_pos: np.ndarray,
        end_pos: np.ndarray,
        max_velocities: np.ndarray,
        total_time: float,
        dt: float = 0.02
    ) -> Trajectory:
        """
        Generate S-curve (jerk-limited) velocity profile
        
        Profile has seven phases:
        1. Increasing acceleration (jerk > 0)
        2. Constant acceleration
        3. Decreasing acceleration (jerk < 0)
        4. Constant velocity
        5. Increasing deceleration (jerk < 0)
        6. Constant deceleration
        7. Decreasing deceleration (jerk > 0)
        
        Args:
            start_pos: Start positions
            end_pos: End positions
            max_velocities: Maximum velocities per axis
            total_time: Total move time
            dt: Sampling period
            
        Returns:
            Trajectory with sample points
        """
        # Simplified S-curve - assume symmetric profile
        # In practice, would compute optimal phase times
        
        jerk = self.limits.max_jerk
        accel_max = self.limits.max_acceleration
        
        # Time to reach max acceleration
        t_jerk = accel_max / jerk
        
        # Assuming we reach max accel and max velocity
        t_accel_total = 2 * t_jerk + (np.min(max_velocities) / accel_max)
        
        # Generate profile (simplified - full implementation would be more complex)
        # For now, return trapezoidal as placeholder
        return self._generate_trapezoidal(
            start_pos, end_pos, max_velocities, total_time, dt
        )
    
    # ========================================================================
    # MULTI-SEGMENT PATHS
    # ========================================================================
    
    def plan_path(
        self,
        waypoints: List[np.ndarray],
        blend_radius: float = 0.0
    ) -> Trajectory:
        """
        Plan multi-segment path through waypoints
        
        Args:
            waypoints: List of positions to pass through
            blend_radius: Radius for velocity blending at corners
            
        Returns:
            Combined trajectory
        """
        if len(waypoints) < 2:
            raise ValueError("Need at least 2 waypoints")
        
        # Plan each segment
        segments = []
        for i in range(len(waypoints) - 1):
            segment = self.plan_point_to_point(
                waypoints[i],
                waypoints[i + 1],
                profile=ProfileType.TRAPEZOIDAL
            )
            segments.append(segment)
        
        # Concatenate segments (with blending if specified)
        if blend_radius > 0:
            return self._blend_segments(segments, blend_radius)
        else:
            return self._concatenate_segments(segments)
    
    def _concatenate_segments(self, segments: List[Trajectory]) -> Trajectory:
        """Concatenate trajectory segments"""
        all_points = []
        time_offset = 0.0
        
        for segment in segments:
            for point in segment.points:
                new_point = TrajectoryPoint(
                    timestamp=point.timestamp + time_offset,
                    position=point.position,
                    velocity=point.velocity,
                    acceleration=point.acceleration
                )
                all_points.append(new_point)
            time_offset += segment.duration
        
        return Trajectory(
            points=all_points,
            duration=time_offset,
            profile_type=ProfileType.SYNCHRONIZED
        )
    
    def _blend_segments(
        self,
        segments: List[Trajectory],
        blend_radius: float
    ) -> Trajectory:
        """
        Blend trajectory segments with corner rounding
        
        TODO: Implement velocity blending algorithm
        """
        # Placeholder - return concatenated for now
        return self._concatenate_segments(segments)
    
    # ========================================================================
    # TRAJECTORY OPTIMIZATION
    # ========================================================================
    
    def optimize_time(
        self,
        start_pos: np.ndarray,
        end_pos: np.ndarray
    ) -> Trajectory:
        """
        Generate time-optimal trajectory within limits
        
        Computes fastest possible move while respecting:
        - Velocity limits
        - Acceleration limits
        - Jerk limits (if using S-curves)
        
        Args:
            start_pos: Starting position
            end_pos: Ending position
            
        Returns:
            Time-optimal trajectory
        """
        # Compute distance
        distance = np.linalg.norm(end_pos - start_pos)
        
        # Time-optimal profile uses maximum acceleration throughout
        # t_accel = sqrt(distance / accel)
        t_accel = math.sqrt(distance / self.limits.max_acceleration)
        peak_velocity = self.limits.max_acceleration * t_accel
        
        if peak_velocity > self.limits.max_velocity:
            # Velocity-limited
            peak_velocity = self.limits.max_velocity
            # Recompute times
            t_accel = peak_velocity / self.limits.max_acceleration
            distance_accel = 0.5 * self.limits.max_acceleration * t_accel**2
            distance_const = distance - 2 * distance_accel
            t_const = distance_const / peak_velocity
            total_time = 2 * t_accel + t_const
        else:
            # Acceleration-limited (triangular profile)
            total_time = 2 * t_accel
        
        # Generate trajectory
        return self.plan_point_to_point(
            start_pos, end_pos,
            max_vel=peak_velocity,
            profile=ProfileType.TRAPEZOIDAL
        )
    
    # ========================================================================
    # UTILITIES
    # ========================================================================
    
    def interpolate_trajectory(
        self,
        trajectory: Trajectory,
        timestamp: float
    ) -> TrajectoryPoint:
        """
        Interpolate trajectory at specific timestamp
        
        Args:
            trajectory: Trajectory to interpolate
            timestamp: Time to sample at
            
        Returns:
            Interpolated trajectory point
        """
        points = trajectory.points
        
        # Find surrounding points
        for i in range(len(points) - 1):
            if points[i].timestamp <= timestamp <= points[i + 1].timestamp:
                # Linear interpolation
                t0 = points[i].timestamp
                t1 = points[i + 1].timestamp
                alpha = (timestamp - t0) / (t1 - t0)
                
                return TrajectoryPoint(
                    timestamp=timestamp,
                    position=points[i].position + alpha * (points[i + 1].position - points[i].position),
                    velocity=points[i].velocity + alpha * (points[i + 1].velocity - points[i].velocity),
                    acceleration=points[i].acceleration + alpha * (points[i + 1].acceleration - points[i].acceleration)
                )
        
        # Outside range - return closest endpoint
        if timestamp < points[0].timestamp:
            return points[0]
        else:
            return points[-1]
    
    def export_to_can_messages(self, trajectory: Trajectory) -> List[bytes]:
        """
        Convert trajectory to CAN TRAJECTORY_POINT messages
        
        Returns:
            List of encoded CAN message payloads
        """
        # TODO: Implement CAN message encoding
        # Format: See protocol/motor_control_messages.md
        pass


# ============================================================================
# EXAMPLE USAGE
# ============================================================================

if __name__ == '__main__':
    # Example trajectory generation
    planner = TrajectoryPlanner(num_axes=2)
    
    # Point-to-point move
    start = np.array([0, 0])
    end = np.array([10000, 5000])  # Encoder counts
    
    trajectory = planner.plan_point_to_point(start, end)
    
    print(f"Trajectory duration: {trajectory.duration:.2f}s")
    print(f"Number of points: {len(trajectory.points)}")
    
    # Multi-waypoint path
    waypoints = [
        np.array([0, 0]),
        np.array([10000, 0]),
        np.array([10000, 10000]),
        np.array([0, 10000]),
        np.array([0, 0])
    ]
    
    path = planner.plan_path(waypoints)
    print(f"Path duration: {path.duration:.2f}s")
