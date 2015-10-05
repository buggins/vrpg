#include "world.h"
#include <stdio.h>
#include <assert.h>
#include "logger.h"
#include "blocks.h"

bool HIGHLIGHT_GRID = true;

bool World::isOpaque(Vector3d v) {
	cell_t cell = getCell(v);
	return BLOCK_TYPE_OPAQUE[cell] && cell != BOUND_SKY;
}

cell_t World::getCell(int x, int y, int z) {
	//y += CHUNK_DY / 2;
	if (y < 0)
		return 3;
	int chunkx = x >> CHUNK_DX_SHIFT;
	int chunkz = z >> CHUNK_DX_SHIFT;
	Chunk * p;
	if (lastChunkX == chunkx && lastChunkZ == chunkz) {
		p = lastChunk;
	}
	else {
		p = chunks.get(chunkx, chunkz);
		lastChunkX = chunkx;
		lastChunkZ = chunkz;
		lastChunk = p;
	}
	if (!p)
		return NO_CELL;
	return p->get(x & CHUNK_DX_MASK, y, z & CHUNK_DX_MASK);
}

bool World::canPass(Vector3d pos, Vector3d size) {
	for (int x = 0; x <= size.x; x++)
		for (int z = 0; z <= size.z; z++)
			for (int y = 0; y < size.y; y++) {
				Vector3d p(pos.x + x, pos.y + y, pos.z + z);
				if (isOpaque(p))
					return false;
			}
	return true;
}

void World::setCell(int x, int y, int z, cell_t value) {
	//y += CHUNK_DY / 2;
	int chunkx = x >> CHUNK_DX_SHIFT;
	int chunkz = z >> CHUNK_DX_SHIFT;
	Chunk * p;
	if (lastChunkX == chunkx && lastChunkZ == chunkz) {
		p = lastChunk;
	}
	else {
		p = chunks.get(chunkx, chunkz);
		lastChunkX = chunkx;
		lastChunkZ = chunkz;
		lastChunk = p;
	}
	if (!p) {
		p = new Chunk();
		chunks.set(chunkx, chunkz, p);
		lastChunkX = chunkx;
		lastChunkZ = chunkz;
		lastChunk = p;
	}
	p->set(x & CHUNK_DX_MASK, y, z & CHUNK_DX_MASK, value);
}


void Direction::set(Dir d) {
	switch (d) {
	default:
	case NORTH:
		set(0, 0, -1);
		break;
	case SOUTH:
		set(0, 0, 1);
		break;
	case WEST:
		set(-1, 0, 0);
		break;
	case EAST:
		set(1, 0, 0);
		break;
	case UP:
		set(0, 1, 0);
		break;
	case DOWN:
		set(0, -1, 0);
		break;
	}
}
void Direction::set(int x, int y, int z) {
	forward = Vector3d(x, y, z);
	if (x) {
		dir = (x > 0) ? EAST : WEST;
	}
	else if (y) {
		dir = (y > 0) ? UP : DOWN;
	}
	else {
		dir = (z > 0) ? SOUTH : NORTH;
	}
	switch (dir) {
	case UP:
		up = Vector3d(1, 0, 0);
		left = Vector3d(0, 0, 1);
		break;
	case DOWN:
		up = Vector3d(1, 0, 0);
		left = Vector3d(0, 0, -1);
		break;
	case NORTH:
		up = Vector3d(0, 1, 0);
		left = Vector3d(-1, 0, 0);
		break;
	case SOUTH:
		up = Vector3d(0, 1, 0);
		left = Vector3d(1, 0, 0);
		break;
	case EAST:
		up = Vector3d(0, 1, 0);
		left = Vector3d(0, 0, -1);
		break;
	case WEST:
		up = Vector3d(0, 1, 0);
		left = Vector3d(0, 0, 1);
		break;
	}
	down = -up;
	right = -left;
	forwardUp = forward + up;
	forwardDown = forward + down;
	forwardLeft = forward + left;
	forwardLeftUp = forward + left + up;
	forwardLeftDown = forward + left + down;
	forwardRight = forward + right;
	forwardRightUp = forward + right + up;
	forwardRightDown = forward + right + down;
}

