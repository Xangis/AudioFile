#include "soundfont.h"
#include <stdio.h>
#include <cstring>

SoundFont::SoundFont()
{
	_loaded = false;
	_name = NULL;
	_engine = NULL;
	_product = NULL;
	_engineer = NULL;
	_software = NULL;
	_date = NULL;
	_comment = NULL;
	_copyright = NULL;
	_sampleData = NULL;
	_rom = NULL;
	_sampleDataLength = 0;
}

SoundFont::~SoundFont()
{
	if( _name != 0 ) delete _name;
	if( _engine != 0 ) delete _engine;
	if( _product != 0 ) delete _product;
	if( _engineer != 0 ) delete _engineer;
	if( _software != 0 ) delete _software;
	if( _date != 0 ) delete _date;
	if( _comment != 0 ) delete _comment;
	if( _copyright != 0 ) delete _copyright;
	if( _sampleData != 0 ) delete _sampleData;
	if( _rom != 0 ) delete _rom;
	for( std::list<sfSample*>::iterator it = _samples.begin(); it != _samples.end(); it++ )
	{
		delete *it;
	}
	for( std::list<sfPresetHeader*>::iterator it = _presets.begin(); it != _presets.end(); it++ )
	{
		delete *it;
	}
	for( std::list<sfPresetBag*>::iterator it = _presetBags.begin(); it != _presetBags.end(); it++ )
	{
		delete *it;
	}
	for( std::list<sfModList*>::iterator it = _presetModulators.begin(); it != _presetModulators.end(); it++ )
	{
		delete *it;
	}
	for( std::list<sfGenList*>::iterator it = _presetGenerators.begin(); it != _presetGenerators.end(); it++ )
	{
		delete *it;
	}
	for( std::list<sfInst*>::iterator it = _instruments.begin(); it != _instruments.end(); it++ )
	{
		delete *it;
	}
	for( std::list<sfInstBag*>::iterator it = _instrumentBags.begin(); it != _instrumentBags.end(); it++ )
	{
		delete *it;
	}
	for( std::list<sfModList*>::iterator it = _instrumentModulators.begin(); it != _instrumentModulators.end(); it++ )
	{
		delete *it;
	}
	for( std::list<sfInstGenList*>::iterator it = _instrumentGenerators.begin(); it != _instrumentGenerators.end(); it++ )
	{
		delete *it;
	}
}

// Note that this memory is internal to the class. You MUST copy it if you're going to store it.
unsigned char* SoundFont::GetName()
{
	return _name;
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
			if( memcmp( cdata, "LIST", 4 ) == 0 )
			{
				printf("LIST chunk found.");
			}
			ptr += 4;
			memcpy(charPtr, ptr, 4);
			unsigned int subChunkSize = value;
			ptr += 4;
			unsigned char* subChunkData = new unsigned char[subChunkSize];
			memcpy(subChunkData, ptr, subChunkSize);
			ptr += subChunkSize;
			if( memcmp(subChunkData, "INFO", 4) == 0 )
			{
				ProcessInfoListChunk(subChunkData, subChunkSize);
			}
			else if( memcmp(subChunkData, "pdta", 4) == 0 )
			{
				ProcessPdtaListChunk(subChunkData, subChunkSize);
			}
			else if( memcmp(subChunkData, "sdta", 4) == 0 )
			{
				ProcessSdtaListChunk(subChunkData, subChunkSize);
			}
			else
			{
				printf("List contents not recognized.");
			}
			delete[] subChunkData;
		}
		delete[] chunkData;
	}
	fclose(fp);
	_loaded = true;
	return true;
}

