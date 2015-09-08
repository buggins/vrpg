#include "world.h"
#include <stdio.h>
#include <assert.h>
#include "logger.h"

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

#if 0
struct VolumeVisitor {
	VolumeData & volume;
	int distance;
	Vector2dArray oldcells;
	Vector2dArray newcells;
	CellVisitor * visitor;
	VolumeVisitor(VolumeData & data, CellVisitor * v) : volume(data), visitor(v) {
	}
	~VolumeVisitor() {
	}
	void appendNextCell(int posIndex, int mask) {
		newcells.append(Vector2d(posIndex, mask));
	}
	void visitAll() {
		CRLog::trace("VolumeVisitor::visitAll() enter");
		int startIndex = volume.getIndex(Vector3d());
		appendNextCell(volume.moveIndex(startIndex, DIR_NORTH), MASK_EX_NORTH | MASK_EX_NORTH_EAST | MASK_EX_NORTH_WEST | MASK_EX_NORTH_UP | MASK_EX_NORTH_DOWN | MASK_EX_NORTH_EAST_UP | MASK_EX_NORTH_WEST_UP | MASK_EX_NORTH_EAST_DOWN | MASK_EX_NORTH_WEST_DOWN);
		appendNextCell(volume.moveIndex(startIndex, DIR_SOUTH), MASK_EX_SOUTH | MASK_EX_SOUTH_EAST | MASK_EX_SOUTH_WEST | MASK_EX_SOUTH_UP | MASK_EX_SOUTH_DOWN | MASK_EX_SOUTH_EAST_UP | MASK_EX_SOUTH_WEST_UP | MASK_EX_SOUTH_EAST_DOWN | MASK_EX_SOUTH_WEST_DOWN);
		appendNextCell(volume.moveIndex(startIndex, DIR_WEST), MASK_EX_WEST | MASK_EX_NORTH_WEST | MASK_EX_SOUTH_WEST | MASK_EX_WEST_UP | MASK_EX_WEST_DOWN | MASK_EX_NORTH_WEST_UP | MASK_EX_SOUTH_WEST_UP | MASK_EX_NORTH_WEST_DOWN | MASK_EX_SOUTH_WEST_DOWN);
		appendNextCell(volume.moveIndex(startIndex, DIR_EAST), MASK_EX_EAST | MASK_EX_NORTH_EAST | MASK_EX_SOUTH_EAST | MASK_EX_EAST_UP | MASK_EX_EAST_DOWN | MASK_EX_NORTH_EAST_UP | MASK_EX_SOUTH_EAST_UP | MASK_EX_NORTH_EAST_DOWN | MASK_EX_SOUTH_EAST_DOWN);
		appendNextCell(volume.moveIndex(startIndex, DIR_UP), MASK_EX_UP | MASK_EX_NORTH_UP | MASK_EX_SOUTH_UP | MASK_EX_WEST_UP | MASK_EX_EAST_UP | MASK_EX_NORTH_WEST_UP | MASK_EX_SOUTH_WEST_UP | MASK_EX_NORTH_EAST_UP | MASK_EX_SOUTH_EAST_UP);
		appendNextCell(volume.moveIndex(startIndex, DIR_DOWN), MASK_EX_DOWN | MASK_EX_NORTH_DOWN | MASK_EX_SOUTH_DOWN | MASK_EX_WEST_DOWN | MASK_EX_EAST_DOWN | MASK_EX_NORTH_WEST_DOWN | MASK_EX_SOUTH_WEST_DOWN | MASK_EX_NORTH_EAST_DOWN | MASK_EX_SOUTH_EAST_DOWN);

		cell_t nearCells[26];
		DirEx directions[26];
		for (distance = 1; distance < volume.size(); distance++) {
			newcells.swap(oldcells);
			newcells.clear();
			for (int stage = 0; stage < 2; stage++) {
				// stage 0 - only simple directions
				// stage 1 - allow diagonals
				for (int i = 0; i < oldcells.length(); i++) {
					Vector2d pt = oldcells[i];
					int index = pt.x;
					int mask = pt.y;

					if (stage == 0)
						mask = mask & 0x3F;
					if (!mask)
						continue;

					cell_t currentCell = volume.get(index);
					int emptyCellMask;
					int dirCount = volume.getNear(index, mask, nearCells, directions, emptyCellMask);
					for (int j = 0; j < dirCount; j++) {
						DirEx dir = directions[j];
						int newIndex = volume.moveIndex(index, dir);
						cell_t newCell = nearCells[dir];
						if (newCell != VISITED_CELL) {
							if (newCell) {
								// occupied cell
								if (dir < 6) {
									// main direction - visit visible face
								}
							}
							else {
								// empty cell
								bool reachableUsingMainDirectionSteps = (emptyCellMask & DIR_TO_MASK[dir]) != 0;
								if (reachableUsingMainDirectionSteps) {
									// new mask
									int newMask = mask & NEAR_DIRECTIONS[dir]; // exclude opposite dirs

									if ((newMask & ~emptyCellMask) != newMask) {

									}

									// limit directions in newMask
									newcells.append(Vector2d(newIndex, newMask));
									// mark as visited
									volume.put(newIndex, VISITED_CELL);
								}
							}
						}
					}
					//MASK_EX_NORTH | MASK_EX_SOUTH | MASK_EX_WEST | MASK_EX_EAST | MASK_EX_UP | MASK_EX_DOWN | MASK_EX_WEST_UP | MASK_EX_EAST_UP | MASK_EX_WEST_DOWN | MASK_EX_EAST_DOWN | MASK_EX_NORTH_WEST | MASK_EX_NORTH_EAST | MASK_EX_NORTH_UP | MASK_EX_NORTH_DOWN | MASK_EX_NORTH_WEST_UP | MASK_EX_NORTH_EAST_UP | MASK_EX_NORTH_WEST_DOWN | MASK_EX_NORTH_EAST_DOWN | MASK_EX_SOUTH_WEST | MASK_EX_SOUTH_EAST | MASK_EX_SOUTH_UP | MASK_EX_SOUTH_DOWN | MASK_EX_SOUTH_WEST_UP | MASK_EX_SOUTH_EAST_UP | MASK_EX_SOUTH_WEST_DOWN | MASK_EX_SOUTH_EAST_DOWN;
				}
			}
		}
		CRLog::trace("VolumeVisitor::visitAll() exit");
	}
};

