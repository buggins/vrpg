#ifndef BLOCKS_H_INCLUDED
#define BLOCKS_H_INCLUDED

#include "worldtypes.h"

#define BLOCK_TEXTURE_FILENAME "res/png/blocks.png"
#define BLOCK_TEXTURE_DX 1024
#define BLOCK_TEXTURE_DY 1024
#define BLOCK_SPRITE_SIZE 16
#define BLOCK_SPRITE_STEP 20
#define BLOCK_SPRITE_OFFSET 21
#define BLOCK_TEXTURE_SPRITES_PER_LINE 50

struct BlockDef {
	int id;
	char * name;
	bool opaque;
	int txIndex;
};

extern BlockDef BLOCK_DEFS[];

#endif // BLOCKS_H_INCLUDED
