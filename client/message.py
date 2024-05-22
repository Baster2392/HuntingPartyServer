class Message:
    def __init__(self, content):
        self.content = content
        self.control_sum = self.calculate_control_sum(content)

    def calculate_control_sum(self, content):
        return sum(ord(char) for char in content) % 256
