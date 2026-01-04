import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import math

def round_to_1sf(x):
    if x == 0: return 0
    return round(x, -int(math.floor(math.log10(abs(x)))))

df = pd.read_csv("data/example_data.csv")
initTime = df.iloc[0]['Time']
df['Time'] -= initTime
df['Time'] /= 1000
df['Time'] = pd.to_datetime(df['Time'], unit='ms')

freq = '10ms' 

'''
Frequency at which the data is taken into account, i.e. for 10ms, 
everything in between each 10ms is not taken into account except 
the last trade in that 10ms period.
'''


df = df.set_index('Time')
df = df.resample(freq).last()
df['time_s'] = df.index.strftime('%M : %S.%f').str[:-4]

plt.figure(figsize=(12, 7))
plt.plot(df['time_s'], df['Price'])
plt.title("LOB Stock Price Series")
plt.xlabel("Time")
plt.ylabel("Price")
plt.grid(True)
plt.xticks(rotation=45)
plt.tight_layout()

ax = plt.gca()
ax.xaxis.set_major_locator(ticker.MaxNLocator(12)) 
ax.set_xlim(left=0.00)

plt.show()