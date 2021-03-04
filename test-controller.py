import asyncio
from pyartnet import ArtNetNode
import math
import sys

ip = '192.168.1.5'
ip = '13.37.69.1'
ip = '10.0.1.1'
delay = 25
# delay = 5
leds = 120
dim_min = 0
dim_max = 0.5


def wave(i):
    i %= leds
    rad = (i / leds) * 2 * math.pi
    val = math.sin(rad) + 1
    val *= (dim_max - dim_min) / 2
    val *= 127.5
    return int(val)


async def raw_wave():
    node = ArtNetNode(ip, max_fps=1500, refresh_every=0)
    node.max_fps = 1500
    node.sleep_time = 1 / node.max_fps
    print('max fps:', node.max_fps)
    print('sleep time:', node.sleep_time)
    await node.start()
    universe = node.add_universe(0)
    channel = universe.add_channel(start=1, width=1 + leds*3)

    tick = 0

    while True:
        tick += 1
        data = [255]
        for i in range(leds):
            i += tick
            offset = leds // 3
            data += [wave(i), wave(i + offset), wave(i + 2*offset)]
            # data += [255 if i == tick % leds else 0, 0, 0]
        channel.add_fade(data, delay)
        await channel.wait_till_fade_complete()


async def test_single():
    b1 = Blob()
    b1.w = 40
    b1.r = 200
    b1.x = 0
    b1.mode = 0
    chan = await render(b1)
    return

    b1.x = 255
    await render(b1, time=1000)

    b1.x = 0
    await render(b1, time=1000)

    b1.r = 0
    await render(b1, time=1000)


async def test_two_fade():
    b1 = Blob()
    b1.w = 21
    b1.r = 100
    b1.mode = 0

    b2 = Blob()
    b2.w = 21
    b2.b = 100
    b2.mode = 1
    b2.x = 50
    chan = await render(b1, b2)

    b2.x = 0
    b1.x = 50
    await render(b1, b2, time=5000, chan=chan)


class Blob:
    x = 0
    w = 0
    r = 0
    g = 0
    b = 0
    mode = 0
    data = 0

    def toBytes(self):
        return [(self.x & (0xff << 8)) >> 8, self.x & 0xff, (self.w & (0xff << 8)) >> 8, self.w & 0xff, self.r, self.g, self.b, self.mode, self.data]


def toChans(*blobs):
    total = list()
    for b in blobs:
        total += b.toBytes()
    return [len(blobs), *total]


async def render(*blobs, time=0, chan=None):
    data = toChans(*blobs)
    if not chan:
        chan = await newChan(len(data))
    chan.add_fade(data, time)
    await chan.wait_till_fade_complete()
    return chan


async def newChan(chans):
    node = ArtNetNode(ip)
    await node.start()
    universe = node.add_universe(0)
    return universe.add_channel(start=1, width=chans)


async def main():
    if False:
        await raw_wave()
        return

    name = sys.argv[1] if len(sys.argv) > 1 else "test_1"
    if name not in globals():
        print('function not found')
        print('available:')
        print(' - ' + '\n - '.join([name for name in set(globals()) if name.startswith('test')]))
        return
    test = globals()[name]
    await test()
    print("done")


loop = asyncio.get_event_loop()
loop.run_until_complete(main())