struct VisitorHelper {
	World & world;
	Position & position;
	int distance;
	Vector3dArray oldcells;
	Vector3dArray newcells;
	BoolSymmetricMatrix visited;
	CellVisitor * visitor;
	VisitorHelper(World & w, Position & p, CellVisitor * v) : world(w), position(p), visitor(v) {
	}
	~VisitorHelper() {
	}
	void newRange(int distance) {
		CRLog::trace("new range: %d\n", distance);
		visited.reset(distance + 1);
		oldcells.swap(newcells);
		newcells.clear();
	}
	void visitVisibleFace(Vector3d newpt, Dir face) {
		Vector3d nextpt = newpt;
		switch (face) {
		default:
		case NORTH:
			nextpt.z--;
			break;
		case SOUTH:
			nextpt.z++;
			break;
		case WEST:
			nextpt.x++;
			break;
		case EAST:
			nextpt.x--;
			break;
		case UP:
			nextpt.y--;
			break;
		case DOWN:
			nextpt.y++;
			break;
		}
		cell_t cell = world.getCell(nextpt);
		if (cell) {
			Vector3d v1 = newpt - position.pos;
			Vector3d v2 = nextpt - newpt;
			int p = v1 * v2;
			if (p > 0) {
				visitor->visitFace(&world, position, nextpt, cell, face);
			}
		}

	}

	void visitVisibleFaces(Vector3d newpt) {
		visitVisibleFace(newpt, UP);
		visitVisibleFace(newpt, DOWN);
		visitVisibleFace(newpt, NORTH);
		visitVisibleFace(newpt, SOUTH);
		visitVisibleFace(newpt, EAST);
		visitVisibleFace(newpt, WEST);
	}

	void planVisits(Vector3d pt) {
		//fprintf(log, "plan visits %d, %d, %d\n", pt.x, pt.y, pt.z);
		needVisit(pt + position.direction.forward);
		needVisit(pt + position.direction.forwardLeft);
		needVisit(pt + position.direction.forwardRight);
		needVisit(pt + position.direction.forwardUp);
		needVisit(pt + position.direction.forwardDown);
		needVisit(pt + position.direction.forwardLeftUp);
		needVisit(pt + position.direction.forwardLeftDown);
		needVisit(pt + position.direction.forwardRightUp);
		needVisit(pt + position.direction.forwardRightDown);
	}

	bool needVisit(Vector3d newpt) {
		Vector2d planeCoord = position.calcPlaneCoords(newpt);
		cell_t cell = world.getCell(newpt);
		bool isVisited = visited.get(planeCoord.x, planeCoord.y);
		if (cell == NO_CELL && !isVisited) {
			newcells.append(newpt);
			//fprintf(log, "    planned visit %d,%d,%d\n", newpt.x, newpt.y, newpt.z);
			visited.set(planeCoord.x, planeCoord.y, true);
			return true;
		}
		return false;
	}
};

//bool inline canPass(cell_t cell) { return !cell || cell == VISITED_CELL;  }

//int calcDistance(Vector3d v1, Vector3d v2) {
//	Vector3d v = v1 - v2;
//	if (v.x < 0)
//		v.x = -v.x;
//	if (v.y < 0)
//		v.y = -v.y;
//	if (v.z < 0)
//		v.z = -v.z;
//	return v.x + v.y + v.z;
//}

VolumeVisitor::VolumeVisitor() {
}
void VolumeVisitor::init(World * w, Position * pos, VolumeData * data, CellVisitor * v) {
	world = w;
	volume = data;
	visitor = v;
	position = pos;
	direction = (DirEx)pos->direction.dir;
	oppdirection = (DirEx)(direction ^ 1);
	dirvector = pos->direction.forward;
}
VolumeVisitor::~VolumeVisitor() {
}