bool SoundFont::ProcessInfoListChunk(unsigned char* data, unsigned int size)
{
	unsigned int ptr = 0;
	unsigned long value;
	unsigned char* charData = (unsigned char*)&value;
	memcpy(charData, &(data[ptr]), 4);
	ptr += 4;
	if( memcmp(charData, "INFO", 4) != 0 )
	{
		printf("Invalid INFO chunk.");
		return 0;
	}
	while( ptr < size )
	{
		memcpy(charData, &(data[ptr]), 4);
		ptr += 4;
		if( memcmp( charData, "ifil", 4 ) == 0 )
		{
			printf("ifil section found.");
    		memcpy(charData, &(data[ptr]), 4);
		    ptr += 4;
			printf("Section is %d bytes", value);
			memcpy(&_versionInfo, &(data[ptr]), 4);
			ptr += 4;
			printf("Major version: %d, Minor version: %d", _versionInfo.wMajor, _versionInfo.wMinor);
		}
        else if( memcmp( charData, "INAM", 4 ) == 0 )
		{
			printf("INAM section found.");
    		memcpy(charData, &(data[ptr]), 4);
	    	ptr += 4;
			if( _name != NULL )
			{
				delete[] _name;
			}
			_name = new unsigned char[value];
			memcpy(_name, &(data[ptr]), value);
			printf("Name: %s", _name);
			ptr += value;
		}
		else if( memcmp( charData, "isng", 4 ) == 0 )
		{
			printf("isng section found.");
    		memcpy(charData, &(data[ptr]), 4);
	    	ptr += 4;
			if( _engine != NULL )
			{
				delete[] _engine;
			}
			_engine = new unsigned char[value];
			memcpy(_engine, &(data[ptr]), value);
			printf("Sound Engine: %s", _engine);
			ptr += value;
		}
		else if( memcmp( charData, "IPRD", 4 ) == 0 )
		{
			printf("IPRD section found.");
    		memcpy(charData, &(data[ptr]), 4);
	    	ptr += 4;
			if( _product != NULL )
			{
				delete[] _product;
			}
			_product = new unsigned char[value];
			memcpy(_product, &(data[ptr]), value);
			printf("Intended Product: %s", _product);
			ptr += value;
		}
		else if( memcmp( charData, "IENG", 4 ) == 0 )
		{
			printf("IENG section found.");
    		memcpy(charData, &(data[ptr]), 4);
	    	ptr += 4;
			if( _engineer != NULL )
			{
				delete[] _engineer;
			}
			_engineer = new unsigned char[value];
			memcpy(_engineer, &(data[ptr]), value);
			printf("Engineer: %s", _engineer);
			ptr += value;
		}
		else if( memcmp( charData, "ISFT", 4 ) == 0 )
		{
			printf("ISFT section found.");
    		memcpy(charData, &(data[ptr]), 4);
	    	ptr += 4;
			if( _software != NULL )
			{
				delete[] _software;
			}
			_software = new unsigned char[value];
			memcpy(_software, &(data[ptr]), value);
			printf("Software: %s", _software);
			ptr += value;
		}
		else if( memcmp( charData, "ICRD", 4 ) == 0 )
		{
			printf("ICRD section found.");
    		memcpy(charData, &(data[ptr]), 4);
	    	ptr += 4;
			if( _date != NULL )
			{
				delete[] _date;
			}
			_date = new unsigned char[value];
			memcpy(_date, &(data[ptr]), value);
			printf("Creation Date: %s", _date);
			ptr += value;
		}
		else if( memcmp( charData, "ICMT", 4 ) == 0 )
		{
			printf("ICMT section found.");
    		memcpy(charData, &(data[ptr]), 4);
	    	ptr += 4;
			if( _comment != NULL )
			{
				delete[] _comment;
			}
			_comment = new unsigned char[value];
			memcpy(_comment, &(data[ptr]), value);
			printf("Comment: %s", _comment);
			ptr += value;
		}
		else if( memcmp( charData, "ICOP", 4 ) == 0 )
		{
			printf("ICOP section found.");
    		memcpy(charData, &(data[ptr]), 4);
	    	ptr += 4;
			if( _copyright != NULL )
			{
				delete[] _copyright;
			}
			_copyright = new unsigned char[value];
			memcpy(_copyright, &(data[ptr]), value);
			printf("Copyright: %s", _copyright);
			ptr += value;
		}
		else if( memcmp( charData, "irom", 4 ) == 0 )
		{
			printf("irom section found.");
    		memcpy(charData, &(data[ptr]), 4);
	    	ptr += 4;
			if( _rom != NULL )
			{
				delete[] _rom;
			}
			_rom = new unsigned char[value];
			memcpy(_rom, &(data[ptr]), value);
			printf("ROM: %s", _rom);
			ptr += value;
		}
		else if( memcmp( charData, "iver", 4 ) == 0 )
		{
			printf("iver section found.");
    		memcpy(charData, &(data[ptr]), 4);
		    ptr += 4;
			printf("Section is %d bytes", value);
			memcpy(&_romVersionInfo, &(data[ptr]), 4);
			ptr += 4;
			printf("Major version: %d, Minor version: %d", _romVersionInfo.wMajor, _romVersionInfo.wMinor);
		}
		else
		{
			printf("Unknown section found: %c%c%c%c", charData[0], charData[1], charData[2], charData[3]);
		}
	}
	return true;
}

