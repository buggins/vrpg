#include "world.h"


struct VisitorHelper {
	World & world;
	Position & position;
	int distance;
	Vector3dArray oldcells;
	Vector3dArray newcells;
	BoolSymmetricMatrix visited;
	VisitorHelper(World & w, Position & p) : world(w), position(p) {

	}
	void newRange(int distance) {
		visited.reset(distance);
		oldcells.swap(newcells);
		newcells.clear();
	}
	void planVisits(Vector3d pt) {
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
		if (world.getCell(newpt) == NO_CELL && !visited.get(planeCoord.x, planeCoord.y)) {
			newcells.append(newpt);
			visited.set(planeCoord.x, planeCoord.y, true);
			return true;
		}
		return false;
	}
};

void World::visitVisibleCells(Position & position) {
	Vector3d pos = position.pos;
	VisitorHelper helper(*this, position);
	helper.newcells.append(pos);
	for (int range = 1; range < maxVisibleRange; range++) {
		helper.newRange(range);
		for (int i = 0; i < helper.oldcells.length(); i++) {
			Vector3d pt = helper.oldcells[i];
			helper.planVisits(pt);
		}
	}
}