bool VolumeVisitor::visitCell(int index, cell_t cell) {
	if (cell == BOUND_SKY || cell >= VISITED_OCCUPIED)
		return false;
	// mark as visited
	volume->put(index, BLOCK_TYPE_CAN_PASS[cell] ? VISITED_CELL : VISITED_OCCUPIED);

	Vector3d pt = volume->indexToPoint(index);
	if (distance > 5 && pt * dirvector < distance)
		return false;

	if (BLOCK_TYPE_VISIBLE[cell]) {
		// call visitor callback
		//int threshold = (distance * 15 / 16);
		Vector3d pos = pt + position->pos;
		int visibleFaces = 0;
		if (pt.y <= 0 && pt * DIRECTION_VECTORS[DIR_UP] <= 0 &&
			!world->isOpaque(pos.move(DIR_UP)))
			visibleFaces |= MASK_UP;
		if (pt.y >= 0 && pt * DIRECTION_VECTORS[DIR_DOWN] <= 0 &&
			!world->isOpaque(pos.move(DIR_DOWN)))
			visibleFaces |= MASK_DOWN;
		if (pt.x <= 0 && pt * DIRECTION_VECTORS[DIR_EAST] <= 0 &&
			!world->isOpaque(pos.move(DIR_EAST)))
			visibleFaces |= MASK_EAST;
		if (pt.x >= 0 && pt * DIRECTION_VECTORS[DIR_WEST] <= 0 &&
			!world->isOpaque(pos.move(DIR_WEST)))
			visibleFaces |= MASK_WEST;
		if (pt.z <= 0 && pt * DIRECTION_VECTORS[DIR_SOUTH] <= 0 &&
			!world->isOpaque(pos.move(DIR_SOUTH)))
			visibleFaces |= MASK_SOUTH;
		if (pt.z >= 0 && pt * DIRECTION_VECTORS[DIR_NORTH] <= 0 &&
			!world->isOpaque(pos.move(DIR_NORTH)))
			visibleFaces |= MASK_NORTH;
		visitor->visit(world, *position, pos, cell, visibleFaces);
	}

	return BLOCK_TYPE_CAN_PASS[cell];
}
void VolumeVisitor::appendNewCell(int index, int distance) {
	Vector3d pos = volume->indexToPoint(index);
	if (dirvector * pos < 0)
		return; // skip opposite direction

	if (pos.x == -distance)
		helpers[DIR_WEST].newcells.append(index);
	if (pos.x == distance)
		helpers[DIR_EAST].newcells.append(index);
	if (pos.z == -distance)
		helpers[DIR_NORTH].newcells.append(index);
	if (pos.z == distance)
		helpers[DIR_SOUTH].newcells.append(index);
	if (pos.y == -distance)
		helpers[DIR_DOWN].newcells.append(index);
	if (pos.y == distance)
		helpers[DIR_UP].newcells.append(index);
}

void VolumeVisitor::visitPlaneForward(int startIndex, DirEx direction) {
	DirectionHelper & helper = helpers[direction];
	cell_t * data = volume->ptr();
	int * thisPlaneDirections = volume->thisPlaneDirections(direction);
	int * nextPlaneDirections = volume->nextPlaneDirections(direction);
	int dist = distance + 1;

	// spread forward between planes oldcells->newcells
	for (int i = 0; i < helper.oldcells.length(); i++) {
		int forwardIndex = helper.oldcells[i] + nextPlaneDirections[0]; // index in next plane
		cell_t cell = data[forwardIndex];
		if (visitCell(forwardIndex, cell))
			helper.newcells.append(forwardIndex);
	}

	helper.prepareSpreading();
}

// move in forward direction
void VolumeVisitor::visitPlaneSpread(int startIndex, DirEx direction) {
	DirectionHelper & helper = helpers[direction];
	cell_t * data = volume->ptr();
	int * thisPlaneDirections = volume->thisPlaneDirections(direction);
	int * nextPlaneDirections = volume->nextPlaneDirections(direction);
	int dist = distance + 1;

	// spread by one cell inside new plane
	int directCells = helper.forwardCellCount;
	// phase 1: just find indexes of direct and diagonal cells to visit
	for (int i = 0; i < directCells; i++) {
		int index = helper.newcells[i];
		for (int dir = 1; dir <= 4; dir++) {
			// forward
			int newindex = index + thisPlaneDirections[dir];
			cell_t cell = data[newindex];
			if (cell < VISITED_OCCUPIED) {
				helper.spreadcells.append(newindex);
			}
			if (BLOCK_TYPE_CAN_PASS[cell]) {
				// diagonal
				int index0 = index + thisPlaneDirections[dir + 4];
				cell = data[index0];
				if (cell < VISITED_OCCUPIED)
					helper.spreadcells.append(index0);
				int prevdir = dir == 1 ? 8 : dir + 3;
				index0 = index + thisPlaneDirections[prevdir];
				cell = data[index0];
				if (cell < VISITED_OCCUPIED)
					helper.spreadcells.append(index0);
			}
		}
	}
	// phase 2: visit cells
	for (int i = helper.spreadcells.length() - 1; i >= 0; i--) {
		int newindex = helper.spreadcells[i];
		cell_t cell = data[newindex];
		if (cell < VISITED_OCCUPIED && visitCell(newindex, cell)) {
			appendNewCell(newindex, dist);
		}
	}
}

