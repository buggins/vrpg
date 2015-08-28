#ifndef WORLDTYPES_H_INCLUDED
#define WORLDTYPES_H_INCLUDED
typedef unsigned char cell_t;

template<typename T, T initValue> struct SymmetricMatrix {
private:
	int _size;
	int dx;
	int dx2;
	T * data;
public:
	SymmetricMatrix(int sz = 0) : _size(sz), data(NULL) {
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
	void swap(Array & v) {
		int tmp;
		tmp = _size; _size = v._size; v._size = tmp;
		tmp = _length; _length = v._length; v._length = tmp;
		T * ptmp;
		ptmp = _data; _data = v._data; v._data = ptmp;
	}
	void reserve(int sz) {
		if (_size < sz) {
			_data = (T*)realloc(_data, sizeof(T)*sz);
		}
		for (int i = _size; i < sz; i++)
			_data[i] = initValue;
	}
	int length() {
		return _length;
	}
	void append(T value) {
		if (_length >= _size)
			reserve(_size == 0 ? 64 : _size * 2);
		_data[_length++] = value;
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

typedef Array<Vector2d> Vector2dArray;
typedef Array<Vector3d> Vector3dArray;

template<typename T, T initValue, void(*disposeFunction)(T value) > struct InfiniteArray {
private:
	T * data;
	int size;
	int minIdx;
	int maxIdx;
	void resize(int sz) {
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
		resize(idx);
		if (data[idx] != initValue)
			disposeFunction(data[idx]);
		data[idx] = value;
		if (minIdx > index)
			minIdx = index;
		if (maxIdx < index + 1)
			maxIdx = index;
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

enum Dir {
	WEST,
	EAST,
	NORTH,
	SOUTH,
	UP,
	DOWN,
};

struct Direction {
	Direction(int x, int y, int z) {
		set(x, y, z);
	}
	Direction(Dir d) {
		set(d);
	}
	Direction() {
		set(0, 0, -1);
	}
	void set(Dir d) {
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
	void set(int x, int y, int z) {
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
		up = forward.turnUp();
		right = forward.turnRight();
		left = forward.turnLeft();
		down = forward.turnDown();
		forwardUp = forward + up;
		forwardDown = forward + down;
		forwardLeft = forward + left;
		forwardLeftUp = forward + left + up;
		forwardLeftDown = forward + left + down;
		forwardRight = forward + right;
		forwardRightUp = forward + right + up;
		forwardRightDown = forward + right + down;
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
};


class CellVisitor {
public:
	virtual ~CellVisitor() {}
	virtual void visit(Position & camPosition, Vector3d pos) = 0;
};

#endif// WORLDTYPES_H_INCLUDED

