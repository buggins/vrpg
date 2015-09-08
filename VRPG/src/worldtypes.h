#ifndef WORLDTYPES_H_INCLUDED
#define WORLDTYPES_H_INCLUDED

#include <stdlib.h>
#include <string.h>
#include "logger.h"

typedef unsigned char cell_t;
const cell_t NO_CELL = 0;
const cell_t END_OF_WORLD = 253;
const cell_t VISITED_CELL = 255;
const cell_t VISITED_OCCUPIED = 254;
const cell_t BOUND_BOTTOM = 253;
const cell_t BOUND_SKY = 252;

enum Dir {
	NORTH = 0,
	SOUTH,
	WEST,
	EAST,
	UP,
	DOWN,
};

/// Extended Dir simple Dir directions can be combined; first 6 items of DirEx match items of Dir - 26 directions (3*3*3-1) 
enum DirEx {
	DIR_NORTH = 0,
	DIR_SOUTH,
	DIR_WEST,
	DIR_EAST,
	DIR_UP,
	DIR_DOWN,
	DIR_WEST_UP,
	DIR_EAST_UP,
	DIR_WEST_DOWN,
	DIR_EAST_DOWN,
	DIR_NORTH_WEST,
	DIR_NORTH_EAST,
	DIR_NORTH_UP,
	DIR_NORTH_DOWN,
	DIR_NORTH_WEST_UP,
	DIR_NORTH_EAST_UP,
	DIR_NORTH_WEST_DOWN,
	DIR_NORTH_EAST_DOWN,
	DIR_SOUTH_WEST,
	DIR_SOUTH_EAST,
	DIR_SOUTH_UP,
	DIR_SOUTH_DOWN,
	DIR_SOUTH_WEST_UP,
	DIR_SOUTH_EAST_UP,
	DIR_SOUTH_WEST_DOWN,
	DIR_SOUTH_EAST_DOWN,
	DIR_MAX,
	DIR_MIN = DIR_NORTH,
};

// 26 direction masks based on Dir
enum DirMask {
	MASK_NORTH = (1 << NORTH),
	MASK_SOUTH = (1 << SOUTH),
	MASK_WEST = (1 << WEST),
	MASK_EAST = (1 << EAST),
	MASK_UP = (1 << UP),
	MASK_DOWN = (1 << DOWN),
	MASK_WEST_UP = (1 << WEST) | MASK_UP,
	MASK_EAST_UP = (1 << EAST) | MASK_UP,
	MASK_WEST_DOWN = (1 << WEST) | MASK_DOWN,
	MASK_EAST_DOWN = (1 << EAST) | MASK_DOWN,
	MASK_NORTH_WEST = MASK_NORTH | MASK_WEST,
	MASK_NORTH_EAST = MASK_NORTH | MASK_EAST,
	MASK_NORTH_UP = MASK_NORTH | MASK_UP,
	MASK_NORTH_DOWN = MASK_NORTH | MASK_DOWN,
	MASK_NORTH_WEST_UP = MASK_NORTH | MASK_WEST | MASK_UP,
	MASK_NORTH_EAST_UP = MASK_NORTH | MASK_EAST | MASK_UP,
	MASK_NORTH_WEST_DOWN = MASK_NORTH | MASK_WEST | MASK_DOWN,
	MASK_NORTH_EAST_DOWN = MASK_NORTH | MASK_EAST | MASK_DOWN,
	MASK_SOUTH_WEST = MASK_SOUTH | MASK_WEST,
	MASK_SOUTH_EAST = MASK_SOUTH | MASK_EAST,
	MASK_SOUTH_UP = MASK_SOUTH | MASK_UP,
	MASK_SOUTH_DOWN = MASK_SOUTH | MASK_DOWN,
	MASK_SOUTH_WEST_UP = MASK_SOUTH | MASK_WEST | MASK_UP,
	MASK_SOUTH_EAST_UP = MASK_SOUTH | MASK_EAST | MASK_UP,
	MASK_SOUTH_WEST_DOWN = MASK_SOUTH | MASK_WEST | MASK_DOWN,
	MASK_SOUTH_EAST_DOWN = MASK_SOUTH | MASK_EAST | MASK_DOWN,
};

