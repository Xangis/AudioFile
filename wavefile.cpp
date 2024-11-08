#include "wavefile.h"
#include <iostream>
#include <string.h>

#ifdef linux
#define FILENAMELENGTH 260
#else
#define FILENAMELENGTH 4096
#endif

/**
* Default constructor.
*/
WaveFile::WaveFile()
{
    memset( &_sfinfo, 0, sizeof( _sfinfo ));
	_dataLength = 0;
	_sampleRate = 0;
	_numSamples = 0;
	_numChannels = 0;
    _blockAlign = 0;
	_bitsPerSample = 16;    
	_soundFile = NULL;
#ifdef WIN32
#ifdef USE_XAUDIO
	_xbuffer = NULL;
#endif
#endif
}

WaveFile::~WaveFile()
{
//#ifdef WIN32
//	if( _xbuffer != NULL )
//	{
//		delete _xbuffer;
//		_xbuffer = NULL;
//	}
//#endif
}

WaveFile* WaveFile::Load(const wchar_t* filename, bool allowNull )
{
#ifdef WIN32
	char fname[FILENAMELENGTH];
	memset(fname, 0, FILENAMELENGTH);
	size_t numConverted = 0;
	wcstombs_s(&numConverted, fname, FILENAMELENGTH, filename, FILENAMELENGTH-1);
	return WaveFile::Load(fname, allowNull);
#else
        return WaveFile::Load(filename, allowNull);
#endif
}

/**
* Static method that loads a wave file from disk and populates a new WaveFile with the data.
*/
WaveFile* WaveFile::Load(const char* filename, bool allowNull )
{
	WaveFile* waveFile = new WaveFile;
	if( waveFile->Load(filename))
	{
		return waveFile;
	}
	else if( allowNull )
	{
		return NULL;
	}
	else
	{
		return new WaveFile;
	}
}

bool WaveFile::Load( const wchar_t* filename )
{
#ifdef WIN32
	char fname[FILENAMELENGTH];
	memset(fname, 0, FILENAMELENGTH);
	size_t numConverted = 0;
	wcstombs_s(&numConverted, fname, FILENAMELENGTH, filename, FILENAMELENGTH-1);
	return Load(fname);
#else
        return Load(filename);
#endif
}

/**
* Loads a wave file from disk and populates this class with the data.
*/
bool WaveFile::Load( const char* filename )
{
	memset(&_sfinfo, 0, sizeof(SF_INFO));
	if( _soundFile != NULL )
	{
		sf_close(_soundFile);
	}
    _soundFile = sf_open( filename, SFM_READ, &_sfinfo );

    if( _soundFile == NULL )
    {
        printf("Could not open %s\n", filename);
        return false;
    }

    _numSamples = (unsigned int)_sfinfo.frames;
    _numChannels = (unsigned short)_sfinfo.channels;
    _sampleRate = _sfinfo.samplerate;

	// Mask off bit rate.
	int format = _sfinfo.format & 0xFFFF;

    if( format == SF_FORMAT_PCM_S8 )
    {
        _bitsPerSample = 8;
        _blockAlign = _numChannels;
        _dataLength = _numSamples * _blockAlign;
    }
    else if( format == SF_FORMAT_PCM_16 )
    {
        _bitsPerSample = 16;
        _blockAlign = _numChannels * 2;
        _dataLength = _numSamples * _blockAlign;
    }
    else if( format == SF_FORMAT_PCM_24 )
    {
        _bitsPerSample = 24;
        _blockAlign = _numChannels * 3;
        _dataLength = _numSamples * _blockAlign;
    }
    else if( format == SF_FORMAT_PCM_32 )
    {
        _bitsPerSample = 32;
        _blockAlign = _numChannels * 4;
        _dataLength = _numSamples * _blockAlign;
    }
    else if( format == SF_FORMAT_FLOAT )
    {
        _bitsPerSample = 16;
        _blockAlign = _numChannels * 2;
        _dataLength = _numSamples * _blockAlign;
    }
    else if( format == SF_FORMAT_DOUBLE )
    {
        _bitsPerSample = 32;
        _blockAlign = _numChannels * 4;
        _dataLength = _numSamples * _blockAlign;
    }
	else if( format == SF_FORMAT_FLAC )
	{
		_bitsPerSample = 16;
		_blockAlign = _numChannels * 2;
		_dataLength = _numSamples * _blockAlign;
	}
	else if( format == SF_FORMAT_PCM_U8 )
	{
		//// Unsigned 8-bit does not work correctly.  In order to support it
		//// we would have to convert to signed 8-bit.
		//_bitsPerSample = 8;
		//_blockAlign = _numChannels;
		//_dataLength = _numSamples * _blockAlign;
		return false;
	}
	else
	{
		return false;
	}
#ifdef WIN32
#ifdef USE_XAUDIO
	ReplaceBuffer();
#endif
#endif
    return true;
}

