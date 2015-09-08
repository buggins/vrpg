#include "worldtypes.h"

/// v is zero based destination coordinates
void VolumeData::putLayer(Vector3d v, cell_t * layer, int dx, int dz, int stripe) {
	cell_t * dst = _data + ((v.y << (ROW_BITS * 2)) | (v.z << ROW_BITS) | v.x);
	for (int z = 0; z < dz; z++) {
		memcpy(dst, layer, sizeof(cell_t) * dx);
		layer += stripe;
		dst += ROW_SIZE;
	}
}

#define UPDATE_CELL(dir) \
				if (mask & (1 << dir)) { \
					cell_t cell = _data[index + directionDelta[dir]]; \
					cells[dir] = cell; \
					dirs[flagCount++] = dir; \
					if (!cell || cell == VISITED_CELL) \
						emptyCellMask |= (1<<dir); \
				}


/// get all near cells for specified position
cell_t VolumeData::getNearCells(int index, cell_t cells[]) {
	for (int d = DIR_MIN; d < DIR_MAX; d++)
		cells[d] = _data[index + directionExDelta[d]];
	return _data[index];
}

static DirEx NEAR_DIRECTIONS_FOR[6 * 8] = {
	// NORTH
	DIR_EAST, DIR_WEST, DIR_UP, DIR_DOWN, DIR_EAST_UP, DIR_WEST_UP, DIR_EAST_DOWN, DIR_WEST_DOWN,
	// SOUTH
	DIR_EAST, DIR_WEST, DIR_UP, DIR_DOWN, DIR_EAST_UP, DIR_WEST_UP, DIR_EAST_DOWN, DIR_WEST_DOWN,
	// WEST
	DIR_NORTH, DIR_SOUTH, DIR_UP, DIR_DOWN, DIR_NORTH_UP, DIR_SOUTH_UP, DIR_NORTH_DOWN, DIR_SOUTH_DOWN,
	// EAST
	DIR_NORTH, DIR_SOUTH, DIR_UP, DIR_DOWN, DIR_NORTH_UP, DIR_SOUTH_UP, DIR_NORTH_DOWN, DIR_SOUTH_DOWN,
	// UP
	DIR_NORTH, DIR_SOUTH, DIR_EAST, DIR_WEST, DIR_NORTH_EAST, DIR_SOUTH_EAST, DIR_NORTH_WEST, DIR_SOUTH_WEST,
	// DOWN
	DIR_NORTH, DIR_SOUTH, DIR_EAST, DIR_WEST, DIR_NORTH_EAST, DIR_SOUTH_EAST, DIR_NORTH_WEST, DIR_SOUTH_WEST,
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
		for (int i = 0; i < 8; i++)
			mainDirectionDeltas[d][1 + i] = mainDirectionDeltas[d][0] + directionExDelta[dirs[i]];
		CRLog::trace("Direction : %d", d);
		for (int i = 0; i < 9; i++) {
			int delta = mainDirectionDeltas[d][i];
			Vector3d pt = indexToPoint(getIndex(Vector3d()) + delta);
			CRLog::trace("Delta : %d,%d,%d", pt.x, pt.y, pt.z);
		}
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
	if (!cell->cell || cell->cell == VISITED_CELL) {
		for (int i = 0; i < 8; i++) {
			cell++;
			deltas++;
			cell->index = index + *deltas;
			cell->cell = _data[cell->index];
			cell->dir = direction;
		}
	}
}

