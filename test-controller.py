import asyncio
from pyartnet import ArtNetNode
import math

ip = '192.168.1.5'
delay = 25
#delay = 5
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
			#data += [255 if i == tick % leds else 0, 0, 0]
		channel.add_fade(data, delay)
		await channel.wait_till_fade_complete()

async def test_static(chan):
	mode = 1
	leds = 16
	group = 3
	phase = 3
	spacing = 15 
	state_handling = 0
	color_blending = 0
	speed = 0
	direction = 0
	pr = 20
	pg = 0
	pb = 0
	sr = 0
	sb = 20
	sg = 0
	tr = 0
	tg = 0
	tb = 20
	data = [mode, leds, group, phase, spacing, state_handling, color_blending, speed, direction, pr, pg, pb, sr, sb, sg, tr, tg, tb] 
	chan.add_fade(data, 0)
	await chan.wait_till_fade_complete()

async def test_reset(chan):
	chan.add_fade([0]*18, 0)
	await chan.wait_till_fade_complete()

async def test2(chan):
	b1 = Blob()
	b1.w = 5
	b1.r = 50
	b2 = Blob()
	b2.x = 10
	b2.w = 5
	b2.g = 50
	b3 = Blob()
	b3.x = 13
	b3.w = 3
	b3.b = 50

	data = toChans(b1, b2, b3)
	print(data)
	print(len(data))
	chan.add_fade(data, 0)
	await chan.wait_till_fade_complete()

	b1.x += 100
	b2.x += 100
	b3.x += 100
	data = toChans(b1, b2, b3)
	chan.add_fade(data, 5000)
	await chan.wait_till_fade_complete()


class Blob:
	x = 0
	w = 0
	r = 0
	g = 0
	b = 0
	mode = 0
	data = 0

	def toBytes(self):
		return [self.x & 0xff << 8, self.x & 0xff, self.w & 0xff << 8, self.w & 0xff, self.r, self.g, self.b, self.mode, self.data]

def toChans(*blobs):
	total = list()
	for b in blobs:
		total += b.toBytes()
	return [len(blobs), *total]

async def main():
	if False:
		await raw_wave()
		return
	node = ArtNetNode(ip)
	await node.start()
	universe = node.add_universe(0)
#	channel = universe.add_channel(start=1, width=18)
	channel = universe.add_channel(start=1, width= 1 + 3*9)

#	await test_reset(channel)
#	await test_static(channel)
#	await raw_wave()
#	await test_reset(channel)
	await test2(channel)
	print("done")

	old = node
	node = ArtNetNode(ip)
	node._socket = old._socket
	await node.start()
	universe = node.add_universe(0)
	channel = universe.add_channel(start=1, width= 1 + 3*9)
	await test2(channel)
	print("done")



loop = asyncio.get_event_loop()
loop.run_until_complete(main())

