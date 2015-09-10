#include "worldtypes.h"

/// v is zero based destination coordinates
void VolumeData::putLayer(Vector3d v, cell_t * layer, int dx, int dz, int stripe) {
	cell_t * dst = _data + ((v.y << (ROW_BITS * 2)) | (v.z << ROW_BITS) | v.x);
	//int nzcount = 0;
	for (int z = 0; z < dz; z++) {
		memcpy(dst, layer, sizeof(cell_t) * dx);
		//for (int x = 0; x < dx; x++)
		//	if (dst[x])
		//		nzcount++;
		layer += stripe;
		dst += ROW_SIZE;

	}
	//CRLog::trace("  non-zero cells copied: %d", nzcount);
}

#define UPDATE_CELL(dir) \
				if (mask & (1 << dir)) { \
					cell_t cell = _data[index + directionDelta[dir]]; \
					cells[dir] = cell; \
					dirs[flagCount++] = dir; \
					if (!cell || cell == VISITED_CELL) \
						emptyCellMask |= (1<<dir); \
				}


static DirEx NEAR_DIRECTIONS_FOR[6 * 8] = {
	// NORTH
	DIR_EAST, DIR_UP, DIR_WEST, DIR_DOWN,		DIR_EAST_UP, DIR_WEST_UP, DIR_WEST_DOWN, DIR_EAST_DOWN,
	// SOUTH
	DIR_EAST, DIR_UP, DIR_WEST, DIR_DOWN,		DIR_EAST_UP, DIR_WEST_UP, DIR_WEST_DOWN, DIR_EAST_DOWN,
	// WEST
	DIR_SOUTH, DIR_UP, DIR_NORTH, DIR_DOWN,		DIR_SOUTH_UP, DIR_NORTH_UP, DIR_NORTH_DOWN, DIR_SOUTH_DOWN,
	// EAST
	DIR_SOUTH, DIR_UP, DIR_NORTH, DIR_DOWN,		DIR_SOUTH_UP, DIR_NORTH_UP, DIR_NORTH_DOWN, DIR_SOUTH_DOWN,
	// UP
	DIR_EAST, DIR_NORTH, DIR_WEST, DIR_SOUTH,	DIR_NORTH_EAST, DIR_NORTH_WEST, DIR_SOUTH_WEST, DIR_SOUTH_EAST,
	// DOWN
	DIR_EAST, DIR_NORTH, DIR_WEST, DIR_SOUTH,	DIR_NORTH_EAST, DIR_NORTH_WEST, DIR_SOUTH_WEST, DIR_SOUTH_EAST,
};

VolumeData::VolumeData(int distBits) : MAX_DIST_BITS(distBits) {
	ROW_BITS = MAX_DIST_BITS + 1;
	MAX_DIST = 1 << MAX_DIST_BITS;
	ROW_SIZE = 1 << (MAX_DIST_BITS + 1);
	DATA_SIZE = ROW_SIZE * ROW_SIZE * ROW_SIZE;
	ROW_MASK = ROW_SIZE - 1;
	_data = new cell_t[DATA_SIZE];
	clear();
	for (int i = 0; i < 64; i++) {
		int delta = 0;
		if (i & MASK_NORTH)
			delta--;
		if (i & MASK_SOUTH)
			delta++;
		if (i & MASK_WEST)
			delta -= ROW_SIZE;
		if (i & MASK_EAST)
			delta += ROW_SIZE;
		if (i & MASK_UP)
			delta += ROW_SIZE * ROW_SIZE;
		if (i & MASK_DOWN)
			delta -= ROW_SIZE * ROW_SIZE;
		directionDelta[i] = delta;
	}
	for (int d = DIR_MIN; d < DIR_MAX; d++)
		directionExDelta[d] = directionDelta[DIR_TO_MASK[d]];
	for (int d = 0; d < 6; d++) {
		DirEx * dirs = NEAR_DIRECTIONS_FOR + 8 * d;
		mainDirectionDeltas[d][0] = directionExDelta[d];
		mainDirectionDeltasNoForward[d][0] = 0;
		for (int i = 0; i < 8; i++) {
			mainDirectionDeltas[d][1 + i] = mainDirectionDeltas[d][0] + directionExDelta[dirs[i]];
			mainDirectionDeltasNoForward[d][1 + i] = directionExDelta[dirs[i]];
		}
		//CRLog::trace("Direction : %d", d);
		//for (int i = 0; i < 9; i++) {
		//	int delta = mainDirectionDeltas[d][i];
		//	Vector3d pt = indexToPoint(getIndex(Vector3d()) + delta);
		//	CRLog::trace("Delta : %d,%d,%d", pt.x, pt.y, pt.z);
		//}
	}
}

void VolumeData::fillLayer(int y, cell_t cell) {
	y += MAX_DIST;
	if (y >= 0 && y < ROW_SIZE) {
		int index = y << (ROW_BITS * 2);
		memset(_data + index, cell, ROW_SIZE * ROW_SIZE);
	}
}

void VolumeData::getNearCellsForDirection(int index, DirEx direction, CellToVisit cells[9]) {
	int * deltas = mainDirectionDeltas[direction];
	CellToVisit * cell = cells + 0;
	cell->index = index + *deltas;
	cell->cell = _data[cell->index];
	cell->dir = direction;
	//if (!cell->cell || cell->cell == VISITED_CELL) {
	for (int i = 0; i < 8; i++) {
		cell++;
		deltas++;
		cell->index = index + *deltas;
		cell->cell = _data[cell->index];
		cell->dir = direction;
	}
	//}
}

void VolumeData::getNearCellsForDirectionNoForward(int index, DirEx direction, CellToVisit cells[9]) {
	int * deltas = mainDirectionDeltasNoForward[direction];
	CellToVisit * cell = cells + 0;
	cell->index = index + *deltas;
	cell->cell = _data[cell->index];
	cell->dir = direction;
	//if (!cell->cell || cell->cell == VISITED_CELL) {
	for (int i = 0; i < 8; i++) {
		cell++;
		deltas++;
		cell->index = index + *deltas;
		cell->cell = _data[cell->index];
		cell->dir = direction;
	}
	//}
}

void VolumeData::getNearCellsForDirection(int index, DirEx direction, cell_t cells[9]) {
	int * deltas = mainDirectionDeltas[direction];
	for (int i = 0; i < 9; i++)
		cells[i] = _data[index + deltas[i]];
}
void VolumeData::getNearCellsForDirectionNoForward(int index, DirEx direction, cell_t cells[9]) {
	int * deltas = mainDirectionDeltasNoForward[direction];
	for (int i = 0; i < 9; i++)
		cells[i] = _data[index + deltas[i]];
}

