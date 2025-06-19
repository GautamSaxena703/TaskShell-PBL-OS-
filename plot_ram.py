import psutil
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import time

usage = []
timestamps = []

fig, ax = plt.subplots()
line, = ax.plot([], [], marker='o', color='green')
ax.set_title("Live RAM Usage")
ax.set_xlabel("Time")
ax.set_ylabel("RAM %")
ax.grid(True)

def update(frame):
    ram = psutil.virtual_memory().percent
    usage.append(ram)
    timestamps.append(time.strftime("%H:%M:%S"))
    if len(timestamps) > 10:
        usage.pop(0)
        timestamps.pop(0)
    # Use numerical indices for x-data
    line.set_data(range(len(timestamps)), usage)
    # Set x-ticks and labels
    ax.set_xticks(range(len(timestamps)))
    ax.set_xticklabels(timestamps, rotation=45)
    ax.relim()
    ax.autoscale_view()
    return line,

ani = animation.FuncAnimation(fig, update, interval=1000)
plt.tight_layout()
plt.show()
