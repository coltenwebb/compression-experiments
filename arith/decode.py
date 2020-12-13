import bitstring
import time

s = bitstring.ConstBitStream(filename='eout')
msg_length = s.read(64).uint
cum = [None] * 256
for i in range(256):
    cum[i] = s.read(64).float

# print(cum)
# print(msg_length)
msg = b''

# the fraction part of a double consists of 52 bits,
# we say 51 just in case. won't matter, since you would need
# the file to be more than an exabyte (2^50) with
# some character appearing only once
# for there to be a problem
P = 51
v = 0
b = 0
l = 1

# initialize v with enough bytes to somewhat precise
for i in range(1, P+1):
    v += s.read(1).uint * (2 ** -i)

def find_character():
    global b,l
    sidx = 255
    x = b + l * cum[sidx]
    y = b + l
    #print('v',v)
    while x > v:
        sidx-=1
        y = x
        x = b + l * cum[sidx]
    b = x
    #l = y-x
    l *= cum[sidx+1]-cum[sidx] if sidx < 255 else 1-cum[sidx]
    global msg
    msg += bytes(sidx)
    #print(chr(sidx), 'b', b, 'l', l, 'v', v)
    print(chr(sidx), end='')
    #print('lbin', bitstring.pack('float:64', l).bin)

def fix_b():
    global b,v
    b -= 1
    v -= 1

def fix_l():
    global b,v,l
    #print('fixl', b, l, v)
    if b >= .5:
        b -= .5
        v -= .5
    b *= 2
    v *= 2
    l *= 2
    try:
        v += s.read(1).uint * (2 ** -P)
    except bitstring.ReadError as e:
        pass


for k in range(msg_length):
    #print('b',b,'l',l)
    find_character()
    if b >= 1:
        fix_b()
    while l <= .5:
        fix_l()
        #time.sleep(.5)
    #time.sleep(.5)

