import numpy as np
import matplotlib.pyplot as plt

samples = 5000
freq1 = 1
freq2 = 1
x = np.arange(samples)

y1 = np.sin(2 * np.pi *freq1 * x / samples)
y2 = np.sin(2 * np.pi *freq2 * (x + 1250) / samples)

print(np.correlate(y1, y2))

plt.plot(y1)
plt.plot(y2)

plt.show()