#include "world.h"
#include <stdio.h>
#include <assert.h>
#include "logger.h"

bool World::isOpaque(Vector3d v) {
	cell_t cell = getCell(v);
	return cell > 0 && cell != BOUND_SKY;
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

//typedef Array<CellToVisit> CellToVisitArray;
typedef Array<lUInt64> CellToVisitArray;

//static DirEx NEAR_DIRECTIONS_FOR[6 * 8] = {
//	// NORTH
//	DIR_EAST, DIR_WEST, DIR_UP, DIR_DOWN, DIR_EAST_UP, DIR_WEST_UP, DIR_EAST_DOWN, DIR_WEST_DOWN,
//	// SOUTH
//	DIR_EAST, DIR_WEST, DIR_UP, DIR_DOWN, DIR_EAST_UP, DIR_WEST_UP, DIR_EAST_DOWN, DIR_WEST_DOWN,
//	// WEST
//	DIR_NORTH, DIR_SOUTH, DIR_UP, DIR_DOWN, DIR_NORTH_UP, DIR_SOUTH_UP, DIR_NORTH_DOWN, DIR_SOUTH_DOWN,
//	// EAST
//	DIR_NORTH, DIR_SOUTH, DIR_UP, DIR_DOWN, DIR_NORTH_UP, DIR_SOUTH_UP, DIR_NORTH_DOWN, DIR_SOUTH_DOWN,
//	// UP
//	DIR_NORTH, DIR_SOUTH, DIR_EAST, DIR_WEST, DIR_NORTH_EAST, DIR_SOUTH_EAST, DIR_NORTH_WEST, DIR_SOUTH_WEST,
//	// DOWN
//	DIR_NORTH, DIR_SOUTH, DIR_EAST, DIR_WEST, DIR_NORTH_EAST, DIR_SOUTH_EAST, DIR_NORTH_WEST, DIR_SOUTH_WEST,
//};


static CellToVisit cells_to_visit[9];
static CellToVisit cells_to_visit_no_forward[9];
struct VolumeVisitor {
	World * world;
	VolumeData & volume;
	CellToVisitArray oldcells;
	CellToVisitArray newcells;
	CellVisitor * visitor;
	Position & position;
	VolumeVisitor(World * w, Position & pos, VolumeData & data, CellVisitor * v) : world(w), volume(data), visitor(v), position(pos) {
		//for (int y = -4; y < 20; y++) {
		//	for (int z = -60; z < 60; z++) {
		//		for (int x = -60; x < 60; x++) {
		//			cell_t cell = data.get(Vector3d(x, y, z));
		//			if (cell && cell < BOUND_SKY)
		//				CRLog::trace("   %d at %d,%d,%d", cell, x, y, z);
		//		}
		//	}
		//}
	}
	~VolumeVisitor() {
	}
	void visitNear(int index, DirEx baseDir) {
		//{
			//Vector3d pt = volume.indexToPoint(index);
			//CRLog::trace("visitNear %d,%d,%d (%d) dir=%d", pt.x, pt.y, pt.z, index, baseDir);
		//}
        
        cell_t thisDirectionVisitedEmpty = VISITED_EMPTY_START + baseDir;
        
		volume.getNearCellsForDirection(index, baseDir, cells_to_visit);
		CellToVisit * cell = cells_to_visit;
		//if (cell->cell && cell->cell < VISITED_OCCUPIED) {
			//Vector3d pt = volume.indexToPoint(cell->index);
			//CRLog::trace("   occupied cell %d at %d,%d,%d (%d) is already visited", cell->cell, pt.x, pt.y, pt.z, cell->index);
		//}
		newcells.reserve(10);
        
		if (cell->cell < VISITED_OCCUPIED) {
			newcells.appendNoCheck(cell->data);
			//if (cell->cell && cell->cell < BOUND_SKY) {
			//	Vector3d pt = volume.indexToPoint(cell->index);
			//	CRLog::trace("    marking cell %d  %d,%d,%d (%d) as visited", cell->cell, pt.x, pt.y, pt.z, cell->index);
			//}
			volume.put(cell->index, cell->cell ? VISITED_OCCUPIED : thisDirectionVisitedEmpty);
		}
		if (!cell->cell || cell->cell >= VISITED_EMPTY_START) {
			for (int i = 0; i < 8; i++) {
				cell++;
                if (cell->cell >= VISITED_EMPTY_START && cell->cell != thisDirectionVisitedEmpty) {
                    // restore cell value
                    Vector3d pos = volume.indexToPoint(cell->index) + position.pos;
                    cell->cell = world->getCell(pos);
                    
                }
				if (cell->cell < VISITED_OCCUPIED) {
					newcells.appendNoCheck(cell->data);
					//if (cell->cell != VISITED_CELL) {
						//Vector3d pt = volume.indexToPoint(cell->index);
						//CRLog::trace("    marking cell %d,%d,%d (%d) as visited *** %d", pt.x, pt.y, pt.z, cell->index, i + 1);
						volume.put(cell->index, cell->cell ? VISITED_OCCUPIED : thisDirectionVisitedEmpty);
					//}
				}
			}
		} else  { //if (cell->cell == VISITED_OCCUPIED)
			//Vector3d pt = volume.indexToPoint(cell->index);
			//CRLog::trace("    cell %d,%d,%d (%d) is already visited", pt.x, pt.y, pt.z, cell->index);
			volume.getNearCellsForDirectionNoForward(index, baseDir, cells_to_visit_no_forward);
			CellToVisit * cell2 = cells_to_visit_no_forward;
			for (int i = 0; i < 8; i++) {
				cell++;
				cell2++;
				if (cell->cell == VISITED_OCCUPIED || cell->cell == thisDirectionVisitedEmpty)
					continue;
				bool hasPath = (cell2->cell >= VISITED_EMPTY_START || !cell2->cell);
				if (!hasPath && i >= 4) {
					CellToVisit * c1 = cells_to_visit + ((i + 1) & 3) + 1;
					CellToVisit * c2 = cells_to_visit + ((i) & 3) + 1;
					if (!c1->cell || c1->cell == VISITED_CELL)
						hasPath = true;
					else if(!c2->cell || c2->cell >= VISITED_EMPTY_START)
						hasPath = true;
					else {
						c1 = cells_to_visit_no_forward + ((i + 1) & 3) + 1;
						c2 = cells_to_visit_no_forward + ((i) & 3) + 1;
						if (!c1->cell || c1->cell >= VISITED_EMPTY_START)
							hasPath = true;
						else if (!c2->cell || c2->cell >= VISITED_EMPTY_START)
							hasPath = true;
					}
				}
				if (hasPath) {
                    if (cell->data >= VISITED_EMPTY_START) {
                        // restore value from world
                        Vector3d pos = volume.indexToPoint(cell->index) + position.pos;
                        cell->cell = world->getCell(pos);
                    }
					newcells.appendNoCheck(cell->data);
					volume.put(cell->index, cell->cell ? VISITED_OCCUPIED : thisDirectionVisitedEmpty);
				}
			}
			return;
		}
	}
	void visitAll() {
		lUInt64 startTs = GetCurrentTimeMillis();
		//CRLog::trace("VolumeVisitor::visitAll() enter");
		int startIndex = volume.getIndex(Vector3d());
		cell_t cell = volume.get(startIndex);
		volume.put(startIndex, VISITED_CELL);
		for (int i = 0; i < 6; i++)
			visitNear(startIndex, (DirEx)i);
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

					Vector3d pt = volume.indexToPoint(currentCell.index);
					Vector3d pos = pt + position.pos;

					//cell_t cellFromWorld = world->getCell(pos);
					//assert(cellFromWorld == currentCell.cell);

					int visibleFaces = 0;
					if (pt.y <= 0 &&
							!world->isOpaque(pos.move(DIR_UP)))
						visibleFaces |= MASK_UP;
					if (pt.y >= 0 && 
							!world->isOpaque(pos.move(DIR_DOWN)))
						visibleFaces |= MASK_DOWN;
					if (pt.x <= 0 && 
						!world->isOpaque(pos.move(DIR_EAST)))
						visibleFaces |= MASK_EAST;
					if (pt.x >= 0 && 
						!world->isOpaque(pos.move(DIR_WEST)))
						visibleFaces |= MASK_WEST;
					if (pt.z <= 0 && 
						!world->isOpaque(pos.move(DIR_SOUTH)))
						visibleFaces |= MASK_SOUTH;
					if (pt.z >= 0 && 
						!world->isOpaque(pos.move(DIR_NORTH)))
						visibleFaces |= MASK_NORTH;
					//CRLog::trace("Visiting cell %d at %d,%d,%d  faces=%02x", currentCell.cell, pos.x, pos.y, pos.z, visibleFaces);
					visitor->visit(world, position, pos, currentCell.cell, visibleFaces);

				} else {
					// empty
					visitNear(currentCell.index, (DirEx)currentCell.dir);
				}
			}
		}
		lUInt64 duration = GetCurrentTimeMillis() - startTs;
		//CRLog::trace("VolumeVisitor::visitAll() exit, lookup took %lld millis", duration);
	}
};


