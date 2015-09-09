#ifndef WORLD_H_INCLUDED
#define WORLD_H_INCLUDED

#include <stdlib.h>
#include "worldtypes.h"

const int MAX_VIEW_DISTANCE_BITS = 6;
const int MAX_VIEW_DISTANCE = (1 << MAX_VIEW_DISTANCE_BITS);

// Layer is 16x16 (CHUNK_DX_SHIFT x CHUNK_DX_SHIFT) cells
#define CHUNK_DX_SHIFT 4
#define CHUNK_DX (1<<CHUNK_DX_SHIFT)
#define CHUNK_DX_MASK (CHUNK_DX - 1)

#define CHUNK_DY_SHIFT 8
#define CHUNK_DY (1<<CHUNK_DY_SHIFT)
#define CHUNK_DY_MASK (CHUNK_DY - 1)

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
	inline cell_t* ptr(int x, int z) {
		//if (!this)
		//	return NO_CELL;
		return cells + (z << CHUNK_DX_SHIFT) + x;
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
	int bottomLayer;
	int topLayer;
public:
	Chunk() : bottomLayer(-1), topLayer(-1) {
		for (int i = 0; i < CHUNK_DY; i++)
			layers[i] = NULL;
	}
	~Chunk() {
		for (int i = 0; i < CHUNK_DY; i++)
			if (layers[i])
				delete layers[i];
	}
	int getMinLayer() { return bottomLayer; }
	int getMaxLayer() { return topLayer; }
	void updateMinMaxLayer(int & minLayer, int & maxLayer) {
		if (minLayer == -1 || minLayer > bottomLayer)
			minLayer = bottomLayer;
		if (maxLayer == -1 || maxLayer < topLayer)
			maxLayer = topLayer;
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
		int layerIndex = y & CHUNK_DY_MASK;
		ChunkLayer * layer = layers[layerIndex];
		if (!layer) {
			layer = new ChunkLayer();
			layers[layerIndex] = layer;
			if (topLayer == -1 || topLayer < layerIndex)
				topLayer = layerIndex;
			if (bottomLayer == -1 || bottomLayer > layerIndex)
				bottomLayer = layerIndex;
		}
		layer->set(x & CHUNK_DX_MASK, z & CHUNK_DY_MASK, cell);
	}
	static void dispose(Chunk * p) {
		delete p;
	}

	/// srcpos coords x, z are in chunk bounds
	void getCells(Vector3d srcpos, Vector3d dstpos, Vector3d size, VolumeData & buf);
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
	virtual void visit(World * world, Position & camPosition, Vector3d pos, cell_t cell, int visibleFaces) { }
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
	VolumeData volumeSnapshot;
	Vector3d volumePos;
	bool volumeSnapshotInvalid;
public:
	World() : maxVisibleRange(MAX_VIEW_DISTANCE), lastChunkX(1000000), lastChunkZ(1000000), lastChunk(NULL), volumeSnapshot(MAX_VIEW_DISTANCE_BITS), volumeSnapshotInvalid(true) {
	}
	~World() {

	}
	void updateVolumeSnapshot();
	void getCellsNear(Vector3d v, VolumeData & buf);
	void visitVisibleCellsAllDirectionsFast(Position & position, CellVisitor * visitor);
	Position & getCamPosition() { return camPosition; }
	cell_t getCell(Vector3d v) {
		return getCell(v.x, v.y, v.z);
	}
	cell_t getCell(int x, int y, int z);
	bool isOpaque(Vector3d v);
	void setCell(int x, int y, int z, cell_t value);
};

#define UNIT_TESTS 1
void runWorldUnitTests();

#endif// WORLD_H_INCLUDED
