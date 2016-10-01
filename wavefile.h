#ifndef _WAVEFILE_H_
#define _WAVEFILE_H_

#include "sndfile.h"
#ifndef linux
// Windows.h is required by XAudio2. So is audiodefs.2 (includes filter parameter struct).
// Required for wx compatibility if we include windows.h. Which is bullshit. Now a wave file loader depends on XAudio, Windows, wxWidgets, and all that shit.
//#include "wx/wx.h"
//#ifndef USE_QT
//#include "Windows.h"
//#include "wx/msw/winundef.h"
//#endif
#ifdef USE_XAUDIO
#include <audiodefs.h>
#include <xaudio2.h>
#endif
#endif

/**
* Represents a wave file.  Used for loading wave data from disk.
*/
class WaveFile
{
public:
    bool Load( const char* filename );
	bool Load( const wchar_t* filename );
    bool Save( const char* filename );
    WaveFile();
	~WaveFile();
    unsigned int GetSampleRate() { return _sampleRate; }
	unsigned int GetBytesPerSample() { return _bitsPerSample / 8; }
    unsigned short GetNumChannels() { return _numChannels; }
    unsigned int GetNumSamples() { return _numSamples; }
    unsigned short GetBitsPerSample() { return _bitsPerSample; }
    unsigned short GetBlockAlign() { return _blockAlign; }
	unsigned int GetDataLength() { return _dataLength; }
    short* GetData();
    SF_INFO* GetSFInfo() { return &_sfinfo; }
    short* GetChunk( unsigned int start, unsigned int end );
	static WaveFile* Load(const char* filename, bool allowNull );
	static WaveFile* Load(const wchar_t* filename, bool allowNull );
	int GetLength();
#ifdef WIN32
#ifdef USE_XAUDIO
	WAVEFORMATEX* GetWaveFormatEx();
	XAUDIO2_BUFFER* GetXAudio2Buffer();
	bool ReplaceBuffer();
#endif
#endif
private:
    SF_INFO _sfinfo;
    SNDFILE* _soundFile;
	unsigned int _dataLength;
	unsigned int _sampleRate;
	unsigned int _numSamples;
	unsigned short _numChannels;
	unsigned short _bitsPerSample;
    unsigned short _blockAlign;
#ifdef WIN32
#ifdef USE_XAUDIO
	XAUDIO2_BUFFER* _xbuffer;
	// Called when replacing an existing sample.
#endif
#endif
};

#endif
