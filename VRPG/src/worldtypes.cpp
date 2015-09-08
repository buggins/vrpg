#include "worldtypes.h"

/// v is zero based destination coordinates
void VolumeData::putLayer(Vector3d v, cell_t * layer, int dx, int dz, int stripe) {
	cell_t * dst = _data + ((v.y << (ROW_SIZE * 2)) | (v.z << ROW_SIZE) | v.x);
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
