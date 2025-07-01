import math

"""F.airy L.I.ght Trail"""
class Flit:
    def __init__(self, position=(0, 0)):
        self.orbital_center = position
        self.global_position = position
        self.color = (255, 255, 255)
        self.oscillation_offset = 0
        """positive value indicates leftword movement, ranging 0-1"""
        self.scale = 1
        self.radius = 15
        self.speed = 10
        self.acceleration = 1
        self.direction = 360
        self.obstacles = []
        self.blots = []

    def update_oscillation(self, oscillation: float):
        self.oscillation_offset = float(oscillation * self.scale)

    def get_global_position(self):
        return (self.global_position[0], self.global_position[1])
    def get_orbital_center(self):
        return (self.orbital_center[0], self.orbital_center[1])
    
    def update_speed(self):
        self.speed *= self.acceleration
        self.speed = min(5, self.speed)
        self.speed = max(-5, self.speed)

    def point_to_line_distance_and_angle(p, a, b):
        """Returns:
        - perpendicular distance from point p to the infinite line through a–b
        - angle (in degrees) from p to the closest point on the line
        """
        x0, y0 = p
        x1, y1 = a
        x2, y2 = b

        dx = x2 - x1
        dy = y2 - y1
        den = dx**2 + dy**2

        if den == 0:
            # a and b are the same point
            dist = math.hypot(x0 - x1, y0 - y1)
            angle = math.degrees(math.atan2(y1 - y0, x1 - x0))
            return dist, angle

        # Project p onto the infinite line
        t = ((x0 - x1) * dx + (y0 - y1) * dy) / den
        closest_x = x1 + t * dx
        closest_y = y1 + t * dy

        # Perpendicular distance
        dist = math.hypot(closest_x - x0, closest_y - y0)

        # Angle from p to the closest point
        angle = math.degrees(math.atan2(closest_y - y0, closest_x - x0))

        return dist, angle
    
    def get_dir_change(self, angle):
        """Returns a signed speed to rotate the flit away from the given angle."""
        def angle_diff(a, b):
            """Returns smallest signed difference from a to b in degrees (-180 to 180)."""
            return ((b - a + 180) % 360) - 180

        diff = angle_diff(self.direction, angle)

        # To move away, we turn in the direction that *increases* the absolute difference
        if diff > 0:
            return -int(self.speed)  # turning counterclockwise increases separation
        else:
            return int(self.speed)   # turning clockwise increases separation


    def account_for_obstacles(self):
        # Screen edges are considered obstacles
        # all obstacles are considered to be a diagonal line \
        for obstacle in self.obstacles:
            start, end = (0,0), (0,0)
            if type(obstacle) is Flit:
                # this places a diagonal line through the Flit
                start, end = (obstacle.get_orbital_center()[0] - obstacle.radius,
                              obstacle.get_orbital_center()[1] - obstacle.radius), \
                             (obstacle.get_orbital_center()[0] + obstacle.radius,
                              obstacle.get_orbital_center()[1] + obstacle.radius)
            else:
                start, end = obstacle

            # Check if the flit is close to the obstacle
            d = 100
            dist, dir = Flit.point_to_line_distance_and_angle(self.get_orbital_center(), start, end)
            if dist < d:
                if 100 < abs(self.direction - dir) < 260:
                    pass
                else:
                # dir_change = self.get_dir_change(dir)
                # self.update_direction(dir_change)
                    self.update_direction(int(self.speed))

    def update_color(self):
        self.color = (
            min(max(int(255 * self.oscillation_offset),0),255),
            255,
            min(max(int((self.radius - 19)*50),0),255)
        )
            
    def add_flit_as_obstacle(self, flit):
        self.obstacles.append(flit)

    def add_screen_edge_obstacles(self, screen_size):
        """Adds the screen edges as obstacles."""
        width, height = screen_size
        self.obstacles.append( ((0, 0), (width, 0)) )
        self.obstacles.append( ((width, 0), (width, height)) )
        self.obstacles.append( ((width, height), (0, height)) )
        self.obstacles.append( ((0, height), (0, 0)) )

    """Applies the direction facing and speed to the position."""
    def update_position(self):
        self.account_for_obstacles()
        dx = self.speed * math.cos(math.radians(self.direction)) * self.scale
        dy = self.speed * math.sin(math.radians(self.direction)) * self.scale

        # apply oscillation
        dx_o = self.oscillation_offset * math.cos(math.radians(self.direction - 90)) * self.scale * 40
        dy_o = self.oscillation_offset * math.sin(math.radians(self.direction - 90)) * self.scale * 30

        self.orbital_center = (self.orbital_center[0] + dx, self.orbital_center[1] + dy)
        self.global_position = (self.orbital_center[0] + dx_o, self.orbital_center[1] + dy_o)
    
    def update_radius(self, radial_change: int):
        self.radius = (20 + radial_change) * self.scale

    """Adds clockwise rotation"""
    def update_direction(self, change: int):
        self.direction = (self.direction + change) % 360

    def __str__(self):
        return f"Flit(position={self.orbital_center}, color={self.color}, oscillation_offset={self.oscillation_offset}, scale={self.scale}, radius={self.radius}, speed={self.speed}, direction={self.direction})"