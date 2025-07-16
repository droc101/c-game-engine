import os
import sys
import shutil
import util

# Converters
import Converter.TextureConverter
import Converter.FileConverter
import Converter.MP3Converter
import Converter.ModelConverter
import Converter.VulkanShaderConverter
import Converter.WAVConverter
import Converter.GLShaderConverter
import Converter.FontConverter

if (len(sys.argv) != 3):
	print("Usage: python genassets.py <input_path> <output_path>")
	sys.exit(1)

util.input_path = sys.argv[1]
util.output_path = sys.argv[2]


# Recreate asset folder structure
def SetupDirs(out_path):
	if os.path.exists(out_path):
		shutil.rmtree(out_path)
	
	os.makedirs(out_path)
	os.makedirs(out_path + "texture/")
	os.makedirs(out_path + "audio/")
	os.makedirs(out_path + "level/")
	os.makedirs(out_path + "glshader/")
	os.makedirs(out_path + "vkshader/")
	os.makedirs(out_path + "model/")
	os.makedirs(out_path + "font/")
	os.makedirs(out_path + "defs/")

# Recursively search for files in the input folder and convert them
def RecursiveSearch(in_path, out_path):
	files = os.listdir(in_path)
	
	# Sort the files so that the order is deterministic
	files.sort()
	
	for file in files:
		if os.path.isdir(in_path + file):
			RecursiveSearch(in_path + file + "/", out_path)
		else:
			path_from_assets = in_path.split("Assets/")[1]
			if file.endswith(".png"):
				print("Converting " + path_from_assets + file)
				Converter.TextureConverter.ConvertPNG(in_path + file)
			elif file.endswith(".mp3"):
				print("Converting " + path_from_assets + file)
				Converter.MP3Converter.ConvertMP3(in_path + file)
			elif file.endswith(".wav"):
				print("Converting " + path_from_assets + file)
				Converter.WAVConverter.ConvertWAV(in_path + file)
			elif file.endswith(".bin"):
				print("Converting " + path_from_assets + file)
                Converter.FileConverter.ConvertFile(in_path + file, 3, "gmap", "level")
			elif file.endswith(".frag"):
				print("Converting " + path_from_assets + file)
				Converter.VulkanShaderConverter.ConvertVulkanFrag(in_path + file)
			elif file.endswith(".vert"):
				print("Converting " + path_from_assets + file)
				Converter.VulkanShaderConverter.ConvertVulkanVert(in_path + file)
			elif file.endswith(".model.json"):
				print("Converting " + path_from_assets + file)
				Converter.ModelConverter.ConvertModelDefinition(in_path + file)
			elif file.endswith(".glsl"):
				print("Converting " + path_from_assets + file)
				Converter.GLShaderConverter.ConvertGLShader(in_path + file)
			elif file.endswith(".font"):
				print("Converting " + path_from_assets + file)
				Converter.FontConverter.ConvertFont(in_path + file)
			elif file.endswith(".def"):
				print("Copying " + path_from_assets + file)
				shutil.copyfile(in_path + file, out_path + "defs/" + file)

SetupDirs(util.output_path)
RecursiveSearch(util.input_path, util.output_path)
