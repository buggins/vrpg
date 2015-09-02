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

