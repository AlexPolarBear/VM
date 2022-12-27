import matplotlib.pyplot as plt
import csv
import numpy as np
import math

import matplotlib.ticker as ticker
x = []
y = []
with open('result.csv','r') as csvfile:
    lines = csv.reader(csvfile, delimiter=',')
    for row in lines:
        x.append(math.log(int(row[0]), 2))
        y.append(float(row[1]))
    
# y1 = [1] + [i for i in y]
# for i in range(len(y)):
#     y[i] /= y1[i]    

plt.plot(x, y, color = 'g', linestyle = 'dashed',
        marker = 'o',label = "Computation Data")

plt.xticks(rotation = 1)
plt.xticks([0] + x)
plt.xlabel('Elements')
plt.ylabel('Time')
plt.grid()
plt.legend()
plt.show()