void VolumeVisitor::visitAll() {
	lUInt64 startTs = GetCurrentTimeMillis();
	//CRLog::trace("VolumeVisitor2::visitAll() enter");
	int startIndex = volume->getIndex(Vector3d());
	cell_t cell = volume->get(startIndex);
	volume->put(startIndex, VISITED_CELL);
	for (int i = 0; i < 6; i++)
		helpers[i].start(startIndex, (DirEx)i);
	for (distance = 0; distance < volume->size() - 2; distance++) {
		for (int dir = 5; dir >= 0; dir--)
			if (dir != oppdirection)
				helpers[dir].nextDistance();
		for (int dir = 5; dir >= 0; dir--)
			if (dir != oppdirection)
				visitPlaneForward(startIndex, (DirEx)dir);
		for (int dir = 5; dir >= 0; dir--)
			if (dir != oppdirection)
				visitPlaneSpread(startIndex, (DirEx)dir);
	}
	lUInt64 duration = GetCurrentTimeMillis() - startTs;
	CRLog::trace("VolumeVisitor2::visitAll() exit, lookup took %lld millis", duration);
}



void World::visitVisibleCellsAllDirectionsFast(Position & position, CellVisitor * visitor) {
#if USE_DIAMOND_VISITOR==1
	visitorHelper.init(this, &position, visitor);
	visitorHelper.visitAll(MAX_VIEW_DISTANCE);
#else
	volumeSnapshotInvalid = true;
	updateVolumeSnapshot();
	visitorHelper.init(this, &position, &volumeSnapshot, visitor);
	visitorHelper.visitAll();
#endif
}

void disposeChunkStripe(ChunkStripe * p) {
	delete p;
}

void World::updateVolumeSnapshot() {
#if USE_DIAMOND_VISITOR!=1
	if (!volumeSnapshotInvalid && volumePos == camPosition.pos)
		return;
	volumePos = camPosition.pos;
	getCellsNear(camPosition.pos, volumeSnapshot);
	volumeSnapshotInvalid = false;
#endif
}

void Chunk::getCells(Vector3d srcpos, Vector3d dstpos, Vector3d size, VolumeData & buf) {
	//CRLog::trace("getCells src=%d,%d,%d  dst=%d,%d,%d  sz=%d,%d,%d", srcpos.x, srcpos.y, srcpos.z
	//	, dstpos.x, dstpos.y, dstpos.z
	//	, size.x, size.y, size.z
	//	);
	for (int y = 0; y < size.y; y++) {
		int yy = srcpos.y + y;
		if (yy >= 0 && yy < CHUNK_DY) {
			ChunkLayer * layer = layers[yy];
			if (layer) {
				Vector3d v = dstpos;
				v.y += y;
				//CRLog::trace("putLayer %d  %d,%d %dx%d  to   %d,%d,%d", yy, srcpos.x, srcpos.z, size.x, size.z, v.x, v.y, v.z);
				buf.putLayer(v, layer->ptr(srcpos.x, srcpos.z), size.x, size.z, CHUNK_DX);
			}
		}
	}
}