/// 26 (3*3*3-1) 3d direction masks - combinatino of DirEx
enum DirMaskEx {
	MASK_EX_NORTH = (1 << DIR_NORTH),
	MASK_EX_SOUTH = (1 << DIR_SOUTH),
	MASK_EX_WEST = (1 << DIR_WEST),
	MASK_EX_EAST = (1 << DIR_EAST),
	MASK_EX_UP = (1 << DIR_UP),
	MASK_EX_DOWN = (1 << DIR_DOWN),
	MASK_EX_WEST_UP = (1 << DIR_WEST_UP),
	MASK_EX_EAST_UP = (1 << DIR_EAST_UP),
	MASK_EX_WEST_DOWN = (1 << DIR_WEST_DOWN),
	MASK_EX_EAST_DOWN = (1 << DIR_EAST_DOWN),
	MASK_EX_NORTH_WEST = (1 << DIR_NORTH_WEST),
	MASK_EX_NORTH_EAST = (1 << DIR_NORTH_EAST),
	MASK_EX_NORTH_UP = (1 << DIR_NORTH_UP),
	MASK_EX_NORTH_DOWN = (1 << DIR_NORTH_DOWN),
	MASK_EX_NORTH_WEST_UP = (1 << DIR_NORTH_WEST_UP),
	MASK_EX_NORTH_EAST_UP = (1 << DIR_NORTH_EAST_UP),
	MASK_EX_NORTH_WEST_DOWN = (1 << DIR_NORTH_WEST_DOWN),
	MASK_EX_NORTH_EAST_DOWN = (1 << DIR_NORTH_EAST_DOWN),
	MASK_EX_SOUTH_WEST = (1 << DIR_SOUTH_WEST),
	MASK_EX_SOUTH_EAST = (1 << DIR_SOUTH_EAST),
	MASK_EX_SOUTH_UP = (1 << DIR_SOUTH_UP),
	MASK_EX_SOUTH_DOWN = (1 << DIR_SOUTH_DOWN),
	MASK_EX_SOUTH_WEST_UP = (1 << DIR_SOUTH_WEST_UP),
	MASK_EX_SOUTH_EAST_UP = (1 << DIR_SOUTH_EAST_UP),
	MASK_EX_SOUTH_WEST_DOWN = (1 << DIR_SOUTH_WEST_DOWN),
	MASK_EX_SOUTH_EAST_DOWN = (1 << DIR_SOUTH_EAST_DOWN),
	MASK_EX_ALL = (1<<26) - 1,
};

// DirEx to DirMask
const DirMask DIR_TO_MASK[] = {
	MASK_NORTH,
	MASK_SOUTH,
	MASK_WEST,
	MASK_EAST,
	MASK_UP,
	MASK_DOWN,
	MASK_WEST_UP,
	MASK_EAST_UP,
	MASK_WEST_DOWN,
	MASK_EAST_DOWN,
	MASK_NORTH_WEST,
	MASK_NORTH_EAST,
	MASK_NORTH_UP,
	MASK_NORTH_DOWN,
	MASK_NORTH_WEST_UP,
	MASK_NORTH_EAST_UP,
	MASK_NORTH_WEST_DOWN,
	MASK_NORTH_EAST_DOWN,
	MASK_SOUTH_WEST,
	MASK_SOUTH_EAST,
	MASK_SOUTH_UP,
	MASK_SOUTH_DOWN,
	MASK_SOUTH_WEST_UP,
	MASK_SOUTH_EAST_UP,
	MASK_SOUTH_WEST_DOWN,
	MASK_SOUTH_EAST_DOWN
};

