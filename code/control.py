import serial


rx = serial.Serial(port="/dev/ttyO1", baudrate=115200, timeout=1)

rx.reset_input_buffer()

print("looking for start")
while rx.read(1)[0]!= 0xb2:
    pass
print("found start")
rx.read(14)

channels = dict()


while True:
    y = tuple(int(i) for i in rx.read(16))
    for i in range(0,16,2):
        if y[i+1] == 0xb2:
            continue

        chan_id = (y[i] & 0x78) >> 3
        servo_pos = ((y[i] & 0x7) << 8) + y[i+1]
        channels[chan_id] = servo_pos

    print("\t".join(map(lambda k: "%d: %d"%(k, channels[k]), sorted(channels))))
    #for k in sorted(channels):
    #    print("%d: %d\t"%(k, channels[k]), end='')

    #print()

