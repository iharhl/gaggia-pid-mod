import matplotlib.pyplot as plt


def main():
    # Read the logfile
    with open("docs/final-tests/log-heatup.txt", "r", encoding="UTF-8") as logfile:
        content = logfile.read()

    # Remove symbols - "b", "'" and "\n"
    content = content.replace("b", "").replace("'", "").replace("\n", "")
    # Split content by spaces
    content = content.split(" ")
    # Remove first and last element as they might be corrupted
    content = content[1:-1]

    # Create x and y data point for the plot
    ypoints = [float(point) for point in content]
    xpoints = [i * 100/1000 for i in range(len(ypoints))] # sampled every ~100 ms

    # Plot the data
    plt.title('Heatup graph')
    plt.axhline(y=107, color='red', linestyle='--', linewidth=2) # setpoint line
    plt.xlabel('Time (s)')
    plt.ylabel('Temperature (°C)')
    plt.plot(xpoints, ypoints)
    plt.show()


if __name__ == "__main__":
    main()
