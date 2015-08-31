#include "world.h"
#include <stdio.h>
#include <assert.h>

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
		log = fopen("visitor.log", "wt");
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
		fprintf(log, "plan visits %d, %d, %d\n", pt.x, pt.y, pt.z);
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
			fprintf(log, "    planned visit %d,%d,%d\n", newpt.x, newpt.y, newpt.z);
			visited.set(planeCoord.x, planeCoord.y, true);
			return true;
		}
		return false;
	}
};

void World::visitVisibleCells(Position & position, CellVisitor * visitor) {
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
