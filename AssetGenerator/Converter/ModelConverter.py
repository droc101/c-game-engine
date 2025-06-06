import struct
import os
import util
import json
from dataclasses import dataclass
from Converter.OBJConverter import *

@dataclass
class ModelMaterial:
	texture : str
	color : list[int]
	shader : int

	def __init__(self):
		self.texture = ""
		self.color = [255, 255, 255, 255]
		self.shader = 2

@dataclass
class ModelLod:
	distance : float
	model : ObjModel

	def __init__(self):
		self.distance = 0.0
		self.model = ObjModel()

@dataclass
class ModelDefinition:
	material_count : int
	skins : list[dict[str, ModelMaterial]]
	lods : list[ModelLod]

	def __init__(self):
		self.material_count = 0
		self.skins = []
		self.lods = []

model_shader_map = [
	"sky",
	"unshaded",
	"shaded"
]

def ConvertModelDefinition(path):
	with open(path, 'r') as file:
		data = json.load(file)
		
		mdef = ModelDefinition()
		mdef.material_count = data["materials"]
		mdef.skins = []
		for skin in data["skins"]:
			skin_data = {}
			for mat in skin:
				mat_data = skin[mat]
				material = ModelMaterial()
				material.texture = "texture/" + mat_data["texture"] + ".gtex"
				material.color = mat_data["color"]
				material.shader = model_shader_map.index(mat_data["shader"])
				skin_data[mat] = material

			if len(skin) != mdef.material_count:
				raise ValueError(f"Skin {skin} has {len(skin)} materials, expected {mdef.material_count}")
			mdef.skins.append(skin_data)

		mdef.lods = []
		for lod in data["lods"]:
			lod_obj = ModelLod()
			lod_obj.distance = lod["distance"]

			obj_path = util.input_path + "model/" + lod["model"]
			if not obj_path.endswith(".obj"):
				raise ValueError(f"Model {obj_path} is not an OBJ file")
			
			if not os.path.exists(obj_path):
				raise FileNotFoundError(f"Model {obj_path} does not exist")

			lod_obj.model = ParseOBJ(obj_path)
			mdef.lods.append(lod_obj)

	mdef.lods.sort(key=lambda lod: lod.distance)

	if len(mdef.lods) == 0:
		raise ValueError("Model has no LODs")
	if len(mdef.skins) == 0:
		raise ValueError("Model has no skins")
	if mdef.material_count == 0:
		raise ValueError("Model has no materials")
	if mdef.lods[0].distance != 0.0:
		raise ValueError("Model does not have a LOD with a distance of 0")

	bin_data = bytearray()
	bin_data.extend(util.IntToBytes(mdef.material_count))
	bin_data.extend(util.IntToBytes(len(mdef.skins)))
	bin_data.extend(util.IntToBytes(len(mdef.lods)))
	for skin in mdef.skins:
		for mat in skin:
			mat_data = skin[mat]
			bin_data.extend(util.CString(mat_data.texture, 64).encode("ascii"))
			color_int = (mat_data.color[0] << 16) | (mat_data.color[1] << 8) | (mat_data.color[2] << 0) | 0xFF000000
			bin_data.extend(util.IntToBytes(color_int))
			bin_data.extend(util.IntToBytes(mat_data.shader))
	for lod in mdef.lods:
		bin_data.extend(struct.pack("f", lod.distance))
		lod_bin_data = lod.model.pack()
		bin_data.extend(lod_bin_data)

	util.WriteAsset(path, "gmdl", "model", util.EncloseData(bin_data, 7))