// DirEx to near direction mask
const int NEAR_DIRECTIONS[] = {
	MASK_EX_NORTH | MASK_EX_NORTH_EAST | MASK_EX_NORTH_WEST | MASK_EX_NORTH_UP | MASK_EX_NORTH_DOWN | MASK_EX_NORTH_EAST_UP | MASK_EX_NORTH_WEST_UP | MASK_EX_NORTH_EAST_DOWN | MASK_EX_NORTH_WEST_DOWN,
	MASK_EX_SOUTH | MASK_EX_SOUTH_EAST | MASK_EX_SOUTH_WEST | MASK_EX_SOUTH_UP | MASK_EX_SOUTH_DOWN | MASK_EX_SOUTH_EAST_UP | MASK_EX_SOUTH_WEST_UP | MASK_EX_SOUTH_EAST_DOWN | MASK_EX_SOUTH_WEST_DOWN,
	MASK_EX_WEST | MASK_EX_NORTH_WEST | MASK_EX_SOUTH_WEST | MASK_EX_WEST_UP | MASK_EX_WEST_DOWN | MASK_EX_NORTH_WEST_UP | MASK_EX_SOUTH_WEST_UP | MASK_EX_NORTH_WEST_DOWN | MASK_EX_SOUTH_WEST_DOWN,
	MASK_EX_EAST | MASK_EX_NORTH_EAST | MASK_EX_SOUTH_EAST | MASK_EX_EAST_UP | MASK_EX_EAST_DOWN | MASK_EX_NORTH_EAST_UP | MASK_EX_SOUTH_EAST_UP | MASK_EX_NORTH_EAST_DOWN | MASK_EX_SOUTH_EAST_DOWN,
	MASK_EX_UP | MASK_EX_NORTH_UP | MASK_EX_SOUTH_UP | MASK_EX_WEST_UP | MASK_EX_EAST_UP | MASK_EX_NORTH_WEST_UP | MASK_EX_SOUTH_WEST_UP | MASK_EX_NORTH_EAST_UP | MASK_EX_SOUTH_EAST_UP,
	MASK_EX_DOWN | MASK_EX_NORTH_DOWN | MASK_EX_SOUTH_DOWN | MASK_EX_WEST_DOWN | MASK_EX_EAST_DOWN | MASK_EX_NORTH_WEST_DOWN | MASK_EX_SOUTH_WEST_DOWN | MASK_EX_NORTH_EAST_DOWN | MASK_EX_SOUTH_EAST_DOWN,
	MASK_EX_WEST_UP,
	MASK_EX_EAST_UP,
	MASK_EX_WEST_DOWN,
	MASK_EX_EAST_DOWN,
	MASK_EX_NORTH_WEST,
	MASK_EX_NORTH_EAST,
	MASK_EX_NORTH_UP,
	MASK_EX_NORTH_DOWN,
	MASK_EX_NORTH_WEST_UP,
	MASK_EX_NORTH_EAST_UP,
	MASK_EX_NORTH_WEST_DOWN,
	MASK_EX_NORTH_EAST_DOWN,
	MASK_EX_SOUTH_WEST,
	MASK_EX_SOUTH_EAST,
	MASK_EX_SOUTH_UP,
	MASK_EX_SOUTH_DOWN,
	MASK_EX_SOUTH_WEST_UP,
	MASK_EX_SOUTH_EAST_UP,
	MASK_EX_SOUTH_WEST_DOWN,
	MASK_EX_SOUTH_EAST_DOWN
};

template<typename T, T initValue> struct SymmetricMatrix {
private:
	int _size;
	int dx;
	int dx2;
	T * data;
public:
	SymmetricMatrix(int sz = 1) : _size(sz), data(NULL) {
		reset(sz);
	}
	~SymmetricMatrix() {
		if (data)
			delete[] data;
	}
	T get(int x, int y) {
		return data[(x + dx2) * dx + (y + dx2)];
	}
	void set(int x, int y, T value) {
		data[(x + dx2) * dx + (y + dx2)] = value;
	}
	int size() {
		return _size;
	}
	void reset(int sz) {
		if (_size != sz || !data) {
			_size = sz;
			dx = _size + _size - 1;
			dx2 = dx / 2;
			if (data)
				delete[] data;
			data = new T[dx * dx];
		}
		for (int i = dx * dx - 1; i >= 0; i--)
			data[i] = initValue;
	}
};

typedef SymmetricMatrix<bool, false> BoolSymmetricMatrix;

struct Vector2d {
	int x;
	int y;
	Vector2d() : x(0), y(0) {
	}
	Vector2d(int xx, int yy) : x(xx), y(yy) {
	}
	bool operator == (Vector2d v) const {
		return x == v.x && y == v.y;
	}
};
const Vector2d ZERO2 = Vector2d(0, 0);

