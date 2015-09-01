#include "blocks.h"

BlockDef BLOCK_DEFS[256] = {
	{ 0, "empty",		INVISIBLE, 0 },
	{ 1, "gray_brick",  OPAQUE, 0 },
	{ 2, "brick",		OPAQUE, 1 },
	{ 3, "bedrock",		OPAQUE, 2 },
	{ 4, "clay",		OPAQUE, 3 },
	{ 5, "cobblestone",	OPAQUE, 4 },
	{ 6, "gravel",		OPAQUE, 5 },
	{ 7, "red_sand",	OPAQUE, 6 },
	{ 8, "sand",		OPAQUE, 7 },
};
