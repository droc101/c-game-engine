//
// Created by droc101 on 5/19/26.
//

#include <engine/assets/DataReader.h>
#include <engine/assets/DataWriter.h>
#include <engine/assets/KvlFile.h>
#include <engine/structs/KVList.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KVL_MAGIC 0x464c564b // "KVLF" in ASCII
#define KVL_VERSION 1

typedef struct KvlFileHeader KvlFileHeader;

struct KvlFileHeader
{
	uint32_t magic;
	uint16_t version;
	uint16_t checksum;
} __attribute__((packed));

bool ReadKvlFile(const char *path, KvList output)
{
	FILE *file = fopen(path, "rb");
	if (file == NULL)
	{
		LogError("Could not open KvlFile \"%s\": %s.\n", path, strerror(errno));
		return false;
	}
	fseek(file, 0, SEEK_END);
	const size_t fileLen = ftell(file);

	if (fileLen < sizeof(KvlFileHeader) + sizeof(size_t)) // header + number of KvList keys
	{
		LogError("KvlFile \"%s\" is invalid\n", path);
		fclose(file);
		return false;
	}

	const size_t bufferSize = fileLen - sizeof(KvlFileHeader);

	fseek(file, 0, SEEK_SET);
	KvlFileHeader header;
	fread(&header, 1, sizeof(KvlFileHeader), file);
	if (header.magic != KVL_MAGIC)
	{
		LogError("KvlFile magic is incorrect (expected %x, got %x)\n", KVL_MAGIC, header.magic);
		fclose(file);
		return false;
	}
	if (header.version != KVL_VERSION)
	{
		LogError("KvlFile version is incorrect (expected %d, got %d)\n", KVL_VERSION, header.version);
		fclose(file);
		return false;
	}

	void *buffer = malloc(bufferSize);
	CheckAlloc(buffer);
	fread(buffer, bufferSize, 1, file);

	if (Checksum(buffer, bufferSize) != header.checksum)
	{
		LogError("KvlFile \"%s\" is damaged\n", path);
		free(buffer);
		fclose(file);
		return false;
	}

	DataReader *reader = CreateDataReader(buffer, bufferSize, 0);
	ReadKvList(reader, output);
	free(buffer);
	DestroyDataReader(reader);

	return true;
}

bool WriteKvlFile(const char *path, KvList input)
{
	DataWriter *writer = CreateDataWriter();
	WriteKvList(input, writer);
	KvListDestroy(input);

	const KvlFileHeader header = {.magic = KVL_MAGIC,
								  .version = KVL_VERSION,
								  .checksum = Checksum(DataWriterGetBuffer(writer), DataWriterGetBufferSize(writer))};

	FILE *file = fopen(path, "wb");
	if (file == NULL)
	{
		LogError("Failed to write KvlFile \"%s\" fopen failed with \"%s\"\n", path, strerror(errno));
		return false;
	}
	fwrite(&header, sizeof(KvlFileHeader), 1, file);
	fwrite(DataWriterGetBuffer(writer), DataWriterGetBufferSize(writer), 1, file);
	fclose(file);

	FreeDataWriter(writer);
	return true;
}
