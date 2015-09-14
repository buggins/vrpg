#include "blocks.h"
#include <stdio.h>


BlockDef * BLOCK_DEFS[256];
bool BLOCK_TYPE_CAN_PASS[256];
bool BLOCK_TYPE_OPAQUE[256];
bool BLOCK_TYPE_VISIBLE[256];

/// registers new block type
void registerBlockType(BlockDef * def) {
	if (BLOCK_DEFS[def->id]) {
		if (BLOCK_DEFS[def->id] == def)
			return;
		delete BLOCK_DEFS[def->id];
	}
	BLOCK_DEFS[def->id] = def;
	// init property shortcuts
	BLOCK_TYPE_CAN_PASS[def->id] = def->canPass();
	BLOCK_TYPE_OPAQUE[def->id] = def->isOpaque();
	BLOCK_TYPE_VISIBLE[def->id] = def->isVisible();
}


struct BlockTypeInitializer {
	BlockTypeInitializer() {
		for (int i = 0; i < 256; i++) {
			BLOCK_DEFS[i] = NULL;
			BLOCK_TYPE_CAN_PASS[i] = true;
		}
		// empty cell
		registerBlockType(new BlockDef(0, "empty", INVISIBLE, 0));
		// standard block types
		registerBlockType(new BlockDef(1, "gray_brick", OPAQUE, 0));
		registerBlockType(new BlockDef(2, "brick", OPAQUE, 1));
		registerBlockType(new BlockDef(3, "bedrock", OPAQUE, 2));
		registerBlockType(new BlockDef(4, "clay", OPAQUE, 3));
		registerBlockType(new BlockDef(5, "cobblestone", OPAQUE, 4));
		registerBlockType(new BlockDef(6, "gravel", OPAQUE, 5));
		registerBlockType(new BlockDef(7, "red_sand", OPAQUE, 6));
		registerBlockType(new BlockDef(8, "sand", OPAQUE, 7));

		registerBlockType(new BlockDef(50, "box", HALF_OPAQUE, 50));
	}
};

// for static initialization
BlockTypeInitializer blockTypeInitializer;

void initBlockTypes() {
	// fill non-registered entries
	for (int i = 0; i < 256; i++) {
		if (!BLOCK_DEFS[i]) {
			char buf[32];
			sprintf(buf, "undef%d", i);
			registerBlockType(new BlockDef(i, strdup(buf), INVISIBLE, 0));
		}
	}
}