#endif

//typedef Array<CellToVisit> CellToVisitArray;
typedef Array<lUInt64> CellToVisitArray;

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


static CellToVisit cells_to_visit[9];
struct VolumeVisitor {
	VolumeData & volume;
	CellToVisitArray oldcells;
	CellToVisitArray newcells;
	CellVisitor * visitor;
	VolumeVisitor(VolumeData & data, CellVisitor * v) : volume(data), visitor(v) {
		//for (int y = -4; y < 20; y++) {
		//	for (int z = -20; z < 20; z++) {
		//		for (int x = -20; x < 20; x++) {
		//			cell_t cell = data.get(Vector3d(x, y, z));
		//			if (cell)
		//				CRLog::trace("   %d at %d,%d,%d", cell, x, y, z);
		//		}
		//	}
		//}
	}
	~VolumeVisitor() {
	}
	void visitNear(int index, DirEx baseDir) {
		{
			Vector3d pt = volume.indexToPoint(index);
			//CRLog::trace("visitNear %d,%d,%d (%d) dir=%d", pt.x, pt.y, pt.z, index, baseDir);
		}
		volume.getNearCellsForDirection(index, baseDir, cells_to_visit);
		CellToVisit * cell = cells_to_visit;
		if (cell->cell == VISITED_OCCUPIED) {
			Vector3d pt = volume.indexToPoint(cell->index);
			//CRLog::trace("    cell %d,%d,%d (%d) is already visited", pt.x, pt.y, pt.z, cell->index);
			return;
		}
		if (cell->cell && cell->cell < VISITED_OCCUPIED) {
			Vector3d pt = volume.indexToPoint(cell->index);
			//CRLog::trace("   occupied cell %d at %d,%d,%d (%d) is already visited", cell->cell, pt.x, pt.y, pt.z, cell->index);
		}
		newcells.reserve(10);
		if (cell->cell < VISITED_OCCUPIED)
			newcells.appendNoCheck(cell->data);
		if (cell->cell < VISITED_OCCUPIED) {
			//Vector3d pt = volume.indexToPoint(cell->index);
			//CRLog::trace("    marking cell %d,%d,%d (%d) as visited", pt.x, pt.y, pt.z, cell->index);
			volume.put(cell->index, cell->cell ? VISITED_OCCUPIED : VISITED_CELL);
		}
		if (!cell->cell || cell->cell == VISITED_CELL) {
			for (int i = 0; i < 8; i++) {
				cell++;
				if (cell->cell < VISITED_OCCUPIED) {
					newcells.appendNoCheck(cell->data);
					//if (cell->cell != VISITED_CELL) {
						//Vector3d pt = volume.indexToPoint(cell->index);
						//CRLog::trace("    marking cell %d,%d,%d (%d) as visited *** %d", pt.x, pt.y, pt.z, cell->index, i + 1);
						volume.put(cell->index, cell->cell ? VISITED_OCCUPIED : VISITED_CELL);
					//}
				}
			}
		}
		/*
		int nextIndex = index + volume.directionExDelta[baseDir];
		cell_t cell = volume._data[nextIndex];
		if (cell == VISITED_CELL)
			return;
		CellToVisit currentCell(nextIndex, cell, baseDir);
		newcells.reserve(10);
		newcells.appendNoCheck(currentCell.data);
		volume.put(currentCell.index, VISITED_CELL);
		if (!currentCell.cell) {
			// empty
			const DirEx * dirs = NEAR_DIRECTIONS_FOR + 8 * baseDir;
			CellToVisit next;
			next.dir = baseDir;
			for (int i = 0; i < 8; i++) {
				//visit(currentCell.index, dirs[i], baseDir);
				next.index = currentCell.index + volume.directionExDelta[dirs[i]];
				next.cell = volume._data[next.index];
				if (next.cell != VISITED_CELL) {
					newcells.appendNoCheck(next.data);
					volume.put(index, VISITED_CELL);
				}
			}
		}
		*/
	}
	void visitAll() {
		lUInt64 startTs = GetCurrentTimeMillis();
		CRLog::trace("VolumeVisitor::visitAll() enter");
		int startIndex = volume.getIndex(Vector3d());
		cell_t cell = volume.get(startIndex);
		volume.put(startIndex, VISITED_CELL);
		visitNear(startIndex, DIR_NORTH);
		visitNear(startIndex, DIR_SOUTH);
		visitNear(startIndex, DIR_WEST);
		visitNear(startIndex, DIR_EAST);
		visitNear(startIndex, DIR_UP);
		visitNear(startIndex, DIR_DOWN);
		for (int distance = 2; distance < volume.size(); distance++) {
			//CRLog::trace("Range: %d  cells: %d", distance - 1, newcells.length());

			newcells.swap(oldcells);
			newcells.clear();
			for (int i = 0; i < oldcells.length(); i++) {
				CellToVisit currentCell = oldcells[i];
				//Vector3d pt = volume.indexToPoint(currentCell.index);
				//CRLog::trace("Visiting cell %d at %d,%d,%d  dir=%d  (index=%d)", currentCell.cell, pt.x, pt.y, pt.z, currentCell.dir, currentCell.index);
				if (currentCell.cell) {
					if (currentCell.cell == BOUND_BOTTOM || currentCell.cell == BOUND_SKY)
						continue;
					//visitor->visitFace()
					//Vector3d pt = volume.indexToPoint(currentCell.index);
					//CRLog::trace("Found occupied cell %d at %d,%d,%d  dir=%d  (index=%d)", currentCell.cell, pt.x, pt.y, pt.z, currentCell.dir, currentCell.index);
				} else {
					// empty
					visitNear(currentCell.index, (DirEx)currentCell.dir);
				}
			}
		}
		lUInt64 duration = GetCurrentTimeMillis() - startTs;
		CRLog::trace("VolumeVisitor::visitAll() exit, lookup took %lld millis", duration);
	}
};

