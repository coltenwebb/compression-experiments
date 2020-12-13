import bitstring
import time

out = bitstring.BitArray() 

msg = open("./enwik8", "rb").read()

# do a pass through to find the cummulative distribution
msg_length = 0
cum = [0] * 256
for c in msg:
    cum[c+1]+=1
    msg_length += 1
for i in range(1, len(cum)):
    cum[i] = cum[i-1] + (cum[i])/msg_length
    if cum[i] > 1:
        cum[i] = 1

#print(cum)

out.append(bitstring.pack('uint:64', msg_length))
for ch in range(256):
    bf = bitstring.pack('float:64', cum[ch])
    out.append(bf)
    cum[ch] = bf.read('float:64')

#print(msg_length)
#print(cum)

b = 0
l = 1

# we make sure l is always bigger than .5
def renormalize():
    # use b as lower
    # [0,1) means what used to be [b,b+.5)
    global b,l
    if b >= .5:
        out.append('0b1')
        b -= .5
    else:
        out.append('0b0')
    b *= 2
    l *= 2


# if b>=1, we just need to bump the frame up
# to do this, we modify bits and b
# b is never >=2
def carry():
    global b,l
    b -= 1
    for i in range(len(out)-1, -1,-1):
        out[i] ^= 1
        if out[i] == 1:
            break
        

# encode the next character in the sequence
for c in msg:
    b += l * cum[c]
    l *= cum[c+1]-cum[c] if c < 255 else 1-cum[c]
    #print(chr(c), 'b', b, 'l', l)
    #time.sleep(.05)
    #print('lbin', bitstring.pack('float:64', l).bin)
    if b >= 1:
        carry()
    while l <= .5:
        renormalize()

# select code value
# note that .5<=l<=1 at this point, so we 
# need at most 1 extra bit
if b < .5:
    out.append('0b1')
else:
    carry()
    out.append('0b0')


#print(out.bin)

of = open('eout', 'wb')
out.tofile(of)