struct Vector3d {
	int x;
	int y;
	int z;
	Vector3d() : x(0), y(0), z(0) {
	}
	Vector3d(int xx, int yy, int zz) : x(xx), y(yy), z(zz) {
	}
	bool operator == (Vector3d v) const {
		return x == v.x && y == v.y && z == v.z;
	}
	Vector3d operator - () const {
		return Vector3d(-x, -y, -z);
	}
	Vector3d operator + (Vector3d v) const {
		return Vector3d(x + v.x, y + v.y, z + v.z);
	}
	Vector3d & operator += (Vector3d v) {
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}
	Vector3d operator - (Vector3d v) const {
		return Vector3d(x - v.x, y - v.y, z - v.z);
	}
	Vector3d & operator -= (Vector3d v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}
	int operator * (Vector3d v) const {
		return x*v.x + y*v.y + z*v.z;
	}
	Vector3d operator * (int n) const {
		return Vector3d(x * n, y * n, z * n);
	}
	Vector3d & operator *= (int n) {
		x *= n;
		y *= n;
		z *= n;
		return *this;
	}
	Vector3d turnLeft() {
		return Vector3d(z, y, -x);
	}
	Vector3d turnRight() {
		return Vector3d(-z, y, x);
	}
	Vector3d turnUp() {
		return Vector3d(x, -z, y);
	}
	Vector3d turnDown() {
		return Vector3d(x, z, -y);
	}
	Vector3d move(DirEx dir) {
		Vector3d res = *this;
		switch (dir) {
		case DIR_NORTH:
			res.z--;
			break;
		case DIR_SOUTH:
			res.z++;
			break;
		case DIR_WEST:
			res.x--;
			break;
		case DIR_EAST:
			res.x++;
			break;
		case DIR_UP:
			res.y++;
			break;
		case DIR_DOWN:
			res.y--;
			break;
		default:
			break;
		}
		return res;
	}
};
const Vector3d ZERO3 = Vector3d(0, 0, 0);

template<typename T> struct Array {
private:
	int _size;
	int _length;
	T * _data;
	const T initValue = T();
public:
	Array() : _size(0), _length(0), _data(NULL) {

	}
	T * ptr(int index = 0) {
		return _data + index;
	}
	void swap(Array & v) {
		int tmp;
		tmp = _size; _size = v._size; v._size = tmp;
		tmp = _length; _length = v._length; v._length = tmp;
		T * ptmp;
		ptmp = _data; _data = v._data; v._data = ptmp;
	}
	/// ensure capacity is enough to fit sz items
	void reserve(int sz) {
		sz += _length;
		if (_size < sz) {
			int newsize = 1024;
			while (newsize < sz)
				newsize <<= 1;
			_data = (T*)realloc(_data, sizeof(T) * newsize);
			for (int i = _size; i < newsize; i++)
				_data[i] = initValue;
			_size = newsize;
		}
	}
	int length() {
		return _length;
	}
	void append(const T & value) {
		if (_length >= _size)
			reserve(_size == 0 ? 64 : _size * 2 - _length);
		_data[_length++] = value;
	}
	inline void appendNoCheck(const T & value) {
		_data[_length++] = value;
	}
	T* append(const T & value, int count) {
		int startLen = _length;
		for (int i = 0; i < count; i++)
			append(value);
		return _data + startLen;
	}
	void clear() {
		_length = 0;
	}
	T get(int index) {
		return _data[index];
	}
	void set(int index, T value) {
		_data[index] = value;
	}
	T & operator[] (int index) {
		return _data[index];
	}
	~Array() {
		if (_data)
			free(_data);
	}
};

typedef Array<float> FloatArray;
typedef Array<int> IntArray;
typedef Array<Vector2d> Vector2dArray;
typedef Array<Vector3d> Vector3dArray;

