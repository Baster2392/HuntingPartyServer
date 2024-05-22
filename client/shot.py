class Shot:
    def __init__(self, x, y):
        self.x = x
        self.y = y
        self.control_sum = self.calculate_control_sum()

    def calculate_control_sum(self):
        return sum([self.x, self.y]) % 256
