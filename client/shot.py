class Shot:
    def __init__(self, x, y):
        self.x = x
        self.y = y
        self.control_sum = calculate_control_sum(self)

def calculate_control_sum(self):
    csum = 0
    csum = self.x
    csum += self.y

    return csum % 256