// Loads all of the RAM-based sound data associated with the SoundFont compatible bank.
bool SoundFont::ProcessSdtaListChunk(unsigned char* data, unsigned int size)
{
	unsigned int ptr = 0;
	unsigned long value;
	unsigned char* charData = (unsigned char*)&value;
	memcpy(charData, &(data[ptr]), 4);
	ptr += 4;
	if( memcmp(charData, "sdta", 4) != 0 )
	{
		printf("Invalid sdta chunk.");
		return 0;
	}
	while( ptr < size )
	{
		memcpy(charData, &(data[ptr]), 4);
		ptr += 4;
		if( memcmp( charData, "smpl", 4 ) == 0 )
		{
			printf("smpl section found.");
    		memcpy(charData, &(data[ptr]), 4);
		    ptr += 4;
			printf("Section is %d bytes", value);
			if( _sampleData != NULL )
			{
				delete[] _sampleData;
			}
			_sampleData = new unsigned char[value];
			memcpy(_sampleData, &(data[ptr]), value);
			ptr += value;
			printf("Sample loaded with size %d bytes", value);
			_sampleDataLength = value;
		}
		else
		{
			printf("Unknown section found: %c%c%c%c", charData[0], charData[1], charData[2], charData[3]);
		}
	}
	return true;
}


