# ISC License
#
# Copyright © 2017, Simonas Kazlauskas
# Copyright © 2018, Pedro Fanha
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above1
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
# REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
# INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
# LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
# OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.

import colorsys, time, subprocess, os

process_path = os.getenv('MSIRGB_PATH')

if process_path == None:
  print("Error: Can't find MSIRGB. Set MSIRGB_PATH to its full path.")
  os._exit(1)

if os.path.isfile(process_path) == False:
  print("Error: Can't find MSIRGB. The path specified is invalid.")
  os._exit(1)

i=0
while True:
  # Transforms the tuple (r, g, b) from colorsys.hsv_to_rgb
  # into a list of the type [RR, GG, BB] where RR, GG, BB are the HEX values for R, G and B with 02 padding
  # r, g, b have range (0, 1), hence * 15 gives an equivalent hex value in decimal, which is what we need
  c = list(map(lambda x: int(('{0:01x}'.format(int(15*x)))*2, 16), colorsys.hsv_to_rgb((i % 96.0) / 96.0, 0.9, 1.0)))
  
  r = c[0]
  g = c[1]
  b = c[2]

  # Convert the separate R, G and B values to a single color value of the form 0xRRGGBB
  c1 = c2 = c3 = c4 = ((r << 16) | (g << 8) | b)

  subprocess.call([process_path, 'config', '-d511', '-f0', str(c1), str(c2), str(c3), str(c4)])
  time.sleep(0.1)
  i+=1
