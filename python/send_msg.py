# send_msg
# The CANlib library is initialized when the canlib module is imported. To be
# able to send a message, Frame also needs to be installed.

# Some simple python code that can be used to test out
# the actuator along with a simple bench-top CAN bus,
# a bench power supply and a Kvaser or whatever USB to CAN tester at hand


from time import time
from canlib import canlib, Frame
import time
import sys

# open a CAN channel
#
ch_a = canlib.openChannel(channel=0)
# ch_b = canlib.openChannel(channel=1)


# After opening the channel, we need to set the bus parameters.
ch_a.setBusParams(canlib.canBITRATE_500K)
# ch_a.setBusParams(canlib.canBITRATE_250K)

# The next step is to Activate the CAN chip for each channel (ch_a and ch_b in
# this example) use .busOn() to make them ready to receive and send messages.
ch_a.busOn()
# ch_b.busOn()

MaxDist = 0.5626  # Max distance the actuator can safely move

Dist = float(input("enter the actuator distance in inches:"))

print("dist ", Dist, "  MaxDist", MaxDist)
if Dist >= MaxDist:
    print("requested distance is greater than maximum safe actuation distance")
    sys.exit(0)

hexDist = hex(int(Dist * 1000 + 500))

bite2 = hexDist[-2:]  # last to digits of distance

# Clutch On, Motor Off  -
bite3 = "8" + hexDist[2: -2: 1]
frame = Frame(id_=0xFF0000, data=[0x0F, 0x4A, int(bite2, 16), int(bite3, 16), 0, 0, 0, 0], flags=canlib.MessageFlag.EXT)
ch_a.write(frame)
time.sleep(.02)  # wait 20 ms

for interval in range(6):
    # Clutch On, Motor On  - Move to new position an hold
    bite3 = "C" + hexDist[2: -2: 1]

    frame = Frame(id_=0xFF0000, data=[0x0F, 0x4A, int(bite2, 16), int(bite3, 16), 0, 0, 0, 0],
                  flags=canlib.MessageFlag.EXT)

    ch_a.write(frame)
    time.sleep(.9)  # wait 900 ms

# Clutch On, Motor Off  -
bite3 = "8" + hexDist[2: -2: 1]
frame = Frame(id_=0xFF0000, data=[0x0F, 0x4A, int(bite2, 16), int(bite3, 16), 0, 0, 0, 0], flags=canlib.MessageFlag.EXT)
ch_a.write(frame)
time.sleep(.02)  # wait 20 ms

# Clutch off, Motor Off  -
bite3 = "0" + hexDist[2: -2: 1]
frame = Frame(id_=0xFF0000, data=[0x0F, 0x4A, int(bite2, 16), int(bite3, 16), 0, 0, 0, 0], flags=canlib.MessageFlag.EXT)
ch_a.write(frame)

# To make sure the message was sent we will attempt to read the message. Using
# timeout, only 500 ms will be spent waiting to receive the CANFrame. If it takes
# longer the program will encounter a timeout error. read the CANFrame by calling
# .read() on the channel that receives the message, To
# then read the message we will use print() and send msg as the input.
msg = ch_a.read(timeout=500)
print(msg)

# After the message has been sent, received and read it is time to inactivate
# the CAN chip. To do this call .busOff() on both channels that went .busOn()
ch_a.busOff()
# ch_b.busOff()

# Lastly, close all channels with close() to finish up.
ch_a.close()
# ch_b.close()

# Depending on the situation it is not always necessary or preferable to go of
# the bus with the channels and, instead only use close().