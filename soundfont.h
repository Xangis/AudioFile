#ifndef _SOUNDFONT_H_
#define _SOUNDFONT_H_

struct sfVersionTag
{
	unsigned short wMajor;
	unsigned short wMinor;
};

struct sfPresetHeader
{
	char achPresetName[20];
	unsigned short wPreset;
	unsigned short wBank;
	unsigned short wPresetBagNdx;
	unsigned long dwLibrary;
	unsigned long dwGenre;
	unsigned long dwMorphology;
};

struct sfPresetBag
{
	unsigned short wGenNdx;
	unsigned short wModNdx;
};

struct sfModList
{
	//
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


#endif