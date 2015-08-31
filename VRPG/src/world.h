#ifndef WORLD_H_INCLUDED
#define WORLD_H_INCLUDED

#include <stdlib.h>
#include "worldtypes.h"

// Layer is 16x16 (CHUNK_DX_SHIFT x CHUNK_DX_SHIFT) cells
#define CHUNK_DX_SHIFT 4
#define CHUNK_DX (1<<CHUNK_DX_SHIFT)
#define CHUNK_DX_MASK (CHUNK_DX - 1)

#define CHUNK_DY_SHIFT 8
#define CHUNK_DY (1<<CHUNK_DY_SHIFT)
#define CHUNK_DY_MASK (CHUNK_DY - 1)

const cell_t NO_CELL = 0;
const cell_t END_OF_WORLD = 255;

// Layer is 256x16x16 CHUNK_DY layers = CHUNK_DY * (CHUNK_DX_SHIFT x CHUNK_DX_SHIFT) cells
struct ChunkLayer {
private:
	cell_t cells[CHUNK_DX * CHUNK_DX];
public:
	ChunkLayer() {
		for (int x = 0; x < CHUNK_DX; x++)
			for (int z = 0; z < CHUNK_DX; z++)
				cells[(z << CHUNK_DX_SHIFT) + x] = NO_CELL;
	}
	inline cell_t get(int x, int z) {
		//if (!this)
		//	return NO_CELL;
		return cells[(z << CHUNK_DX_SHIFT) + x];
	}
	inline void set(int x, int z, cell_t cell) {
		cells[(z << CHUNK_DX_SHIFT) + x] = cell;
	}
};

struct Chunk {
private:
	ChunkLayer * layers[CHUNK_DY];
public:
	Chunk() {
		for (int i = 0; i < CHUNK_DY; i++)
			layers[i] = NULL;
	}
	~Chunk() {
		for (int i = 0; i < CHUNK_DY; i++)
			if (layers[i])
				delete layers[i];
	}
	inline cell_t get(int x, int y, int z) {
		//if (!this)
		//	return NO_CELL;
		ChunkLayer * layer = layers[y & CHUNK_DY_MASK];
		if (!layer)
			return NO_CELL;
		return layer->get(x & CHUNK_DX_MASK, z & CHUNK_DY_MASK);
	}
	inline void set(int x, int y, int z, cell_t cell) {
		ChunkLayer * layer = layers[y & CHUNK_DY_MASK];
		if (!layer) {
			layer = new ChunkLayer();
			layers[y & CHUNK_DY_MASK] = layer;
		}
		layer->set(x & CHUNK_DX_MASK, z & CHUNK_DY_MASK, cell);
	}
	static void dispose(Chunk * p) {
		delete p;
	}
};

typedef InfiniteArray<Chunk*, (Chunk*)NULL, Chunk::dispose> ChunkStripe;
void disposeChunkStripe(ChunkStripe * p);

typedef InfiniteArray<ChunkStripe*, (ChunkStripe*)NULL, disposeChunkStripe> ChunkStripes;

struct ChunkMatrix {
	int minx;
	int maxx;
	int minz;
	int maxz;
private:
	ChunkStripes stripes;
public:
	ChunkMatrix() : minx(0), maxx(0), minz(0), maxz(0) {

	}
	~ChunkMatrix() {

	}
	int minX() { return minx; }
	int maxX() { return maxx; }
	int minZ() { return minz; }
	int maxZ() { return maxz; }
	Chunk * get(int x, int z) {
		ChunkStripe * p = stripes.get(z);
		if (!p)
			return NULL;
		return p->get(x);
	}
	void set(int x, int z, Chunk * chunk) {
		ChunkStripe * p = stripes.get(z);
		if (!p) {
			p = new ChunkStripe();
			stripes.set(z, p);
			if (minz > z)
				minz = z;
			if (maxz < z + 1)
				maxz = z + 1;
		}
		p->set(x, chunk);
		if (minx > x)
			minx = x;
		if (maxx < x + 1)
			maxx = x + 1;
	}
};

class World;
class CellVisitor {
public:
	virtual ~CellVisitor() {}
	virtual void newDirection(Position & camPosition) { }
	virtual void visitFace(World * world, Position & camPosition, Vector3d pos, cell_t cell, Dir face) { }
	virtual void visit(World * world, Position & camPosition, Vector3d pos) { }
};

/// Voxel World
class World {
private:
	ChunkMatrix chunks;
	Position camPosition;
	int maxVisibleRange;
	int lastChunkX;
	int lastChunkZ;
	Chunk * lastChunk;
public:
	World() : maxVisibleRange(32), lastChunkX(1000000), lastChunkZ(1000000), lastChunk(NULL) {
	}
	~World() {

	}
	void visitVisibleCells(Position & position, CellVisitor * visitor, bool visitThisPosition = true);
	void visitVisibleCellsAllDirections(Position & position, CellVisitor * visitor);
	Position & getCamPosition() { return camPosition; }
	cell_t getCell(Vector3d v) {
		return getCell(v.x, v.y, v.z);
	}
	cell_t getCell(int x, int y, int z);
	void setCell(int x, int y, int z, cell_t value);
};

#define UNIT_TESTS 1
void runWorldUnitTests();

#endif// WORLD_H_INCLUDED
