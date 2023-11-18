#ifndef TFMXSONG_H
#define TFMXSONG_H

#include "tfmx.h"

struct Header
{
	char magic[10];
	char pad[6];
	char text[6][40];
	unsigned short start[32],end[32],tempo[32];
	short mute[8];
	unsigned int trackstart,pattstart,macrostart;
	char pad2[36];
};

#endif // TFMXSONG_H