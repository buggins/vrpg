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

enum BlockVisibility {
	INVISIBLE,
	OPAQUE, // completely opaque (cells covered by this block are invisible)
	OPAQUE_SEPARATE_TX,
	HALF_OPAQUE, // partially paque, cells covered by this block can be visible, render as normal block
	HALF_OPAQUE_SEPARATE_TX,
	HALF_TRANSPARENT, // should be rendered last (semi transparent texture)
};

class BlockDef {
public:
	cell_t id;
	const char * name;
	BlockVisibility visibility;
	int txIndex;
	BlockDef() : id(0), name(""), visibility(INVISIBLE), txIndex(0) {
	}
	BlockDef(cell_t blockId, const char * blockName, BlockVisibility v, int tx) : id(blockId), name(blockName), visibility(v), txIndex(tx) {
	}
	virtual ~BlockDef() {}
	// blocks behind this block can be visible
	virtual bool canPass() { 
		return visibility == INVISIBLE 
			|| visibility == HALF_OPAQUE 
			|| visibility == HALF_OPAQUE_SEPARATE_TX 
			|| visibility == HALF_TRANSPARENT; 
	}
	// block is fully opaque (all blocks behind are invisible)
	virtual bool isOpaque() {
		return visibility == OPAQUE
			|| visibility == OPAQUE_SEPARATE_TX;
	}
};


// block type definitions
extern BlockDef * BLOCK_DEFS[256];
// faster check for block->canPass()
extern bool BLOCK_TYPE_CAN_PASS[256];
// faster check for block->isOpaque()
extern bool BLOCK_TYPE_OPAQUE[256];

/// registers new block type
void registerBlockType(BlockDef * def);
/// init block types array
void initBlockTypes();


#endif // BLOCKS_H_INCLUDED