bool SoundFont::ProcessPdtaListChunk(unsigned char* data, unsigned int size)
{
	unsigned int ptr = 0;
	unsigned long value;
	unsigned char* charData = (unsigned char*)&value;
	memcpy(charData, &(data[ptr]), 4);
	ptr += 4;
	if( memcmp(charData, "pdta", 4) != 0 )
	{
		printf("Invalid pdta chunk.");
		return 0;
	}
	while( ptr < size )
	{
		memcpy(charData, &(data[ptr]), 4);
		ptr += 4;
		if( memcmp( charData, "phdr", 4 ) == 0 )
		{
			printf("phdr section found.");
    		memcpy(charData, &(data[ptr]), 4);
		    ptr += 4;
			printf("Section is %d bytes", value);
			unsigned char* theData = new unsigned char[value];
			memcpy(theData, &(data[ptr]), value);
			for( unsigned int i = 0; i < value/38; i++ )
			{
				sfPresetHeader* presetHdr = new sfPresetHeader();
				memcpy( presetHdr, &(data[ptr]), 38 );
				_presets.push_back(presetHdr);
				ptr += 38;
			}
			delete[] theData;
		}
		else if( memcmp( charData, "pbag", 4 ) == 0 )
		{
			printf("pbag section found.");
    		memcpy(charData, &(data[ptr]), 4);
		    ptr += 4;
			printf("Section is %d bytes", value);
			unsigned char* theData = new unsigned char[value];
			memcpy(theData, &(data[ptr]), value);
			for( unsigned int i = 0; i < value/4; i++ )
			{
				sfPresetBag* presetBag = new sfPresetBag();
				memcpy( presetBag, &(data[ptr]), 4 );
				_presetBags.push_back(presetBag);
				ptr += 4;
			}
			delete[] theData;
		}
		else if( memcmp( charData, "pmod", 4 ) == 0 )
		{
			printf("pmod section found.");
    		memcpy(charData, &(data[ptr]), 4);
		    ptr += 4;
			printf("Section is %d bytes", value);
			unsigned char* theData = new unsigned char[value];
			memcpy(theData, &(data[ptr]), value);
			for( unsigned int i = 0; i < value/10; i++ )
			{
				sfModList* modList = new sfModList();
				memcpy( modList, &(data[ptr]), 10 );
				_presetModulators.push_back(modList);
				ptr += 10;
			}
			delete[] theData;
		}
		else if( memcmp( charData, "pgen", 4 ) == 0 )
		{
			printf("pgen section found.");
    		memcpy(charData, &(data[ptr]), 4);
		    ptr += 4;
			printf("Section is %d bytes", value);
			unsigned char* theData = new unsigned char[value];
			memcpy(theData, &(data[ptr]), value);
			for( unsigned int i = 0; i < value/4; i++ )
			{
				sfGenList* genList = new sfGenList();
				memcpy( genList, &(data[ptr]), 4 );
				_presetGenerators.push_back(genList);
				ptr += 4;
			}
			delete[] theData;
		}
		else if( memcmp( charData, "inst", 4 ) == 0 )
		{
			printf("inst section found.");
    		memcpy(charData, &(data[ptr]), 4);
		    ptr += 4;
			printf("Section is %d bytes", value);
			unsigned char* theData = new unsigned char[value];
			memcpy(theData, &(data[ptr]), value);
			for( unsigned int i = 0; i < value/22; i++ )
			{
				sfInst* inst = new sfInst();
				memcpy( inst, &(data[ptr]), 22 );
				_instruments.push_back(inst);
				ptr += 22;
			}
			delete[] theData;
		}
		else if( memcmp( charData, "ibag", 4 ) == 0 )
		{
			printf("ibag section found.");
    		memcpy(charData, &(data[ptr]), 4);
		    ptr += 4;
			printf("Section is %d bytes", value);
			unsigned char* theData = new unsigned char[value];
			memcpy(theData, &(data[ptr]), value);
			for( unsigned int i = 0; i < value/4; i++ )
			{
				sfInstBag* instBag = new sfInstBag();
				memcpy( instBag, &(data[ptr]), 4 );
				_instrumentBags.push_back(instBag);
				ptr += 4;
			}
			delete[] theData;
		}
		else if( memcmp( charData, "imod", 4 ) == 0 )
		{
			printf("imod section found.");
    		memcpy(charData, &(data[ptr]), 4);
		    ptr += 4;
			printf("Section is %d bytes", value);
			unsigned char* theData = new unsigned char[value];
			memcpy(theData, &(data[ptr]), value);
			for( unsigned int i = 0; i < value/10; i++ )
			{
				sfModList* modList = new sfModList();
				memcpy( modList, &(data[ptr]), 10 );
				_instrumentModulators.push_back(modList);
				ptr += 10;
			}
			delete[] theData;
		}
		else if( memcmp( charData, "igen", 4 ) == 0 )
		{
			printf("igen section found.");
    		memcpy(charData, &(data[ptr]), 4);
		    ptr += 4;
			printf("Section is %d bytes", value);
			unsigned char* theData = new unsigned char[value];
			memcpy(theData, &(data[ptr]), value);
			for( unsigned int i = 0; i < value/4; i++ )
			{
				sfInstGenList* genList = new sfInstGenList();
				memcpy( genList, &(data[ptr]), 4 );
				_instrumentGenerators.push_back(genList);
				ptr += 4;
			}
			delete[] theData;
		}
		else if( memcmp( charData, "shdr", 4 ) == 0 )
		{
			printf("shdr section found.");
    		memcpy(charData, &(data[ptr]), 4);
		    ptr += 4;
			printf("Section is %d bytes", value);
			unsigned char* theData = new unsigned char[value];
			memcpy(theData, &(data[ptr]), value);
			for( unsigned int i = 0; i < value/46; i++ )
			{
				sfSample* sample = new sfSample();
				memcpy( sample, &(data[ptr]), 46 );
				_samples.push_back(sample);
				ptr += 46;
			}
			delete[] theData;
		}
		else
		{
			printf("Unknown section found: %c%c%c%c", charData[0], charData[1], charData[2], charData[3]);
		}
	}
	return true;
}