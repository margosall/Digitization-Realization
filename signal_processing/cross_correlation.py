import numpy as np
import matplotlib.pyplot as plt
from scipy.signal import correlate

a,b, N = 0, 1, 1000        #Boundaries, datapoints
shift = 0                 #Shift, note 3/10 of L = b-a

x = np.linspace(a,b,N)
x1 = 1*x + shift
time = np.arange(1-N,N)     #Theoritical definition, time is centered at 0

y1 = np.sin(2*np.pi*x/b)
y2 = np.cos(2*np.pi*x1/b)


#Really only helps with large irregular data, try it
# y1 -= y1.mean()
# y2 -= y2.mean()
# y1 /= y1.std()
# y2 /= y2.std()

cross_correlation = correlate(y1,y2)
shift_calculated = time[cross_correlation.argmax()] * 1.0 * b/N
y3 = np.cos(2*np.pi*(x1-shift_calculated)/b)
print ("Preset shift: ", shift, "\nCalculated shift: ", shift_calculated)

print(np.correlate(y1, y3))


plt.plot(x,y1)
plt.plot(x,y2)
plt.plot(x,y3)
plt.legend(("Regular", "Shifted", "Recovered"))
plt.savefig("SO_timeshift.png")
plt.show()