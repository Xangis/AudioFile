#ifndef _SOUNDFONT_H_
#define _SOUNDFONT_H_

#include <list>

// Implementation note:
//
// We're using unsigned short in place of WORD.
// We're using unsigned long in place of DWORD.
// We're using unsigned char in place of BYTE.
//
// This is to avoid the Windows-ism of those types. This is intended to be multiplatform code.

// RIFF and LIST chunks may contain nested chunks called sub-chunks as their data.

//
// Typedefs and enums
//
typedef struct
{
	unsigned char byLo;
	unsigned char byHi;
} rangesType;

typedef union
{
	rangesType ranges;
	short shAmount;
	unsigned short wAmount;
} genAmountType;

typedef enum
{
	monoSample = 1,
	rightSample = 2,
	leftSample = 4,
	linkedSample = 8,
	RomMonoSample = 0x8001,
	RomRightSample = 0x8002,
	RomLeftSample = 0x8004,
	RomLinkedSample = 0x8008
} SFSampleLink;

// I have not found definitive documentation for these, but declared struct
// size makesit look like they are each shorts.
typedef unsigned short SFModulator;
typedef unsigned short SFGenerator;
typedef unsigned short SFTransform;

//
// Data structs.
//
struct sfChunk
{
	unsigned long ckID; // unsigned char[4]
	unsigned long ckSize;
	unsigned char* ckData;
};

struct sfVersionTag
{
	unsigned short wMajor;
	unsigned short wMinor;
};

struct sfPresetHeader
{
	char achPresetName[20];
	unsigned short wPreset; // MIDI preset number.
	unsigned short wBank; // MIDI bank number.
	unsigned short wPresetBagNdx; // Index to the preset's zone list in the PBAG sub-chunk.
	unsigned long dwLibrary; // Unused, for future implementation.
	unsigned long dwGenre; // Unused, for future implementation.
	unsigned long dwMorphology; // Unused, for future implementation.
};

struct sfPresetBag
{
	unsigned short wGenNdx;
	unsigned short wModNdx;
};

// "In SoundFont 2.00, no modulators have yet been defined, and the IMOD sub-chunk
// will always consist of ten zero-valued bytes.
struct sfModList
{
	SFModulator sfModSrcOper;
	SFGenerator sfModDestOper;
	short modAmount;
	SFModulator sfModAmtSrcOper;
	SFTransform sfModTransOper;
};

struct sfGenList
{
	SFGenerator sfGenOper;
	genAmountType genAmount;
};

struct sfInst
{
	char achInstName[20];
	unsigned short wInstBagNdx;
};

struct sfInstBag
{
	unsigned short wInstGenNdx;
	unsigned short wInstModNdx;
};

struct sfInstModList
{
	SFModulator sfModSrcOper;
	SFGenerator sfModDestOper;
	short modAmount;
	SFModulator sfModAmtSrcOper;
	SFTransform sfModTransOper;
};

struct sfInstGenList
{
	SFGenerator sfGenOper;
	genAmountType genAmount;
};

struct sfSample
{
	char achSampleName[20];
	unsigned long dwStart;
	unsigned long dwEnd;
	unsigned long dwStartloop;
	unsigned long dwEndloop;
	unsigned long dwSampleRate;
	unsigned char byOriginalKey;
	char chCorrection;
	unsigned short wSampleLink;
	SFSampleLink sfSampleType;
};

class SoundFont
{
public:
	SoundFont();
	~SoundFont();
	bool Load(const char* filename);
	unsigned char* GetName();
private:
	bool ProcessInfoListChunk(unsigned char* data, unsigned int size);
	bool ProcessSdtaListChunk(unsigned char* data, unsigned int size);
	bool ProcessPdtaListChunk(unsigned char* data, unsigned int size);
	bool _loaded;
	unsigned char* _name;
	unsigned char* _engine;
	unsigned char* _product;
	unsigned char* _engineer;
	unsigned char* _software;
	unsigned char* _date;
	unsigned char* _comment;
	unsigned char* _copyright;
	unsigned char* _sampleData;
	unsigned char* _rom;
	unsigned int _sampleDataLength;
	sfVersionTag _versionInfo;
	sfVersionTag _romVersionInfo;
	std::list<sfSample *> _samples;
	std::list<sfPresetHeader *> _presets;
	std::list<sfPresetBag *> _presetBags;
	std::list<sfModList *> _presetModulators;
	std::list<sfGenList *> _presetGenerators;
	std::list<sfInst *> _instruments;
	std::list<sfInstBag *> _instrumentBags;
	std::list<sfModList *> _instrumentModulators;
	std::list<sfInstGenList *> _instrumentGenerators;
};

#endif