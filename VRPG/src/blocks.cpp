#include "blocks.h"
#include <stdio.h>
#include "world.h"


BlockDef * BLOCK_DEFS[256];
bool BLOCK_TYPE_CAN_PASS[256];
bool BLOCK_TYPE_OPAQUE[256];
bool BLOCK_TYPE_VISIBLE[256];
bool BLOCK_TERRAIN_SMOOTHING[256];

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
	BLOCK_TERRAIN_SMOOTHING[def->id] = def->terrainSmoothing();
}

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


static float face_vertices_north[VERTEX_COMPONENTS * 4] =
{
	-0.5, 0.5, -0.5,	0.0, 0.0, -1.0,		1.0, 1.0, 1.0,		0.0, 0.0,
	0.5, 0.5, -0.5,		0.0, 0.0, -1.0,		1.0, 1.0, 1.0,		1.0, 0.0,
	-0.5, -0.5, -0.5,	0.0, 0.0, -1.0,		1.0, 1.0, 1.0,		0.0, 1.0,
	0.5, -0.5, -0.5,	0.0, 0.0, -1.0,		1.0, 1.0, 1.0,		1.0, 1.0,
};

static float face_vertices_south[VERTEX_COMPONENTS * 4] =
{
	-0.5, -0.5, 0.5,	0.0, 0.0, 1.0,		1.0, 1.0, 1.0,		0.0, 0.0,
	0.5, -0.5, 0.5,		0.0, 0.0, 1.0,		1.0, 1.0, 1.0,		1.0, 0.0,
	-0.5, 0.5, 0.5,		0.0, 0.0, 1.0,		1.0, 1.0, 1.0,		0.0, 1.0,
	0.5, 0.5, 0.5,		0.0, 0.0, 1.0,		1.0, 1.0, 1.0,		1.0, 1.0,
};

static float face_vertices_west[VERTEX_COMPONENTS * 4] =
{
	-0.5, -0.5, -0.5,	-1.0, 0.0, 0.0,		1.0, 1.0, 1.0,		0.0, 0.0,
	-0.5, -0.5, 0.5,	-1.0, 0.0, 0.0,		1.0, 1.0, 1.0,		1.0, 0.0,
	-0.5, 0.5, -0.5,	-1.0, 0.0, 0.0,		1.0, 1.0, 1.0,		0.0, 1.0,
	-0.5, 0.5, 0.5,		-1.0, 0.0, 0.0,		1.0, 1.0, 1.0,		1.0, 1.0
};

static float face_vertices_east[VERTEX_COMPONENTS * 4] =
{
	0.5, -0.5, 0.5,		1.0, 0.0, 0.0,		1.0, 1.0, 1.0,		0.0, 0.0,
	0.5, -0.5, -0.5,	1.0, 0.0, 0.0,		1.0, 1.0, 1.0,		1.0, 0.0,
	0.5, 0.5, 0.5,		1.0, 0.0, 0.0,		1.0, 1.0, 1.0,		0.0, 1.0,
	0.5, 0.5, -0.5,		1.0, 0.0, 0.0,		1.0, 1.0, 1.0,		1.0, 1.0,
};

static float face_vertices_up[VERTEX_COMPONENTS * 4] =
{
	-0.5, 0.5, 0.5,		0.0, 1.0, 0.0,		1.0, 1.0, 1.0,		0.0, 0.0,
	0.5, 0.5, 0.5,		0.0, 1.0, 0.0,		1.0, 1.0, 1.0,		1.0, 0.0,
	-0.5, 0.5, -0.5,	0.0, 1.0, 0.0,		1.0, 1.0, 1.0,		0.0, 1.0,
	0.5, 0.5, -0.5,		0.0, 1.0, 0.0,		1.0, 1.0, 1.0,		1.0, 1.0,
};

static float face_vertices_down[VERTEX_COMPONENTS * 4] =
{
	-0.5, -0.5, -0.5,	0.0, -1.0, 0.0,		1.0, 1.0, 1.0,		0.0, 0.0,
	0.5, -0.5, -0.5,	0.0, -1.0, 0.0,		1.0, 1.0, 1.0,		1.0, 0.0,
	-0.5, -0.5, 0.5,	0.0, -1.0, 0.0,		1.0, 1.0, 1.0,		0.0, 1.0,
	0.5, -0.5, 0.5,		0.0, -1.0, 0.0,		1.0, 1.0, 1.0,		1.0, 1.0,
};

static int face_indexes[6] =
{
    0, 1, 2, 2, 1, 3
};

static int face_indexes_back[6] =
{
    0, 2, 1, 2, 3, 1
};

static void fillFaceMesh(float * data, float * src, float x0, float y0, float z0, int tileX, int tileY) {
	for (int i = 0; i < 4; i++) {
		float * srcvertex = src + i * VERTEX_COMPONENTS;
		float * dstvertex = data + i * VERTEX_COMPONENTS;
		for (int j = 0; j < 11; j++) {
			float v = srcvertex[j];
			switch (j) {
			case 0: // x
				v += x0;
				break;
			case 1: // y
				v += y0;
				break;
			case 2: // z
				v += z0;
				break;
			case 9: // tx.u
				v = ((tileX + v * BLOCK_SPRITE_SIZE)) / (float)BLOCK_TEXTURE_DX;
				break;
			case 10: // tx.v
				v = (BLOCK_TEXTURE_DY - (tileY + v * BLOCK_SPRITE_SIZE)) / (float)BLOCK_TEXTURE_DY;
				break;
			}
			dstvertex[j] = v;
		}
	}
}