void World::visitVisibleCellsAllDirectionsFast(Position & position, CellVisitor * visitor) {
	volumeSnapshotInvalid = true;
	updateVolumeSnapshot();
	VolumeVisitor visitorHelper(volumeSnapshot, visitor);
	visitorHelper.visitAll();
}

void World::visitVisibleCellsAllDirections(Position & position, CellVisitor * visitor) {
	Position p = position;
	visitor->newDirection(p);
	visitVisibleCells(p, visitor);
	p = position;
	p.turnLeft();
	p.forward();
	visitor->newDirection(p);
	if (getCell(p.pos) == 0)
		visitVisibleCells(p, visitor);
	p = position;
	p.turnRight();
	p.forward();
	visitor->newDirection(p);
	if (getCell(p.pos) == 0)
		visitVisibleCells(p, visitor);
	p = position;
	p.turnRight();
	p.turnRight();
	//p.forward();
	visitor->newDirection(p);
	if (getCell(p.pos) == 0)
		visitVisibleCells(p, visitor);
	p = position;
	p.turnUp();
	p.forward();
	visitor->newDirection(p);
	if (getCell(p.pos) == 0)
		visitVisibleCells(p, visitor);
	p = position;
	p.turnDown();
	p.forward();
	visitor->newDirection(p);
	if (getCell(p.pos) == 0)
		visitVisibleCells(p, visitor);
}

void World::visitVisibleCells(Position & position, CellVisitor * visitor, bool visitThisPosition) {
	Vector3d pos = position.pos;
	VisitorHelper helper(*this, position, visitor);
	helper.newcells.append(pos);
	for (int range = 1; range < maxVisibleRange; range++) {
		helper.newRange(range);
		for (int i = 0; i < helper.oldcells.length(); i++) {
			Vector3d pt = helper.oldcells[i];
			cell_t cell = getCell(pt);
			if (!cell)
				helper.visitVisibleFaces(pt);
			helper.planVisits(pt);
		}
	}
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
				buf.putLayer(v, layer->ptr(srcpos.x, srcpos.z), size.x, size.z, CHUNK_DX);
			}
		}
	}
}

void World::getCellsNear(Vector3d v, VolumeData & buf) {
	buf.clear();
	int sz = buf.size();
	int y0 = v.y;
	v.x -= sz;
	v.y -= sz;
	v.z -= sz;
	int dy = sz * 2;
	Vector3d endv = v;
	endv.x += sz * 2;
	endv.y += sz * 2;
	endv.z += sz * 2;
	if (v.y < 0) {
		dy -= -v.y;
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
					Vector3d(x - v.x, v.y, z - v.z),
					Vector3d(dx, dy, dz),
					buf);
			}
			x = nextx;
		}
		z = nextz;
	}
	if (minLayer != -1) {
		buf.fillLayer(minLayer - y0, BOUND_BOTTOM);
		buf.fillLayer(maxLayer - y0, BOUND_SKY);
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

