import util
import json
import struct

def ConvertFont(path):
	file = open(path, "rb")
	font = json.loads(file.read())

	widths = []
	indices = []
	unknownCharacterUvOffset = font["chars"].find('?')

	if unknownCharacterUvOffset == -1:
		print("Font does not contain a '?' character, skipping")
		return

	for i in range(128):
		widths.append(font["default_char_width"])
		index = font["chars"].find(chr(i).upper() if font["uppercase_only"] else chr(i))
		indices.append(unknownCharacterUvOffset if index == -1 else index)
	
	for char in font["char_widths"]:
		if (ord(char) > 128) or (ord(char) < 0):
			print("Invalid character in font: '" + char + "' (" + str(ord(char)) + "), skipping")
			continue
		widths[ord(char)] = font["char_widths"][char]
	
	if len(widths) != 128:
		print("Invalid number of characters in font, must be 128")
		return

    texnam = util.CString(font["texture"], 80)
	# chars = util.CString(font["chars"], 128)

	data = bytearray([])
	data += struct.pack("I", font["width"])
	data += struct.pack("I", font["total_height"])
	data += struct.pack("I", font["baseline"])
	data += struct.pack("I", font["char_spacing"])
	data += struct.pack("I", font["line_spacing"])
	data += struct.pack("I", font["space_width"])
	data += struct.pack("I", font["default_size"])
	data += struct.pack("I", len(font["chars"]))
	data += struct.pack("B", font["uppercase_only"])
	data += texnam.encode("ascii")
	for index in indices:
		data += struct.pack("B", index)
	for width in widths:
		data += struct.pack("B", width)

	util.WriteAsset(path, "gfon", "font", util.EncloseData(data, 8))