import matplotlib.pyplot as plt
import numpy as np

# Data for each test case and each number of threads (1, 4, 16, 64, 128, 256)
# Each entry contains [time_taken_1, time_taken_2, time_taken_3]
times = {
    # 256 mb
    "tc_0": {
        1: [1924.098, 2058.409, 1928.439],
        4: [561.881, 551.429, 517.455],
        16: [169.238, 153.506, 159.144],
        64: [98.540, 89.380, 99.461],
        128: [91.661, 99.291, 92.173],
        256: [96.092, 100.751, 102.296]
    },
    # 512 mb
    "tc_1": {
        1: [4094.587, 3947.701, 3882.823],
        4: [1051.615, 1060.341, 1055.224],
        16: [290.539, 290.640, 296.164],
        64: [160.960, 165.954, 160.233],
        128: [147.810, 155.299, 158.837],
        256: [157.845, 164.187, 161.384]
    },
    # 1 gb
    "tc_2": {
        1: [7438.919, 8234.853, 7532.173],
        4: [2047.849, 2028.552, 2144.608],
        16: [568.848, 571.310, 561.671],
        64: [291.265, 290.695, 297.833],
        128: [283.406, 287.719, 281.649],
        256: [290.474, 289.656, 287.014]
    },
    "tc_3": {
        1: [15565.048, 16107.099, 15943.311],
        4: [4100.952, 4068.322, 4028.326],
        16: [1114.912, 1124.919, 1116.941],
        64: [558.812, 575.004, 590.122],
        128: [547.455, 546.519, 536.731],
        256: [547.923, 539.991, 551.624]
    },
    "tc_4": {
        1: [31364.167, 29358.811, 31723.116],
        4: [8018.611, 8057.586, 8056.611],
        16: [2211.118, 2243.471, 2232.191],
        64: [1143.416, 1036.662, 1080.320],
        128: [1064.744, 1072.102, 1033.938],
        256: [1096.441, 1084.590, 1084.494]
    }
}

# Calculate the average time taken for each thread count
avg_times = {tc: {threads: np.mean(data) for threads, data in times_data.items()} for tc, times_data in times.items()}

# Calculate the speedup: time for single thread divided by time for n threads
speedups = {tc: {threads: avg_times[tc][1] / time for threads, time in avg_times[tc].items()} for tc in avg_times}

# Updating test case labels to reflect their memory sizes instead of generic labels
memory_labels = {
    "tc_0": "256 MB",
    "tc_1": "512 MB",
    "tc_2": "1 GB",
    "tc_3": "2 GB",
    "tc_4": "4 GB"
}

fig, axes = plt.subplots(1, 2, figsize=(16, 6))

# Colors for different test cases to distinguish them in the plots
colors = ['b', 'g', 'r', 'c', 'm', 'y', 'k']
test_cases = list(avg_times.keys())

# Define explicit thread counts for x-axis ticks
thread_counts = [1, 4, 16, 64, 128, 256]

# Plotting Time vs Threads for all test cases in one graph with specified thread counts
for i, tc in enumerate(test_cases):
    avg_time = [avg_times[tc][tcnt] for tcnt in thread_counts]
    axes[0].plot(thread_counts, avg_time, marker='o', color=colors[i % len(colors)], label=f"{memory_labels[tc]}")

axes[0].set_title("Time vs Threads for Different File Sizes")
axes[0].set_xlabel("Number of Threads")
axes[0].set_ylabel("Average Time (ms)")
axes[0].set_xticks(thread_counts)
axes[0].grid(True)
axes[0].legend(title="File Size")

# Plotting Speedup vs Threads for all test cases in one graph with specified thread counts
for i, tc in enumerate(test_cases):
    speedup = [speedups[tc][tcnt] for tcnt in thread_counts]
    axes[1].plot(thread_counts, speedup, marker='o', color=colors[i % len(colors)], label=f"{memory_labels[tc]}")

axes[1].set_title("Speedup vs Threads for Different File Sizes")
axes[1].set_xlabel("Number of Threads")
axes[1].set_ylabel("Speedup")
axes[1].set_xticks(thread_counts)
axes[1].grid(True)
axes[1].legend(title="File Size")

plt.show()