template<typename T, T initValue, void(*disposeFunction)(T value) > struct InfiniteArray {
private:
	T * data;
	int size;
	int minIdx;
	int maxIdx;
	void resize(int sz) {
		if (sz < 128)
			sz = 128;
		else
			sz = sz * 2;
		if (size < sz) {
			data = (T*)realloc(data, sizeof(T) * sz);
			for (int i = size; i < sz; i++)
				data[i] = initValue;
			size = sz;
		}
	}
public:
	int minIndex() {
		return minIdx;
	}
	int maxIndex() {
		return maxIdx;
	}
	void set(int index, T value) {
		int idx = index < 0 ? (-index) * 2 - 1 : index * 2;
		resize(idx + 1);
		T oldData = data[idx];
		if (oldData != initValue)
			disposeFunction(oldData);
		data[idx] = value;
		if (minIdx > index)
			minIdx = index;
		if (maxIdx < index + 1)
			maxIdx = index + 1;
	}
	T get(int index) {
		if (index < minIdx || index >= maxIdx)
			return initValue;
		int idx = index < 0 ? (-index) * 2 - 1 : index * 2;
		return data[idx];
	}
	InfiniteArray() : data(NULL), size(0), minIdx(0), maxIdx(0) {
	}
	~InfiniteArray() {
		if (data) {
			for (int i = 0; i < size; i++) {
				if (data[i] != initValue)
					disposeFunction(data[i]);
			}
			free(data);
		}
		data = NULL;
		size = 0;
	}

};

/// returns opposite direction to specified direction
inline Dir opposite(Dir d) {
	return (Dir)(d ^ 1);
}

inline Dir turnLeft(Dir d) {
	switch (d) {
	case WEST:
		return SOUTH;
	case EAST:
		return NORTH;
	default:
	case NORTH:
		return WEST;
	case SOUTH:
		return EAST;
	case UP:
		return SOUTH;
	case DOWN:
		return NORTH;
	}
}

inline Dir turnRight(Dir d) {
	switch (d) {
	case WEST:
		return NORTH;
	case EAST:
		return SOUTH;
	default:
	case NORTH:
		return EAST;
	case SOUTH:
		return WEST;
	case UP:
		return NORTH;
	case DOWN:
		return SOUTH;
	}
}

inline Dir turnUp(Dir d) {
	switch (d) {
	case WEST:
		return UP;
	case EAST:
		return UP;
	default:
	case NORTH:
		return UP;
	case SOUTH:
		return UP;
	case UP:
		return SOUTH;
	case DOWN:
		return NORTH;
	}
}

inline Dir turnDown(Dir d) {
	switch (d) {
	case WEST:
		return DOWN;
	case EAST:
		return DOWN;
	default:
	case NORTH:
		return DOWN;
	case SOUTH:
		return DOWN;
	case UP:
		return NORTH;
	case DOWN:
		return SOUTH;
	}
}


struct Direction {
	Direction(int x, int y, int z) {
		set(x, y, z);
	}
	Direction(Vector3d v) {
		set(v);
	}
	Direction(Dir d) {
		set(d);
	}
	Direction() {
		set(0, 0, -1);
	}
	/// set by direction code
	void set(Dir d);
	/// set by vector
	void set(int x, int y, int z);
	/// set by vector
	void set(Vector3d v) { set(v.x, v.y, v.z); }

	void turnLeft() {
		set(::turnLeft(dir));
	}
	void turnRight() {
		set(::turnRight(dir));
	}
	void turnUp() {
		set(::turnUp(dir));
	}
	void turnDown() {
		set(::turnDown(dir));
	}

	Dir dir;
	Vector3d forward;
	Vector3d up;
	Vector3d right;
	Vector3d left;
	Vector3d down;
	Vector3d forwardUp;
	Vector3d forwardDown;
	Vector3d forwardLeft;
	Vector3d forwardLeftUp;
	Vector3d forwardLeftDown;
	Vector3d forwardRight;
	Vector3d forwardRightUp;
	Vector3d forwardRightDown;
};

struct Position {
	Vector3d pos;
	Direction direction;
	Position() {

	}
	Position(Position & p) : pos(p.pos), direction(p.direction) {

	}
	Position(Vector3d position, Vector3d dir) : pos(position), direction(dir) {

	}
	Vector2d calcPlaneCoords(Vector3d v) {
		v = v - pos;
		switch (direction.dir) {
		default:
		case NORTH:
			return Vector2d(v.x, v.y);
		case SOUTH:
			return Vector2d(-v.x, v.y);
		case EAST:
			return Vector2d(v.z, v.y);
		case WEST:
			return Vector2d(-v.z, v.y);
		case UP:
			return Vector2d(-v.z, v.x);
		case DOWN:
			return Vector2d(v.z, v.x);
		}
	}
	void turnLeft() {
		direction.turnLeft();
	}
	void turnRight() {
		direction.turnRight();
	}
	void turnUp() {
		direction.turnUp();
	}
	void turnDown() {
		direction.turnDown();
	}
	void forward(int step = 1) {
		pos += direction.forward * step;
	}
	void backward(int step = 1) {
		pos -= direction.forward * step;
	}
};

