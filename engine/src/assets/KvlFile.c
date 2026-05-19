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

bool ReadKvlFile(const char *path, KvList output)
{
	FILE *file = fopen(path, "rb");
	if (file == NULL)
	{
		LogError("Could not fopen KvlFile \"%s\".\n", path);
		return false;
	}
	fseek(file, 0, SEEK_END);
	const size_t fileLen = ftell(file);

	if (fileLen < sizeof(size_t) + sizeof(uint16_t)) // number of KvList keys + checksum
	{
		LogError("KvlFile \"%s\" is invalid\n", path);
		fclose(file);
		return false;
	}

	const size_t bufferSize = fileLen - sizeof(uint16_t);
	void *buffer = malloc(bufferSize);
	CheckAlloc(buffer);

	fseek(file, bufferSize, SEEK_SET);
	uint16_t checksum = 0;
	fread(&checksum, 1, sizeof(uint16_t), file);

	fseek(file, 0, SEEK_SET);
	fread(buffer, bufferSize, 1, file);

	if (Checksum(buffer, bufferSize) != checksum)
	{
		LogError("KvlFile \"%s\" is damaged\n", path);
		free(buffer);
		fclose(file);
		return false;
	}

	size_t offset = 0;
	ReadKvList(buffer, bufferSize, &offset, output);
	free(buffer);

	return true;
}

bool WriteKvlFile(const char *path, KvList input)
{
	DataWriter *writer = CreateDataWriter();
	WriteKvList(input, writer);
	KvListDestroy(input);
	const uint16_t checksum = Checksum(DataWriterGetBuffer(writer), DataWriterGetBufferSize(writer));

	FILE *file = fopen(path, "wb");
	if (file == NULL)
	{
		LogError("Failed to write KvlFile \"%s\" fopen failed with \"%s\"\n", path, strerror(errno));
		return false;
	}
	fwrite(DataWriterGetBuffer(writer), DataWriterGetBufferSize(writer), 1, file);
	fwrite(&checksum, sizeof(uint16_t), 1, file);
	fclose(file);

	FreeDataWriter(writer);
	return true;
}
