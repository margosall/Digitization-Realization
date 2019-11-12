value = -32768

a = (1 - -1)/(32767 - -32768)
b = 1 - a * 32767

new_value = a * value + b

print(new_value, a, b)