void World::getCellsNear(Vector3d pos, VolumeData & buf) {
	Vector3d v = pos;
	buf.clear();
	//setCell(pos.x, pos.y, pos.z, 15);
	//setCell(pos.x + 1, pos.y, pos.z, 16);
	//setCell(pos.x, pos.y, pos.z + 2, 17);
	//setCell(pos.x, pos.y + 5, pos.z, 18);
	int sz = buf.size();
	int y0 = v.y;
	v.x -= sz;
	v.y -= sz;
	v.z -= sz;
	int dsty = 0;
	int dy = sz * 2;
	Vector3d endv = v;
	endv.x += sz * 2;
	endv.y += sz * 2;
	endv.z += sz * 2;
	if (v.y < 0) {
		dy -= -v.y;
		dsty += -v.y;
		v.y = 0;
	}
	int minLayer = -1;
	int maxLayer = -1;
	for (int z = v.z; z < endv.z;) {
		int zz = z & CHUNK_DX_MASK;
		int nextz = z + CHUNK_DX - zz;
		if (nextz > endv.z)
			nextz = endv.z;
		int dz = nextz - z;
		int chunkz = z >> CHUNK_DX_SHIFT;
		for (int x = v.x; x < endv.x;) {
			int xx = x & CHUNK_DX_MASK;
			int nextx = x + CHUNK_DX - xx;
			if (nextx > endv.x)
				nextx = endv.x;
			int dx = nextx - x;
			int chunkx = x >> CHUNK_DX_SHIFT;
			Chunk * p = chunks.get(chunkx, chunkz);
			if (p) {
				p->updateMinMaxLayer(minLayer, maxLayer);
				//CRLog::trace("chunk %d,%d  pos %d,%d next %d,%d", chunkx, chunkz, x, z, nextx, nextz);
				p->getCells(
					Vector3d(xx, v.y, zz),
					Vector3d(x - v.x, dsty, z - v.z),
					Vector3d(dx, dy, dz),
					buf);
			}
			x = nextx;
		}
		z = nextz;
	}

	//assert(buf.get(Vector3d(0, 0, 0)) == 15);
	//assert(buf.get(Vector3d(1, 0, 0)) == 16);
	//assert(buf.get(Vector3d(0, 0, 2)) == 17);
	//assert(buf.get(Vector3d(0, 5, 0)) == 18);

	//for (int z = -5; z < 5; z++) {
	//	for (int y = -1; y < 5; y++) {
	//		for (int x = -5; x < 5; x++) {
	//			assert(getCell(pos + Vector3d(x, y, z)) == buf.get(Vector3d(x, y, z)));
	//		}
	//	}
	//}

	if (minLayer != -1) {
		if (minLayer > y0)
			minLayer = y0;
		if (maxLayer < y0)
			maxLayer = y0;
		buf.fillLayer(minLayer - y0 - 1, BOUND_BOTTOM);
		buf.fillLayer(maxLayer - y0 + 1, BOUND_SKY);
	}
}

#if UNIT_TESTS==1
void testVectors();


void testVectors() {
	Vector3d v1(1, 0, 0);
	Direction d1(1, 0, 0);
	assert(d1.dir == EAST);
	assert(d1.forward == Vector3d(1, 0, 0));
	assert(d1.up == Vector3d(0, 1, 0));
	assert(d1.down == Vector3d(0, -1, 0));
	assert(d1.left == Vector3d(0, 0, -1));
	assert(d1.right == Vector3d(0, 0, 1));

	Direction d3(-1, 0, 0);
	assert(d3.dir == WEST);
	assert(d3.forward == Vector3d(-1, 0, 0));
	assert(d3.up == Vector3d(0, 1, 0));
	assert(d3.down == Vector3d(0, -1, 0));
	assert(d3.left == Vector3d(0, 0, 1));
	assert(d3.right == Vector3d(0, 0, -1));

	Direction d2(0, 0, -1);
	assert(d2.dir == NORTH);
	assert(d2.forward == Vector3d(0, 0, -1));
	assert(d2.up == Vector3d(0, 1, 0));
	assert(d2.down == Vector3d(0, -1, 0));
	assert(d2.left == Vector3d(-1, 0, 0));
	assert(d2.right == Vector3d(1, 0, 0));

	Direction d4(0, 0, 1);
	assert(d4.dir == SOUTH);
	assert(d4.forward == Vector3d(0, 0, 1));
	assert(d4.up == Vector3d(0, 1, 0));
	assert(d4.down == Vector3d(0, -1, 0));
	assert(d4.left == Vector3d(1, 0, 0));
	assert(d4.right == Vector3d(-1, 0, 0));

}
#endif

void runWorldUnitTests() {
#if UNIT_TESTS==1
	testVectors();
#endif
}

/// ensure that data is in range [minvalue, maxvalue]
void TerrainGen::limit(int minvalue, int maxvalue) {
	// find actual min/max
	int minv, maxv;
	minv = maxv = get(0, 0);
	for (int y = 0; y <= dy; y++) {
		for (int x = 0; x <= dx; x++) {
			int v = get(x, y);
			if (minv > v)
				minv = v;
			if (maxv < v)
				maxv = v;
		}
	}
	int mul = (maxvalue - minvalue);
	int div = (maxv - minv);
	if (div > 0) {
		for (int y = 0; y <= dy; y++) {
			for (int x = 0; x <= dx; x++) {
				set(x, y, minvalue + (get(x, y) - minv) * mul / div);
			}
		}
	}
}

