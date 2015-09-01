#include "world.h"
#include <stdio.h>
#include <assert.h>

#include <time.h>
#ifdef LINUX
#include <sys/time.h>
#if !defined(__APPLE__)
#include <malloc.h>
#endif
#endif
#ifdef __APPLE__
#include <sys/time.h>
#endif

#if !defined(__SYMBIAN32__) && defined(_WIN32)
extern "C" {
#include <windows.h>
}
#endif

#ifdef _WIN32
static bool __timerInitialized = false;
static double __timeTicksPerMillis;
static lUInt64 __timeStart;
static lUInt64 __timeAbsolute;
static lUInt64 __startTimeMillis;
#endif

void CRReinitTimer();

void CRReinitTimer() {
#ifdef _WIN32
	LARGE_INTEGER tps;
	QueryPerformanceFrequency(&tps);
	__timeTicksPerMillis = (double)(tps.QuadPart / 1000L);
	LARGE_INTEGER queryTime;
	QueryPerformanceCounter(&queryTime);
	__timeStart = (lUInt64)(queryTime.QuadPart / __timeTicksPerMillis);
	__timerInitialized = true;
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	__startTimeMillis = (ft.dwLowDateTime | (((lUInt64)ft.dwHighDateTime) << 32)) / 10000;
#else
	// do nothing. it's for win32 only
#endif
}


lUInt64 GetCurrentTimeMillis() {
#ifdef _WIN32
	if (!__timerInitialized) {
		CRReinitTimer();
		return __startTimeMillis;
	}
	else {
		LARGE_INTEGER queryTime;
		QueryPerformanceCounter(&queryTime);
		__timeAbsolute = (lUInt64)(queryTime.QuadPart / __timeTicksPerMillis);
		return __startTimeMillis + (lUInt64)(__timeAbsolute - __timeStart);
	}
#else
    timeval ts;
    gettimeofday(&ts, NULL);
    return ts.tv_sec * (lUInt64)1000 + ts.tv_usec / 1000;
#endif
}


cell_t World::getCell(int x, int y, int z) {
	y += CHUNK_DY / 2;
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
	y += CHUNK_DY / 2;
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
	FILE * log;
	VisitorHelper(World & w, Position & p, CellVisitor * v) : world(w), position(p), visitor(v) {
		log = fopen("visitor.log", "at");
	}
	~VisitorHelper() {
		fclose(log);
	}
	void newRange(int distance) {
		fprintf(log, "new range: %d\n", distance);
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
