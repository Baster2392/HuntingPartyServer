class Message:
    def __init__(self, content):
        self.content = content
        self.controlSum = 0
        self.calculate_control_sum()

    def calculate_control_sum(self):
        csum = 0
        for char in self.content:
            csum += ord(char)
        self.controlSum = csum % 256