void TerrainGen::generateWithScale(int seed, short * initData, int stepBits, TerrainGen & scaleMap) {
	rnd.setSeed(seed);
	int step = 1 << stepBits;
	for (int y = 0; y <= dy; y += step) {
		for (int x = 0; x <= dx; x += step) {
			set(x, y, *initData++);
		}
	}
	int half = step >> 1;
	while (half > 0) {
		for (int y = half; y < dy; y += step) {
			for (int x = half; x < dx; x++) {
				int scale = (scaleMap.get(x, y) * step) >> 8;
				scale = rnd.nextInt(scale * 2) - scale;
				if (step < 4)
					scale = 0;
				square(x, y, half, scale);
			}
		}
		for (int y = 0; y <= dy; y += half) {
			for (int x = (y + half) % step; x <= dx; x += step) {
				int scale = (scaleMap.get(x, y) * step) >> 8;
				scale = rnd.nextInt(scale * 2) - scale;
				if (step < 4)
					scale = 0;
				diamond(x, y, half, scale);
			}
		}
		step >>= 1;
		half >>= 1;
	}
}

void TerrainGen::filter(int range) {
	short * tmp = new short[dx * dy];
	memset(tmp, 0, dx*dy*sizeof(short));
	int div = (range * 2 + 1) * (range * 2 + 1);
	for (int y = 0; y <= dy; y++) {
		for (int x = 0; x <= dx; x++) {
			int s = 0;
			for (int yy = -range; yy <= range; yy++) {
				for (int xx = -range; xx <= range; xx++) {
					s += get(x + xx, y + yy);
				}
			}
			s /= div;
			tmp[(y << ypow) + y + x] = (short)s;
		}
	}
	memcpy(data, tmp, dx*dy*sizeof(short));
	delete[] tmp;
}

void TerrainGen::generate(int seed, short * initData, int stepBits) {
	rnd.setSeed(seed);
	int step = 1 << stepBits;
	for (int y = 0; y <= dy; y += step) {
		for (int x = 0; x <= dx; x += step) {
			set(x, y, *initData++);
		}
	}
	int half = step >> 1;
	while (half > 0) {
		int scale = step;
		for (int y = half; y < dy; y += step) {
			for (int x = half; x < dx; x++) {
				square(x, y, half, rnd.nextInt(scale * 2) - scale);
			}
		}
		for (int y = 0; y <= dy; y += half) {
			for (int x = (y + half) % step; x <= dx; x += step) {
				diamond(x, y, half, rnd.nextInt(scale * 2) - scale);
			}
		}
		step >>= 1;
		half >>= 1;
	}
}


void TerrainGen::diamond(int x, int y, int size, int offset) {
	int avg = (get(x, y - size) + get(x + size, y) + get(x, y + size) + get(x - size, y)) >> 2;
	set(x, y, avg + offset);
}

void TerrainGen::square(int x, int y, int size, int offset) {
	int avg = (get(x - size, y - size) + get(x + size, y - size) + get(x - size, y + size) + get(x - size, y - size)) >> 2;
	set(x, y, avg + offset);
}

int myAbs(int d) {
	return d < 0 ? -d : d;
}

int bitsFor(int n) {
	int res;
	for (res = 0; n > 0; res++)
		n >>= 1;
	return res;
}

/// returns 0 for 0, 1 for negatives, 2 for positives
int mySign(int n) {
	if (n > 0)
		return 1;
	else if (n < 0)
		return -1;
	else
		return 0;
}

/// vector to index
int diamondIndex(Vector3d v, int distBits) {
	int m0 = 1 << distBits;
	int x = v.x + m0;
	int y = v.z + m0;
	if (v.y < 0) {
		// inverse index for lower half
		m0--;
		x ^= m0;
		y ^= m0;
	}
	int index = x + (y << (distBits + 1));
	return index;
}

/// index to vector
#if 0
Vector3d diamondVector(int index, int distBits, int dist) {
	Vector3d v;
	int xx = index & ((1 << (distBits + 1)) - 1);
	int yy = index >> (distBits + 1);
	int m0 = 1 << distBits;
	v.x = xx - m0;
	v.y = yy - m0;
	int m0dist = myAbs(v.x) + myAbs(v.y);
	if (m0dist > dist) {
		// bottom half
		int mask = m0 - 1;
		v.x = (xx ^ mask) - m0;
		v.y = (yy ^ mask) - m0;
		m0dist = myAbs(v.x) + myAbs(v.y);
		v.y = -(dist - m0dist);
	} else {
		// top half
		v.y = dist - m0dist;
	}
	return v;
}
#endif