bool inline canPass(cell_t cell) { return !cell || cell == VISITED_CELL;  }

struct VolumeVisitor2 {
	World * world;
	VolumeData & volume;
	CellVisitor * visitor;
	Position & position;
	VolumeVisitor2(World * w, Position & pos, VolumeData & data, CellVisitor * v) : world(w), volume(data), visitor(v), position(pos) {
	}
	~VolumeVisitor2() {
	}
	void visitNear(int index, DirEx baseDir) {
	}
	void visitPlane(int startIndex, DirEx direction, int distance) {
		cell_t thisPlaneCells[9];
		cell_t nextPlaneCells[9];
		cell_t * data = volume.ptr();
		int * thisPlaneDirections = volume.thisPlaneDirections(direction);
		int * nextPlaneDirections = volume.nextPlaneDirections(direction);
		int offset = distance + 1;
		int planeSize = distance * 2 + 3;
		startIndex = startIndex + nextPlaneDirections[0] * distance + thisPlaneDirections[2] * offset + thisPlaneDirections[3] * offset;
		int rowIndex = startIndex;
		for (int row = 0; row < planeSize; row++) {
			int cellIndex = rowIndex; // cell index in old plane
			for (int col = 0; col < planeSize; col++) {
				// cellIndex is old plane index
				int forwardIndex = cellIndex + nextPlaneDirections[0]; // index in next plane
				cell_t cell = data[forwardIndex];
				if (cell < VISITED_OCCUPIED) {
					// new cell is not yet visited at all
					// check for path to new cell
					cell_t cellBefore = data[cellIndex];
					bool pathFound = cellBefore == VISITED_CELL; // direct path
					if (!pathFound) {
						// no direct path found, trying to bypass
						volume.getNearCellsForDirection(cellIndex, direction, nextPlaneCells);
						volume.getNearCellsForDirectionNoForward(cellIndex, direction, thisPlaneCells);
						if (thisPlaneCells[1] == VISITED_CELL) {
							pathFound =
								canPass(nextPlaneCells[1])
								|| (canPass(thisPlaneCells[5]) && canPass(nextPlaneCells[5]) && canPass(nextPlaneCells[2]))
								|| (canPass(thisPlaneCells[8]) && canPass(nextPlaneCells[8]) && canPass(nextPlaneCells[4]));
						}
						if (!pathFound && thisPlaneCells[2] == VISITED_CELL) {
							pathFound =
								canPass(nextPlaneCells[2])
								|| (canPass(thisPlaneCells[5]) && canPass(nextPlaneCells[5]) && canPass(nextPlaneCells[1]))
								|| (canPass(thisPlaneCells[6]) && canPass(nextPlaneCells[6]) && canPass(nextPlaneCells[3]));
						}
						if (!pathFound && thisPlaneCells[3] == VISITED_CELL) {
							pathFound =
								canPass(nextPlaneCells[3])
								|| (canPass(thisPlaneCells[6]) && canPass(nextPlaneCells[6]) && canPass(nextPlaneCells[2]))
								|| (canPass(thisPlaneCells[7]) && canPass(nextPlaneCells[7]) && canPass(nextPlaneCells[4]));
						}
						if (!pathFound && thisPlaneCells[4] == VISITED_CELL) {
							pathFound =
								canPass(nextPlaneCells[4])
								|| (canPass(thisPlaneCells[7]) && canPass(nextPlaneCells[7]) && canPass(nextPlaneCells[3]))
								|| (canPass(thisPlaneCells[8]) && canPass(nextPlaneCells[8]) && canPass(nextPlaneCells[4]));
						}
						if (!pathFound && thisPlaneCells[5] == VISITED_CELL) {
							pathFound = canPass(nextPlaneCells[5]) && (canPass(nextPlaneCells[1]) || canPass(nextPlaneCells[2]));
						}
						if (!pathFound && thisPlaneCells[6] == VISITED_CELL) {
							pathFound = canPass(nextPlaneCells[6]) && (canPass(nextPlaneCells[2]) || canPass(nextPlaneCells[3]));
						}
						if (!pathFound && thisPlaneCells[7] == VISITED_CELL) {
							pathFound = canPass(nextPlaneCells[7]) && (canPass(nextPlaneCells[3]) || canPass(nextPlaneCells[4]));
						}
						if (!pathFound && thisPlaneCells[8] == VISITED_CELL) {
							pathFound = canPass(nextPlaneCells[8]) && (canPass(nextPlaneCells[4]) || canPass(nextPlaneCells[1]));
						}
					}
					if (pathFound && cell != BOUND_SKY) {
						if (cell) {
							// call visitor callback
							Vector3d pt = volume.indexToPoint(forwardIndex);
							Vector3d pos = pt + position.pos;
							int visibleFaces = 0;
							if (pt.y <= 0 &&
								!world->isOpaque(pos.move(DIR_UP)))
								visibleFaces |= MASK_UP;
							if (pt.y >= 0 &&
								!world->isOpaque(pos.move(DIR_DOWN)))
								visibleFaces |= MASK_DOWN;
							if (pt.x <= 0 &&
								!world->isOpaque(pos.move(DIR_EAST)))
								visibleFaces |= MASK_EAST;
							if (pt.x >= 0 &&
								!world->isOpaque(pos.move(DIR_WEST)))
								visibleFaces |= MASK_WEST;
							if (pt.z <= 0 &&
								!world->isOpaque(pos.move(DIR_SOUTH)))
								visibleFaces |= MASK_SOUTH;
							if (pt.z >= 0 &&
								!world->isOpaque(pos.move(DIR_NORTH)))
								visibleFaces |= MASK_NORTH;
							visitor->visit(world, position, pos, cell, visibleFaces);
						}
						// mark as visited
						data[forwardIndex] = canPass(cell) ? VISITED_CELL : VISITED_OCCUPIED;
					}
				}
				cellIndex += thisPlaneDirections[1];
			}
			rowIndex += thisPlaneDirections[4];
		}
	}
	void visitAll() {
		lUInt64 startTs = GetCurrentTimeMillis();
		CRLog::trace("VolumeVisitor2::visitAll() enter");
		int startIndex = volume.getIndex(Vector3d());
		cell_t cell = volume.get(startIndex);
		volume.put(startIndex, VISITED_CELL);
		for (int distance = 0; distance < volume.size() - 2; distance++) {
			for (int dir = 0; dir < 6; dir++)
				visitPlane(startIndex, (DirEx)dir, distance);
		}
		lUInt64 duration = GetCurrentTimeMillis() - startTs;
		CRLog::trace("VolumeVisitor2::visitAll() exit, lookup took %lld millis", duration);
	}
};



void World::visitVisibleCellsAllDirectionsFast(Position & position, CellVisitor * visitor) {
	volumeSnapshotInvalid = true;
	updateVolumeSnapshot();
	VolumeVisitor2 visitorHelper(this, position, volumeSnapshot, visitor);
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

void World::getCellsNear(Vector3d v, VolumeData & buf) {
	buf.clear();
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

