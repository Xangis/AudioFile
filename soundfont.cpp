#include "soundfont.h"
#include <stdio.h>
#include <cstring>

SoundFont::SoundFont()
{
	_loaded = false;
}

SoundFont::~SoundFont()
{
}

bool SoundFont::Load(const char* filename)
{
    // TODO: Implement loading code.

	// A typical SoundFont file starts with RIFF and contains three LIST chunks after an sfbk tag:
	// one containing INFO ifil chunk metadata
	// one containing sdta smpl data
	// one containing pdta phdr data
	FILE* fp = fopen(filename, "rb");
	if( fp != NULL )
	{
		unsigned long value;
		unsigned char *charPtr = (unsigned char*)&value;
		int read = fread(charPtr, 4, 1, fp);
		if( memcmp(charPtr, "RIFF", 4) != 0 )
		{
			printf("Not a valid SoundFont file.");
			fclose(fp);
			return false;
		}
		read = fread(charPtr, 4, 1, fp);
		unsigned int chunkSize = value;
		unsigned char* chunkData = new unsigned char[chunkSize];
		read = fread(chunkData, chunkSize, 1, fp);
		if( memcmp(chunkData, "sfbk", 4) != 0 )
		{
			printf("Soundfont sfbk data block not found.");
			return false;
		}
		unsigned char* ptr = &(chunkData[4]);
		while( ptr < chunkData + chunkSize )
		{
			char cdata[4];
			cdata[0] = *ptr;
			cdata[1] = *(ptr+1);
			cdata[2] = *(ptr+2);
			cdata[3] = *(ptr+3);
			ptr += 4;
			memcpy(charPtr, ptr, 4);
			unsigned int subChunkSize = value;
			ptr += 4;
			unsigned char* subChunkData = new unsigned char[subChunkSize];
			memcpy(subChunkData, ptr, subChunkSize);
			ptr += subChunkSize;
			delete[] subChunkData;
		}
		delete[] chunkData;
	}
	fclose(fp);
	return true;
}