DiamondVisitor::DiamondVisitor() 
{
}

void DiamondVisitor::init(World * w, Position * pos, CellVisitor * v) {
	world = w;
	position = pos;
	visitor = v;
	pos0 = position->pos;
}
void DiamondVisitor::visitCell(Vector3d v) {
	//CRLog::trace("visitCell(%d %d %d) dist=%d", v.x, v.y, v.z, myAbs(v.x) + myAbs(v.y) + myAbs(v.z));

	if (v * position->direction.forward < dist / 3)
		return;

	int m0 = 1 << maxDistBits;
	int x = v.x + m0;
	int y = v.z + m0;
	if (v.y < 0) {
		// inverse index for lower half
		m0--;
		x ^= m0;
		y ^= m0;
	}
	int index = x + (y << (maxDistBits + 1));
	//int index = diamondIndex(v, maxDistBits);
	cell_t cell = visited[index];
	if (cell == visitedOccupied || cell == visitedEmpty)
		return;
	// read cell from world
	Vector3d pos = pos0 + v;
	cell = world->getCell(pos);
	if (BLOCK_TYPE_VISIBLE[cell]) {
		int visibleFaces = 0;
		if (v.y <= 0 && v * DIRECTION_VECTORS[DIR_UP] <= 0 &&
			!world->isOpaque(pos.move(DIR_UP)))
			visibleFaces |= MASK_UP;
		if (v.y >= 0 && v * DIRECTION_VECTORS[DIR_DOWN] <= 0 &&
			!world->isOpaque(pos.move(DIR_DOWN)))
			visibleFaces |= MASK_DOWN;
		if (v.x <= 0 && v * DIRECTION_VECTORS[DIR_EAST] <= 0 &&
			!world->isOpaque(pos.move(DIR_EAST)))
			visibleFaces |= MASK_EAST;
		if (v.x >= 0 && v * DIRECTION_VECTORS[DIR_WEST] <= 0 &&
			!world->isOpaque(pos.move(DIR_WEST)))
			visibleFaces |= MASK_WEST;
		if (v.z <= 0 && v * DIRECTION_VECTORS[DIR_SOUTH] <= 0 &&
			!world->isOpaque(pos.move(DIR_SOUTH)))
			visibleFaces |= MASK_SOUTH;
		if (v.z >= 0 && v * DIRECTION_VECTORS[DIR_NORTH] <= 0 &&
			!world->isOpaque(pos.move(DIR_NORTH)))
			visibleFaces |= MASK_NORTH;
		visitor->visit(world, *position, pos, cell, visibleFaces);
	}
	// mark as visited
	cell = BLOCK_TYPE_CAN_PASS[cell] ? visitedEmpty : visitedOccupied;
	visited[index] = cell;
	if (cell == visitedEmpty)
		newcells.append(v);
}
void DiamondVisitor::visitAll(int maxDistance) {
	maxDist = maxDistance;
	maxDistBits = bitsFor(maxDist);
	int sz = ((1 << maxDistBits) * (1 << maxDistBits)) << 2;
	visited.clear();
	visited.append(0, sz);
	oldcells.reserve(maxDist * 4 * 4);
	newcells.reserve(maxDist * 4 * 4);

	dist = 1;

	visitedOccupied = 2;
	visitedEmpty = 3;
	oldcells.clear();
	oldcells.append(Vector3d(0, 0, 0));

	for (; dist < maxDistance; dist++) {
		// for each distance
		if (oldcells.length() == 0) // no cells to pass through
			break;
		newcells.clear();
		visitedOccupied += 2;
		visitedEmpty += 2;
		for (int i = 0; i < oldcells.length(); i++) {
			Vector3d pt = oldcells[i];
			int sx = mySign(pt.x);
			int sy = mySign(pt.y);
			int sz = mySign(pt.z);
			if (sx && sy && sz) {
				// 1, 1, 1
				visitCell(Vector3d(pt.x + sx, pt.y, pt.z));
				visitCell(Vector3d(pt.x, pt.y + sy, pt.z));
				visitCell(Vector3d(pt.x, pt.y, pt.z + sz));
			} else {
				// has 0 in one of coords
				if (!sx) {
					if (!sy) {
						if (!sz) {
							// 0, 0, 0
							visitCell(Vector3d(pt.x + 1, pt.y, pt.z));
							visitCell(Vector3d(pt.x - 1, pt.y, pt.z));
							visitCell(Vector3d(pt.x, pt.y + 1, pt.z));
							visitCell(Vector3d(pt.x, pt.y - 1, pt.z));
							visitCell(Vector3d(pt.x, pt.y, pt.z + 1));
							visitCell(Vector3d(pt.x, pt.y, pt.z - 1));
						} else {
							// 0, 0, 1
							visitCell(Vector3d(pt.x, pt.y, pt.z + sz));
							visitCell(Vector3d(pt.x + 1, pt.y, pt.z));
							visitCell(Vector3d(pt.x - 1, pt.y, pt.z));
							visitCell(Vector3d(pt.x, pt.y + 1, pt.z));
							visitCell(Vector3d(pt.x, pt.y - 1, pt.z));
						}
					} else {
						if (!sz) {
							// 0, 1, 0
							visitCell(Vector3d(pt.x, pt.y + sy, pt.z));
							visitCell(Vector3d(pt.x + 1, pt.y, pt.z));
							visitCell(Vector3d(pt.x - 1, pt.y, pt.z));
							visitCell(Vector3d(pt.x, pt.y, pt.z + 1));
							visitCell(Vector3d(pt.x, pt.y, pt.z - 1));
						} else {
							// 0, 1, 1
							visitCell(Vector3d(pt.x, pt.y + sy, pt.z));
							visitCell(Vector3d(pt.x, pt.y, pt.z + sz));
							visitCell(Vector3d(pt.x + 1, pt.y, pt.z));
							visitCell(Vector3d(pt.x - 1, pt.y, pt.z));
						}
					}
				} else {
					if (!sy) {
						if (!sz) {
							// 1, 0, 0
							visitCell(Vector3d(pt.x + sx, pt.y, pt.z));
							visitCell(Vector3d(pt.x, pt.y + 1, pt.z));
							visitCell(Vector3d(pt.x, pt.y - 1, pt.z));
							visitCell(Vector3d(pt.x, pt.y, pt.z + 1));
							visitCell(Vector3d(pt.x, pt.y, pt.z - 1));
						} else {
							// 1, 0, 1
							visitCell(Vector3d(pt.x + sx, pt.y, pt.z));
							visitCell(Vector3d(pt.x, pt.y, pt.z + sz));
							visitCell(Vector3d(pt.x, pt.y + 1, pt.z));
							visitCell(Vector3d(pt.x, pt.y - 1, pt.z));
						}
					} else {
						// 1, 1, 0
						visitCell(Vector3d(pt.x + sx, pt.y, pt.z));
						visitCell(Vector3d(pt.x, pt.y + sy, pt.z));
						visitCell(Vector3d(pt.x, pt.y, pt.z + 1));
						visitCell(Vector3d(pt.x, pt.y, pt.z - 1));
					}
				}
			}
		}
		newcells.swap(oldcells);
	}
}

/// iterator is based on Terasology implementation
/// https://github.com/MovingBlocks/Terasology/blob/develop/engine/src/main/java/org/terasology/math/Diamond3iIterator.java
class DiamondIterator {
private:
	Vector3d origin;
	int maxDistance;
	int x;
	int y;
	int z;
	int level;
public:
	DiamondIterator(Vector3d orig, int maxDist, int startDistance) : origin(orig), maxDistance(maxDist + 1), x(0), y(0), z(0) {
		level = startDistance + 1;
		x = -level;
	}

	bool hasNext() {
		return level < maxDistance;
	}

	Vector3d next() {
		Vector3d result(origin.x + x, origin.y + y, origin.z + z);
		if (z < 0) {
			z *= -1;
		}
		else if (y < 0) {
			y *= -1;
			z = -(level - myAbs(x) - myAbs(y));
		}
		else {
			y = -y + 1;
			if (y > 0) {
				if (++x <= level) {
					y = myAbs(x) - level;
					z = 0;
				}
				else {
					level++;
					x = -level;
					y = 0;
					z = 0;
				}
			}
			else {
				z = -(level - myAbs(x) - myAbs(y));
			}
		}

		return result;
	}
};
