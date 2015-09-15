#include "world.h"
#include <stdio.h>
#include <assert.h>
#include "logger.h"
#include "blocks.h"

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

struct DirectionHelper {
	DirEx dir;
	IntArray oldcells;
	IntArray newcells;
	IntArray spreadcells;
	int forwardCellCount;
	void start(int index, DirEx direction) {
		dir = direction;
		oldcells.clear();
		newcells.clear();
		newcells.append(index);
	}
	void nextDistance() {
		forwardCellCount = 0;
		newcells.swap(oldcells);
		newcells.clear();
		for (int i = 0; i < 4; i++) {
			spreadcells.clear();
		}
	}
	void prepareSpreading() {
		forwardCellCount = newcells.length();
	}
};

int calcDistance(Vector3d v1, Vector3d v2) {
	Vector3d v = v1 - v2;
	if (v.x < 0)
		v.x = -v.x;
	if (v.y < 0)
		v.y = -v.y;
	if (v.z < 0)
		v.z = -v.z;
	return v.x + v.y + v.z;
}

struct VolumeVisitor {
	World * world;
	VolumeData & volume;
	CellVisitor * visitor;
	Position & position;
	DirectionHelper helpers[6];
	DirEx direction; // camera forward direction
	DirEx oppdirection; // opposite direction
	Vector3d dirvector;
	int distance;
	VolumeVisitor(World * w, Position & pos, VolumeData & data, CellVisitor * v) : world(w), volume(data), visitor(v), position(pos) {
		direction = (DirEx)pos.direction.dir;
		oppdirection = (DirEx)(direction ^ 1);
		dirvector = pos.direction.forward;
	}
	~VolumeVisitor() {
	}
	bool visitCell(int index, cell_t cell) {
		if (cell == BOUND_SKY || cell >= VISITED_OCCUPIED)
			return false;
		if (BLOCK_TYPE_VISIBLE[cell]) {
			// call visitor callback
			Vector3d pt = volume.indexToPoint(index);
			//int threshold = (distance * 15 / 16);
			int viewAngleCosMod = pt * dirvector;
			if (viewAngleCosMod >= 1) { //threshold
				Vector3d pos = pt + position.pos;
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
				visitor->visit(world, position, pos, cell, visibleFaces);
			}
		}

		// mark as visited
		volume.put(index, BLOCK_TYPE_CAN_PASS[cell] ? VISITED_CELL : VISITED_OCCUPIED);
		return BLOCK_TYPE_CAN_PASS[cell];
	}
	void appendNewCell(int index, int distance) {
		Vector3d pos = volume.indexToPoint(index);
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

	void visitPlaneForward(int startIndex, DirEx direction) {
		DirectionHelper & helper = helpers[direction];
		cell_t * data = volume.ptr();
		int * thisPlaneDirections = volume.thisPlaneDirections(direction);
		int * nextPlaneDirections = volume.nextPlaneDirections(direction);
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
	void visitPlaneSpread(int startIndex, DirEx direction) {
		DirectionHelper & helper = helpers[direction];
		cell_t * data = volume.ptr();
		int * thisPlaneDirections = volume.thisPlaneDirections(direction);
		int * nextPlaneDirections = volume.nextPlaneDirections(direction);
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

	void visitAll() {
		lUInt64 startTs = GetCurrentTimeMillis();
		//CRLog::trace("VolumeVisitor2::visitAll() enter");
		int startIndex = volume.getIndex(Vector3d());
		cell_t cell = volume.get(startIndex);
		volume.put(startIndex, VISITED_CELL);
		for (int i = 0; i < 6; i++)
			helpers[i].start(startIndex, (DirEx)i);
		for (distance = 0; distance < volume.size() - 2; distance++) {
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
};



void World::visitVisibleCellsAllDirectionsFast(Position & position, CellVisitor * visitor) {
	volumeSnapshotInvalid = true;
	updateVolumeSnapshot();

	VolumeVisitor visitorHelper(this, position, volumeSnapshot, visitor);
	visitorHelper.visitAll();
}

void disposeChunkStripe(ChunkStripe * p) {
	delete p;
}

void World::updateVolumeSnapshot() {
	if (!volumeSnapshotInvalid && volumePos == camPosition.pos)
		return;
	volumePos = camPosition.pos;
	getCellsNear(camPosition.pos, volumeSnapshot);
	volumeSnapshotInvalid = false;
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