/// return number of found directions for passed flags, cells are returned using DirEx index
int VolumeData::getNear(int index, int mask, cell_t cells[], DirEx dirs[], int & emptyCellMask) {
	emptyCellMask = 0;
	int flagCount = 0;
	if (mask & (MASK_EX_NORTH | MASK_EX_SOUTH | MASK_EX_WEST | MASK_EX_EAST | MASK_EX_UP | MASK_EX_DOWN | MASK_EX_WEST_UP 
			| MASK_EX_EAST_UP | MASK_EX_WEST_DOWN | MASK_EX_EAST_DOWN | MASK_EX_NORTH_WEST | MASK_EX_NORTH_EAST | MASK_EX_NORTH_UP)) {
		if (mask & (MASK_EX_NORTH | MASK_EX_SOUTH | MASK_EX_WEST | MASK_EX_EAST | MASK_EX_UP | MASK_EX_DOWN | MASK_EX_WEST_UP)) {
			if (mask & (MASK_EX_NORTH | MASK_EX_SOUTH | MASK_EX_WEST | MASK_EX_EAST)) {
				if (mask & (MASK_EX_NORTH | MASK_EX_SOUTH)) {
					UPDATE_CELL(DIR_NORTH);
					UPDATE_CELL(DIR_SOUTH);
				}
				if (mask & (MASK_EX_WEST | MASK_EX_EAST)) {
					UPDATE_CELL(DIR_WEST);
					UPDATE_CELL(DIR_EAST);
				}
			}
			if (mask & (MASK_EX_UP | MASK_EX_DOWN | MASK_EX_WEST_UP)) {
				UPDATE_CELL(DIR_UP);
				UPDATE_CELL(DIR_DOWN);
				UPDATE_CELL(DIR_WEST_UP);
			}
		}
		if (mask & (MASK_EX_EAST_UP | MASK_EX_WEST_DOWN | MASK_EX_EAST_DOWN | MASK_EX_NORTH_WEST | MASK_EX_NORTH_EAST | MASK_EX_NORTH_UP)) {
			if (mask & (MASK_EX_EAST_UP | MASK_EX_WEST_DOWN | MASK_EX_EAST_DOWN)) {
				UPDATE_CELL(DIR_EAST_UP);
				UPDATE_CELL(DIR_WEST_DOWN);
				UPDATE_CELL(DIR_EAST_DOWN);
			}
			if (mask & (MASK_EX_NORTH_WEST | MASK_EX_NORTH_EAST | MASK_EX_NORTH_UP)) {
				UPDATE_CELL(DIR_NORTH_WEST);
				UPDATE_CELL(DIR_NORTH_EAST);
				UPDATE_CELL(DIR_NORTH_UP);
			}
		}
	}
	if (mask & (MASK_EX_NORTH_DOWN | MASK_EX_NORTH_WEST_UP | MASK_EX_NORTH_EAST_UP | MASK_EX_NORTH_WEST_DOWN | MASK_EX_NORTH_EAST_DOWN | MASK_EX_SOUTH_WEST | MASK_EX_SOUTH_EAST 
			| MASK_EX_SOUTH_UP | MASK_EX_SOUTH_DOWN | MASK_EX_SOUTH_WEST_UP | MASK_EX_SOUTH_EAST_UP | MASK_EX_SOUTH_WEST_DOWN | MASK_EX_SOUTH_EAST_DOWN)) {
		if (mask & (MASK_EX_NORTH_DOWN | MASK_EX_NORTH_WEST_UP | MASK_EX_NORTH_EAST_UP | MASK_EX_NORTH_WEST_DOWN | MASK_EX_NORTH_EAST_DOWN | MASK_EX_SOUTH_WEST | MASK_EX_SOUTH_EAST)) {
			if (mask & (MASK_EX_NORTH_DOWN | MASK_EX_NORTH_WEST_UP | MASK_EX_NORTH_EAST_UP | MASK_EX_NORTH_WEST_DOWN)) {
				if (mask & (MASK_EX_NORTH_DOWN | MASK_EX_NORTH_WEST_UP)) {
					UPDATE_CELL(DIR_NORTH_DOWN);
					UPDATE_CELL(DIR_NORTH_WEST_UP);
				}
				if (mask & (MASK_EX_NORTH_EAST_UP | MASK_EX_NORTH_WEST_DOWN)) {
					UPDATE_CELL(DIR_NORTH_EAST_UP);
					UPDATE_CELL(DIR_NORTH_WEST_DOWN);
				}
			}
			if (mask & (MASK_EX_NORTH_EAST_DOWN | MASK_EX_SOUTH_WEST | MASK_EX_SOUTH_EAST)) {
				UPDATE_CELL(DIR_NORTH_EAST_DOWN);
				UPDATE_CELL(DIR_SOUTH_WEST);
				UPDATE_CELL(DIR_SOUTH_EAST);
			}
		}
		if (mask & (MASK_EX_SOUTH_UP | MASK_EX_SOUTH_DOWN | MASK_EX_SOUTH_WEST_UP | MASK_EX_SOUTH_EAST_UP | MASK_EX_SOUTH_WEST_DOWN | MASK_EX_SOUTH_EAST_DOWN)) {
			if (mask & (MASK_EX_SOUTH_UP | MASK_EX_SOUTH_DOWN | MASK_EX_SOUTH_WEST_UP)) {
				UPDATE_CELL(DIR_SOUTH_UP);
				UPDATE_CELL(DIR_SOUTH_DOWN);
				UPDATE_CELL(DIR_SOUTH_WEST_UP);
			}
			if (mask & (MASK_EX_SOUTH_EAST_UP | MASK_EX_SOUTH_WEST_DOWN | MASK_EX_SOUTH_EAST_DOWN)) {
				UPDATE_CELL(DIR_SOUTH_EAST_UP);
				UPDATE_CELL(DIR_SOUTH_WEST_DOWN);
				UPDATE_CELL(DIR_SOUTH_EAST_DOWN);
			}
		}
	}
	return flagCount;
}