static void createFaceMesh(float * data, Dir face, float x0, float y0, float z0, int tileIndex) {

	int tileX = (tileIndex % BLOCK_TEXTURE_SPRITES_PER_LINE) * BLOCK_SPRITE_STEP + BLOCK_SPRITE_OFFSET;
	int tileY = (tileIndex / BLOCK_TEXTURE_SPRITES_PER_LINE) * BLOCK_SPRITE_STEP + BLOCK_SPRITE_OFFSET;
	// data is 11 comp * 4 vert floats
	switch (face) {
	default:
	case NORTH:
		fillFaceMesh(data, face_vertices_north, x0, y0, z0, tileX, tileY);
		break;
	case SOUTH:
		fillFaceMesh(data, face_vertices_south, x0, y0, z0, tileX, tileY);
		break;
	case WEST:
		fillFaceMesh(data, face_vertices_west, x0, y0, z0, tileX, tileY);
		break;
	case EAST:
		fillFaceMesh(data, face_vertices_east, x0, y0, z0, tileX, tileY);
		break;
	case UP:
		fillFaceMesh(data, face_vertices_up, x0, y0, z0, tileX, tileY);
		break;
	case DOWN:
		fillFaceMesh(data, face_vertices_down, x0, y0, z0, tileX, tileY);
		break;
	}
}


void BlockDef::createFace(World * world, Position & camPosition, Vector3d pos, Dir face, FloatArray & vertices, IntArray & indexes) {
	int v0 = vertices.length() / VERTEX_COMPONENTS;
	float * vptr = vertices.append(0.0f, VERTEX_COMPONENTS * 4);
	int * iptr = indexes.append(0, 6);
	createFaceMesh(vptr, face, pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f, txIndex);
	for (int i = 0; i < 6; i++)
		iptr[i] = v0 + face_indexes[i];
	if (HIGHLIGHT_GRID && ((pos.x & 7) == 0 || (pos.z & 7) == 0)) {
		for (int i = 0; i < 4; i++) {
			vptr[11 * i + 6 + 0] = 1.4f;
			vptr[11 * i + 6 + 1] = 1.4f;
			vptr[11 * i + 6 + 2] = 1.4f;
		}
	}
}

void BlockDef::createFaces(World * world, Position & camPosition, Vector3d pos, int visibleFaces, FloatArray & vertices, IntArray & indexes) {
	for (int i = 0; i < 6; i++)
		if (visibleFaces & (1 << i))
			createFace(world, camPosition, pos, (Dir)i, vertices, indexes);
}

static VolumeData nearVolume(1);

class TerrainBlock : public BlockDef {
public:
	TerrainBlock(cell_t blockId, const char * blockName, int tx) : BlockDef(blockId, blockName, OPAQUE, tx) {

	}
	virtual bool terrainSmoothing() {
		return true;
	}
	virtual void createFaces(World * world, Position & camPosition, Vector3d pos, int visibleFaces, FloatArray & vertices, IntArray & indexes) {
		world->getCellsNear(pos, nearVolume);
		bool emptyAbove = BLOCK_TYPE_CAN_PASS[nearVolume.get(Vector3d(0, 1, 0))];
		bool sameBlockBelow = nearVolume.get(Vector3d(0, -1, 0)) == id;
		bool sameBlockNorth = nearVolume.get(Vector3d(0, 0, -1)) == id;
		bool sameBlockSouth = nearVolume.get(Vector3d(0, 0, 1)) == id;
		bool sameBlockWest = nearVolume.get(Vector3d(-1, 0, 0)) == id;
		bool sameBlockEast = nearVolume.get(Vector3d(1, 0, 0)) == id;
		bool emptyBlockNorth = BLOCK_TYPE_CAN_PASS[nearVolume.get(Vector3d(0, 0, -1))];
		bool emptyBlockSouth = BLOCK_TYPE_CAN_PASS[nearVolume.get(Vector3d(0, 0, 1))];
		bool emptyBlockWest = BLOCK_TYPE_CAN_PASS[nearVolume.get(Vector3d(-1, 0, 0))];
		bool emptyBlockEast = BLOCK_TYPE_CAN_PASS[nearVolume.get(Vector3d(1, 0, 0))];
		BlockDef::createFaces(world, camPosition, pos, visibleFaces, vertices, indexes);
	}
};


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

		registerBlockType(new TerrainBlock(100, "terrain_bedrock", 2));
		registerBlockType(new TerrainBlock(101, "terrain_clay", 3));
		registerBlockType(new TerrainBlock(102, "terrain_cobblestone", 4));
		registerBlockType(new TerrainBlock(103, "terrain_gravel", 5));
		registerBlockType(new TerrainBlock(104, "terrain_red_sand", 6));
		registerBlockType(new TerrainBlock(105, "terrain_sand", 7));
	}
};


// for static initialization
BlockTypeInitializer blockTypeInitializer;