/**
 * Gets the length of the wave file in seconds.
 */
int WaveFile::GetLength()
{
    if( _sampleRate > 0 )
    {
        return _numSamples / _sampleRate;
    }
    return -1;
}

/**
 * Copies the sound data for this wave file into a new file.
 */
bool WaveFile::Save( const char *filename )
{
    SNDFILE* outfile = sf_open( filename, SFM_WRITE, &_sfinfo );
    if( outfile == NULL )
    {
        return false;
    }
	short* data = GetData();
    sf_write_short( outfile, data, (_numSamples * _numChannels));
	delete[] data;
    sf_close( outfile );

    return true;
}

/**
* This returns a new buffer containing data loaded from disk.  As such, changing or manipulating
* it does not have any effect on the original data.  The caller is responsible for destroying this
* data after they are done with it.  This should only be called for 16-bit files.
*/
short* WaveFile::GetData()
{
    sf_seek( _soundFile, 0, SEEK_SET );
    short* shortData = new short[_numSamples * _numChannels];
    sf_read_short( _soundFile, shortData, (_numSamples * _numChannels));
    return shortData;
}

/**
*  Gets a chunk of data at the specified start and end position in the file.
*
*  This function creates new data that must be destroyed by the caller.
*
*  The chunk of data is read as shorts, and the data length read is end-start.
*  This means a start of 400 and an end of 800 would read 400 bytes from 400-799.
*/
short* WaveFile::GetChunk( unsigned int start, unsigned int end )
{
	int dataLength = (end-start)*_numChannels;
    sf_seek( _soundFile, start, SEEK_SET );
    short* shortData = new short[dataLength];
    sf_read_short( _soundFile, shortData, dataLength);
    return shortData;
}

/**
* Retreives a WAVEFORMATEX structure based on the wave file data.
*/
#ifdef WIN32
#ifdef USE_XAUDIO2
WAVEFORMATEX* WaveFile::GetWaveFormatEx()
{
	WAVEFORMATEX* wfx = new WAVEFORMATEX;
	wfx->cbSize = (WORD)GetDataLength();
	wfx->nBlockAlign = GetBlockAlign();
	wfx->nChannels = GetNumChannels();
	wfx->nSamplesPerSec = GetSampleRate();
	wfx->wBitsPerSample = GetBitsPerSample();
	wfx->wFormatTag = 0x0001;
	wfx->nAvgBytesPerSec = wfx->nBlockAlign * wfx->nSamplesPerSec;
	return wfx;
}

/**
* Gets an XAUDIO2_BUFFER filled with this wave file's data.  Having this
* method here ties us to having XAudio2 if we use the AudioFile project,
* so we may want to put this in a separate library at some point.
*/
XAUDIO2_BUFFER* WaveFile::GetXAudio2Buffer()
{
	if( _xbuffer == NULL )
	{
		_xbuffer = new XAUDIO2_BUFFER;
		_xbuffer->pAudioData = (unsigned char *)GetData();
		_xbuffer->AudioBytes = GetDataLength();
		_xbuffer->Flags = XAUDIO2_END_OF_STREAM;
		_xbuffer->pContext = NULL;
		_xbuffer->LoopLength = 0;
		_xbuffer->LoopBegin = 0;
		_xbuffer->PlayLength = GetNumSamples();
		_xbuffer->PlayBegin = 0;
		_xbuffer->LoopCount = 0;
	}
	return _xbuffer;

}

/* 
Replaces the existing XAudio2 buffer with new data when a new sample is loaded. 
*/
bool WaveFile::ReplaceBuffer()
{
	if( _xbuffer != NULL )
	{
		delete[] _xbuffer->pAudioData;
		delete _xbuffer;
		_xbuffer = NULL;
	}
	XAUDIO2_BUFFER* buffer = GetXAudio2Buffer();
	if( buffer != NULL )
	{
		return true;
	}
	return false;
}

#endif // ifdef USE_XAUDIO2
#endif