#pragma pack(push)
#pragma pack(1)
struct CellToVisit {
	union {
		struct {
			int index;
			cell_t cell;
			unsigned char dir;
		};
		lUInt64 data;
	};
	CellToVisit() : data(0) {}
	CellToVisit(int idx, cell_t cellValue, DirEx direction) : index(idx), cell(cellValue), dir(direction) {}
	CellToVisit(const CellToVisit & v) : data(v.data) {}
	CellToVisit(lUInt64 v) : data(v) {}
	inline CellToVisit& operator = (CellToVisit v) {
		data = v.data;
		return *this;
	}
	inline CellToVisit& operator = (lUInt64 v) {
		data = v;
		return *this;
	}
};
#pragma pack(pop)

struct VolumeData {
	int MAX_DIST_BITS;
	int ROW_BITS;
	int MAX_DIST;
	int ROW_SIZE;
	int DATA_SIZE;
	int ROW_MASK;
	cell_t * _data;
	int directionDelta[64];
	int directionExDelta[26];
	int mainDirectionDeltas[6][9];
	VolumeData(int distBits);
	~VolumeData() {
		delete[] _data;
	}
	int size() { return MAX_DIST; }
	void clear() {
		memset(_data, 0, sizeof(cell_t) * DATA_SIZE);
	}

	/// put cell w/o bounds checking, (0,0,0) is center of array
	inline void put(Vector3d v, cell_t cell) {
		_data[((v.y + MAX_DIST) << (ROW_BITS * 2)) | ((v.z + MAX_DIST) << ROW_BITS) | (v.x + MAX_DIST)] = cell;
	}

	/// v is zero based destination coordinates
	void putLayer(Vector3d v, cell_t * layer, int dx, int dz, int stripe);

	/// put cell w/o bounds checking
	inline void put(int index, cell_t cell) {
		_data[index] = cell;
	}

	/// read w/o bounds checking, (0,0,0) is center of array
	inline cell_t get(Vector3d v) {
		return _data[((v.y + MAX_DIST) << (ROW_BITS * 2)) | ((v.z + MAX_DIST) << ROW_BITS) | (v.x + MAX_DIST)];
	}

	inline cell_t get(int index) {
		return _data[index];
	}

	/// get array index for point - (0,0,0) is center
	inline int getIndex(Vector3d v) {
		return ((v.y + MAX_DIST) << (ROW_BITS * 2)) | ((v.z + MAX_DIST) << ROW_BITS) | (v.x + MAX_DIST);
	}

	inline Vector3d indexToPoint(int index) {
		return Vector3d((index & ROW_MASK) - MAX_DIST,
			((index >> (ROW_BITS * 2)) & ROW_MASK) - MAX_DIST,
			((index >> (ROW_BITS)) & ROW_MASK) - MAX_DIST);
	}

	inline int moveIndex(int oldIndex, DirMask direction) {
		return oldIndex + directionDelta[direction];
	}
	
	inline int moveIndex(int oldIndex, DirEx direction) {
		return oldIndex + directionExDelta[direction];
	}

	inline CellToVisit getNext(int index, DirEx direction, DirEx baseDir) {
		int nextIndex = index + directionExDelta[direction];
		return CellToVisit(nextIndex, _data[nextIndex], baseDir);
	}

	/// return number of found directions for passed flags, cells are returned using DirEx index
	int getNear(int index, int mask, cell_t cells[], DirEx dirs[], int & emptyCellMask);
	/// get all near cells for specified position
	cell_t getNearCells(int index, cell_t cells[]);

	void getNearCellsForDirection(int index, DirEx direction, CellToVisit cells[9]);

	void fillLayer(int y, cell_t cell);
};

#endif// WORLDTYPES_H_INCLUDED

