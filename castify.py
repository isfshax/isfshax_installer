#!/usr/bin/env python3

import sys, os, struct
import __future__

from base64 import b16decode
from Cryptodome.Cipher import AES
from Cryptodome.Hash import SHA

#To get the IV: Compile dimok789/FIX94's iosuhax, copy them out of scripts/keys.py.
#make sure to capitalise all the letters
key = b16decode(b"B5XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX")
iv = b16decode(b"91XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX")

no_crypto = False

loaderfile = sys.argv[1]
elffile = sys.argv[2]
outfile = sys.argv[3]

print("Building payload...\n")

data = open(loaderfile, "rb").read()

hdrlen, loaderlen, elflen, arg = struct.unpack(">IIII", data[:16])

if hdrlen < 0x10:
    print("ERROR: header length is 0x%X, expected at least 0x10." % hdrlen)
    sys.exit(1)

loaderoff = hdrlen
elfoff = loaderoff + loaderlen
elfend = elfoff + elflen

hdr = data[:hdrlen]
loader = data[loaderoff:elfoff]

elf = open(elffile,"rb").read()

if elflen > 0:
    print("WARNING: loader already contains ELF, will replace.")

elflen = len(elf)

if loaderlen < len(loader):
    print("ERROR: loader is larger than its reported length.")
    sys.exit(1)

if loaderlen > len(loader):
    print("Padding loader with 0x%X zeroes." % (loaderlen - len(loader)))
    loader += b"\x00" * (loaderlen - len(loader))

print("Header size: 0x%X bytes." % hdrlen)
print("Loader size: 0x%X bytes." % loaderlen)
print("ELF size:    0x%X bytes." % elflen)

payload = struct.pack(">IIII", hdrlen, loaderlen, elflen, 0) + hdr[16:]
payload += loader
payload += elf

print("\nBuilding ancast image...\n")

hb_flags = 0x0000

payloadlen = len(payload)
if ((payloadlen + 0xFFF) & ~0xFFF) > payloadlen:
    print("Padding payload with 0x%X zeroes." % (((payloadlen + 0xFFF) & ~0xFFF) - payloadlen))
    payload += b"\x00" * (((payloadlen + 0xFFF) & ~0xFFF) - payloadlen)

if no_crypto:
    hb_flags |= 0b1
else:
    c = AES.new(key, AES.MODE_CBC, iv)
    payload = c.encrypt(payload)

h = SHA.new(payload)

outdata = struct.pack(">I4xI20xI256x124xHBBIII20sI56x", 0xEFA282D9, 0x20, 0x02, hb_flags, 0x00, 0x00, 0x21, 0x02, len(payload), h.digest(), 0x02)
outdata += payload

print("Body size:   0x%X bytes." % len(payload))
print("Body hash:   %s." % h.hexdigest())

f = open(outfile, "wb")
f.write(outdata)
f.close()
