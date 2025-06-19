import psutil
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import datetime

usage = []
timestamps = []

fig, ax = plt.subplots()

def update(i):
    cpu = psutil.cpu_percent()
    usage.append(cpu)
    timestamps.append(datetime.datetime.now().strftime("%H:%M:%S"))

    usage_plot = usage[-20:]  # show last 20 seconds
    ts_plot = timestamps[-20:]
    
    ax.clear()
    ax.plot(ts_plot, usage_plot, marker='o')
    ax.set_title("Real-time CPU Usage")
    ax.set_xlabel("Time")
    ax.set_ylabel("CPU %")
    ax.tick_params(axis='x', rotation=45)
    ax.grid(True)

ani = animation.FuncAnimation(fig, update, interval=1000)
plt.tight_layout()
plt.show()
