import math
import random
class Blot:
    def __init__(self, x, y, radius):
        self.x = x
        self.y = y
        self.xr = x
        self.yr = y
        self.radius = radius
        self.counter = 15
        self.is_parent = True
        self.direction = 0
        self.speed = random.randint(1,5)
        self.flits = []

    def randomize(self):
        # if self.counter < 8:
            self.xr += random.randint(-1, 1)
            self.yr += random.randint(-1, 1)

    def follow_flits(self):
        dir_goal = 0
        for flit in self.flits:
            dir_goal += flit.direction
        dir_goal /= len(self.flits) if self.flits else 1

        diff = (dir_goal - self.direction + 360) % 360
        if diff == 0:
            return  # Already aligned
        elif diff < 180:
            self.direction = (self.direction + 1) % 360
        else:
            self.direction = (self.direction - 1) % 360

    def update_position(self):
        self.follow_flits()
        dx = self.speed * math.cos(math.radians(self.direction))
        dy = self.speed * math.sin(math.radians(self.direction))

        # self.x += dx/3
        self.xr += dx
        # self.y += dy/3
        self.yr += dy

        self.speed = max(0, self.speed - 0.05)