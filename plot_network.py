import psutil
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import time

upload = []
download = []
timestamps = []

net1 = psutil.net_io_counters()

fig, ax = plt.subplots()
line_up, = ax.plot([], [], label='Upload KB/s', marker='o')
line_down, = ax.plot([], [], label='Download KB/s', marker='o')
ax.set_title("Live Network Usage")
ax.set_xlabel("Time")
ax.set_ylabel("Speed (KB/s)")
ax.grid(True)
ax.legend()

def update(frame):
    global net1
    net2 = psutil.net_io_counters()
    upload_speed = (net2.bytes_sent - net1.bytes_sent) / 1024
    download_speed = (net2.bytes_recv - net1.bytes_recv) / 1024
    net1 = net2

    upload.append(upload_speed)
    download.append(download_speed)
    timestamps.append(time.strftime("%H:%M:%S"))
    if len(timestamps) > 10:
        upload.pop(0)
        download.pop(0)
        timestamps.pop(0)
    # Use numerical indices for x-data
    line_up.set_data(range(len(timestamps)), upload)
    line_down.set_data(range(len(timestamps)), download)
    # Set x-ticks and labels
    ax.set_xticks(range(len(timestamps)))
    ax.set_xticklabels(timestamps, rotation=45)
    ax.relim()
    ax.autoscale_view()
    return line_up, line_down

ani = animation.FuncAnimation(fig, update, interval=1000)
plt.tight_layout()
plt.show()
