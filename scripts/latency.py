import pandas as pd
import matplotlib.pyplot as plt

# Load data
df = pd.read_csv("data/example_latency.csv") 
df['Time'] -= df.iloc[0]['Time']
df['Time'] /= 1000.0 #Convert to milliseconds

plt.figure(figsize=(12, 7))
plt.plot(df['NumOfOrders'], df['Time'], color='tab:blue', alpha=0.6, label='Individual Op')
plt.title("Latency Scalability: Performance over Cumulative Operations")
plt.xlabel("Total Number of Operations Performed")
plt.ylabel("Latency (ms)")
plt.grid(True, linestyle='--', alpha=0.7)

ax = plt.gca()
ax.set_ylim(bottom=0.00)
ax.set_xlim(left=0.00)

plt.legend()
plt.show()