#ifndef WORLD_H_INCLUDED
#define WORLD_H_INCLUDED

#include <stdlib.h>

typedef unsigned char cell_t;

// Layer is 16x16 (CHUNK_DX_SHIFT x CHUNK_DX_SHIFT) cells
#define CHUNK_DX_SHIFT 4
#define CHUNK_DX (1<<CHUNK_DX_SHIFT)
#define CHUNK_DX_MASK (CHUNK_DX - 1)

#define CHUNK_DY_SHIFT 8
#define CHUNK_DY (1<<CHUNK_DY_SHIFT)
#define CHUNK_DY_MASK (CHUNK_DY - 1)

const cell_t NO_CELL = 0;
const cell_t END_OF_WORLD = 255;

template<typename T, T initValue, void (*disposeFunction)(T value) > struct InfiniteArray {
private:
	T * data;
	int size;
	int minIdx;
	int maxIdx;
	void resize(int sz) {
		if (size < sz) {
			data = (T*)realloc(data, sizeof(T) * sz);
			for (int i = size; i < sz; i++)
				data[i] = initValue;
			size = sz;
		}
	}
public:
	int minIndex() {
		return minIdx;
	}
	int maxIndex() {
		return maxIdx;
	}
	void set(int index, T value) {
		int idx = index < 0 ? (-index) * 2 - 1 : index * 2;
		resize(idx);
		if (data[idx] != initValue)
			disposeFunction(data[idx]);
		data[idx] = value;
		if (minIdx > index)
			minIdx = index;
		if (maxIdx < index + 1)
			maxIdx = index;
	}
	T get(int index) {
		if (index < minIdx || index >= maxIdx)
			return initValue;

	}
	InfiniteArray() : data(NULL), size(0), minIdx(0), maxIdx(0) {
	}
	~InfiniteArray() {
		if (data) {
			for (int i = 0; i < size; i++) {
				if (data[i] != initValue)
					disposeFunction(data[i]);
			}
			free(data);
		}
		data = NULL;
		size = 0;
	}

};

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
		if (!this)
			return NO_CELL;
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
		if (!this)
			return NO_CELL;
		ChunkLayer * layer = layers[y & CHUNK_DY_MASK];
		if (!layer)
			return NO_CELL;
		return layer->get(x & CHUNK_DX_MASK, z & CHUNK_DY_MASK);
	}
	inline cell_t set(int x, int y, int z, cell_t cell) {
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

typedef InfiniteArray<Chunk*, NULL, Chunk::dispose> ChunkStripe;
void disposeChunkStripe(ChunkStripe * p) {
	delete p;
}
typedef InfiniteArray<ChunkStripe*, NULL, disposeChunkStripe> ChunkStripes;

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

struct World {
	ChunkMatrix chunks;
public:
	World() {

	}
	~World() {

	}
	cell_t getCell(int x, int y, int z) {
		int chunkx = x >> CHUNK_DX_SHIFT;
		int chunkz = z >> CHUNK_DX_SHIFT;
		Chunk * p = chunks.get(chunkx, chunkz);
		if (!p)
			return NO_CELL;
		return p->get(x & CHUNK_DX_MASK, y, z & CHUNK_DX_MASK);
	}
	void setCell(int x, int y, int z, cell_t value) {
		int chunkx = x >> CHUNK_DX_SHIFT;
		int chunkz = z >> CHUNK_DX_SHIFT;
		Chunk * p = chunks.get(chunkx, chunkz);
		if (!p) {
			p = new Chunk();
			chunks.set(chunkx, chunkz, p);
		}
		p->set(x & CHUNK_DX_MASK, y, z & CHUNK_DX_MASK, value);
	}
};

#endif WORLD_H_INCLUDED
