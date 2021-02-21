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
	num = 1

	x = 0
	w = 5
	r = 0
	g = 0
	b = 100
	mode = 0
	mode_data = 0

	data = [num, x & 0xff << 8, x & 0xff, w & 0xff << 8, w & 0xff, r, g, b, mode, mode_data] 
	data = [num, x & 0xff << 8, x & 0xff, w & 0xff << 8, w & 0xff, r, g, b, mode, mode_data, (x+10) & 0xff << 8, (x+10) & 0xff, w & 0xff << 8, w & 0xff, r, g, b, mode, mode_data] 
	#chan.add_fade(data + data2, 0)
	chan.add_fade(data, 0)
	print('length:', len(data))
	await chan.wait_till_fade_complete()
	return
	chan.add_fade(data, 5000)
	await chan.wait_till_fade_complete()

	x = 115
	
	data = [num, x & 0xff << 8, x & 0xff, w & 0xff << 8, w & 0xff, r, g, b, mode, mode_data] 
	data = [num, x & 0xff << 8, x & 0xff, w & 0xff << 8, w & 0xff, r, g, b, mode, mode_data] 
	chan.add_fade(data, 10000)
	await chan.wait_till_fade_complete()


async def main():
	if False:
		await raw_wave()
		return
	node = ArtNetNode(ip)
	await node.start()
	universe = node.add_universe(0)
#	channel = universe.add_channel(start=1, width=18)
	channel = universe.add_channel(start=1, width= 1 + 2*9)

#	await test_reset(channel)
#	await test_static(channel)
#	await raw_wave()
#	await test_reset(channel)
	await test2(channel)
	print("done")


loop = asyncio.get_event_loop()
loop.run_until_complete(main())
