import serial


def main():
    # Establish connection with Pi Pico connected via USB to my macbook
    port = "/dev/tty.usbmodem1101"
    baudrate = 115200
    conn = serial.Serial(port, baudrate)
    # Open log file and write chunks of data inf (until Ctrl+C is pressed or conn fails)
    with open("build/log.txt", "w+", encoding="UTF-8") as logfile:
        while True:
            data = conn.read(64)
            print(data)
            logfile.write(str(data))


if __name__ == "__main__":
    main()
