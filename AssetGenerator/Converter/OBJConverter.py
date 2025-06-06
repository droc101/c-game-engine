import struct
import util
from dataclasses import dataclass

#region Types & Helpers

@dataclass
class Vector3:
	x : float
	y : float
	z : float

	def __eq__(self, value):
		return self.x == value.x and self.y == value.y and self.z == value.z
	
	def __hash__(self):
		return hash((self.x, self.y, self.z))

	def pack(self):
		ba = bytearray()
		ba += struct.pack("fff", *[self.x,self.y,self.z])
		return ba

@dataclass	
class Vector2:
	x : float
	y : float

	def __eq__(self, value):
		return self.x == value.x and self.y == value.y
	
	def __hash__(self):
		return hash((self.x, self.y))
	
	def pack(self):
		ba = bytearray()
		ba += struct.pack("ff", *[self.x,self.y])
		return ba

@dataclass
class Vertex:
	pos : Vector3
	norm : Vector3
	uv : Vector2

	def __eq__(self, value):
		return self.pos == value.pos and self.norm == value.norm and self.uv == value.uv

	def __hash__(self):
		return hash((self.pos, self.norm, self.uv))
	
	def pack(self):
		ba = bytearray()
		ba += self.pos.pack()
		ba += self.uv.pack()
		ba += self.norm.pack()
		return ba

@dataclass
class ObjModel:
	# Working Data
	positions : list[Vector3]
	normals : list[Vector3]
	uvs : list[Vector2]
	vertex_index_map : dict[str, int]

	# Final Data
	verts : list[Vertex]
	materials : dict[str, list[int]] # list of indices for each material
	total_index_count : int

	def __init__(self):
		self.positions = []
		self.normals = []
		self.uvs = []
		self.verts = []
		self.vertex_index_map = {}
		self.materials = {}
		self.total_index_count = 0

	def pack(self):
		# Pack vertex data into a binary format
		vtx_bin_data = bytearray()
		for vertex in self.verts:
			vtx_bin_data += vertex.pack()
		
		# Pack index data into a binary format

		idx_count_data = bytearray()
		idx_bin_data = bytearray()

		mat_idx = 0
		for mat in self.materials:
			idx_count_data += util.IntToBytes(len(self.materials[mat]))
			for idx in self.materials[mat]:
				idx_bin_data += util.IntToBytes(idx)
			mat_idx += 1

		# Make the header and add the data
		bin_data = bytearray()
		bin_data.extend(util.IntToBytes(len(self.verts)))
		bin_data += vtx_bin_data
		bin_data += util.IntToBytes(self.total_index_count)
		bin_data += idx_count_data
		bin_data += idx_bin_data
		
	
		return bin_data

# Create a Vector3 from a string
def v3l(line : str):

	floats = list(map(float, line.split()[1:]))
	return Vector3(floats[0], floats[1], floats[2])

# Create a Vector2 from a string
def v2l(line : str):
	floats = list(map(float, line.split()[1:]))
	return Vector2(floats[0], floats[1])

# Find the index of a vertex in the vertex list (or add it if it doesn't exist)
def find_vtx_index(obj : ObjModel, vtx : Vertex):
	if vtx in obj.vertex_index_map:
		return obj.vertex_index_map[vtx]
	else:
		obj.vertex_index_map[vtx] = len(obj.verts)
		obj.verts.append(vtx)
		return len(obj.verts) - 1

#endregion

# Parse an OBJ model into a binary format
# This parser only supports the following OBJ features:
# - Vertex positions
# - Vertex normals
# - Vertex UVs
def ParseOBJ(file_path):
	obj : ObjModel = ObjModel()
	
	# Loop through the file once to get the vertex data
	with open(file_path, 'r') as f:
		for line in f:
			if line.startswith('v '):
				obj.positions.append(v3l(line))
			elif line.startswith('vt '):
				obj.uvs.append(v2l(line))
				obj.uvs[-1].y = 1 - obj.uvs[-1].y # for some reason it's upside down because why not
			elif line.startswith('vn '):
				obj.normals.append(v3l(line))

	# Loop through the file a second time to get the face index data
	with open(file_path, 'r') as f:
		mat = "DefaultMaterial"
		for line in f:
			if line.startswith('usemtl '):
				mat = line.split()[1]
				if mat not in obj.materials:
					obj.materials[mat] = []
			if line.startswith('f '):
				face = line.split()[1:]

				if mat == "DefaultMaterial":
					print("Warning: No material specified for face. Using DefaultMaterial.")
	
				for v in face:

					v_idx, vt_idx, vn_idx = v.split('/')

					# one based indexing 😀
					v_idx = int(v_idx) - 1
					vt_idx = int(vt_idx) - 1
					vn_idx = int(vn_idx) - 1

					unique_vertex = Vertex(obj.positions[v_idx], obj.normals[vn_idx], obj.uvs[vt_idx])
					idx = find_vtx_index(obj, unique_vertex)
					# Add the index to the material list
					if mat in obj.materials:
						obj.materials[mat].append(idx)
					else:
						obj.materials[mat] = [idx]

	for _, indices in obj.materials.items():
		obj.total_index_count += len(indices)

	# # Pack vertex data into a binary format
	# vtx_bin_data = bytearray()
	# for vertex in obj.verts:
	# 	vtx_bin_data += vertex.pack()
	
	# # Pack index data into a binary format
	# idx_bin_data = bytearray()
	# for idx in obj.inds:
	# 	idx_bin_data += util.IntToBytes(idx)
 
	# # Make the header and add the data
	# bin_data = bytearray()
	# bin_data += struct.pack('4s', b'MSH\0')
	# bin_data.extend(util.IntToBytes(len(obj.inds)))
	# bin_data.extend(util.IntToBytes(len(obj.verts)))
	# bin_data += struct.pack('4s', b'DAT\0')
	# bin_data += idx_bin_data
	# bin_data += vtx_bin_data
 
	# return bin_data

	return obj

# def ConvertOBJ(path):
# 	data = list(ParseOBJ(path))

# 	data += util.IntToBytes(len(data))
# 	data += util.IntToBytes(0)
# 	data += util.IntToBytes(0)
# 	data += util.IntToBytes(0)

# 	util.WriteAsset(path, "gmdl", "model", util.EncloseData(data, 7))