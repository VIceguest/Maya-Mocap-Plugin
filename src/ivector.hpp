////////////////////////////////////////////////////////////////////////////
//
//  MoCap File Importer
//  Created by Jiang
////////////////////////////////////////////////////////////////////////////

#ifndef __IVECTOR_HPP__
#define __IVECTOR_HPP__

// We are in namespace "imath" now
// DO NOT including any header file here

#define IVECTOR_PI 3.1415926535897932384626433832795

class badIndex {
	std::string msg;		// message of exception
public:
	badIndex() {};
	badIndex(std::string str) : msg(str) {}
};
////////////////////////////////////////////////////////////////////////////
// class for vectors with 3 elements
template <typename T>
class iVector {
public:
	T x, y, z;

	// constructor
	iVector() : x(0.0), y(0.0), z(0.0) { }

	iVector(const iVector<T> &v1) : x(v1.x), y(v1.y), z(v1.z) { }

	iVector(const T xx, const T yy, const T zz) : x(xx), y(yy), z(zz) { }

	iVector(const float (&c)[3]) : x(c[0]), y(c[1]), z(c[2]) { }

	inline void clear() { x = y = z = 0.0; }

	// set & get
	inline iVector<T>& operator=(const iVector<T> &v1) {
		x = v1.x; y = v1.y; z = v1.z;
		return *this;
	}

	inline iVector<T>& operator=(const T (&i)[3]) {
		x = i[0]; y = i[1]; z = i[2];
		return *this;
	}
	
	inline iVector<T>& set(const T &xx, const T &yy, const T &zz) {
		x = xx; y = yy; z = zz;
		return *this;
	}

	inline void get(T *xx, T *yy, T *zz) {
		*xx = x; *yy = y; *zz = z;
	}

	// return an array
	T *a() const {
		static T i[3];
		i[0] = x; i[1] = y; i[2] = z;
		return (T *)(&i);
	}
	
	// addition
	inline iVector<T> operator+(const iVector<T> &v1) const {
		iVector<T> v2(x + v1.x, y + v1.y, z + v1.z);
		return v2;
	}

	// substraction
	inline iVector<T> operator-(const iVector<T> &v1) const {
		return iVector<T>(x - v1.x, y - v1.y, z - v1.z);
	}

	// scaling with a parameter
	inline iVector<T> operator*(const T &a) const {
		return iVector<T> (x * a, y * a, z * a);
	}

	// dot product
	inline T operator*(const iVector<T> &v1) const {
		T i = x * v1.x + y * v1.y + z * v1.z;
		return i;
	}

	// cross product
	inline iVector<T> operator^(const iVector<T> &v1) const {
		iVector<T> v2(
			y * v1.z - z * v1.y,
			z * v1.x - x * v1.z,
			x * v1.y - y * v1.x
		);
		return v2;
	}

	// add a value
	inline iVector<T>& operator+=(const iVector<T> &v1) {
		x += v1.x; y += v1.y; z += v1.z;
		return *this;
	}

	// sub a value
	inline iVector<T>& operator-=(const iVector<T> &v1) {
		x -= v1.x; y -= v1.y; z -= v1.z;
		return *this;
	}

	// multiple a value
	inline iVector<T>& operator*=(const T &a) {
		x *= a; y *= a; z *= a;
		return *this;
	}

	// comparason
	inline bool operator==(const iVector<T> &v1) const {
		return (x == v1.x && y == v1.y && z == v1.z);
	}

	inline bool operator!=(const iVector<T> &v1) const {
		return !(*this == v1);
	}

	// normalization
	inline iVector<T>& normalize() {
		T factor = 1.0 / length();
		x *= factor; y *= factor; z *= factor;
		return *this;
	}

	// length
	inline T length() const {
		return static_cast<T>(sqrt(x * x + y * y + z * z));
	}

	// neg
	inline iVector<T> operator-() const {
		return iVector<T>(-x, -y, -z);
	}

	// convertion
	inline iVector<T>& toDegrees() {
		x = x * 180.0 / IVECTOR_PI;
		y = y * 180.0 / IVECTOR_PI;
		z = z * 180.0 / IVECTOR_PI;
		return *this;
	}

	inline iVector<T>& toRadians() {
		x = x * IVECTOR_PI / 180.0;
		y = y * IVECTOR_PI / 180.0;
		z = z * IVECTOR_PI / 180.0;
		return *this;
	}

	inline friend std::ostream& operator<<(std::ostream &out, iVector<T> &temp) {
		temp.output(out);
		return out;
	}

private:
	// overriding standard output operator
	inline void output(std::ostream &out) {
		out << "(" << x << ", " << y << ", " << z << ")";
	}
};


////////////////////////////////////////////////////////////////////////////
// class for vectors with 4 elements
template <typename T>
class iVector4 {
public:
	T x, y, z, w;
	// constructors
	iVector4() : x(0.0), y(0.0), z(0.0), w(0.0) { }

	iVector4(const iVector4<T> &v1) : x(v1.x), y(v1.y), z(v1.z), w(v1.w) { }

	iVector4(const T &xx, const T &yy, const T &zz, const T &ww) :
		x(xx), y(yy), z(zz), w(ww) { }

	iVector4(const float (&c)[4]) {
		x = c[0]; y = c[1]; z = c[2]; w = c[3];
	}

	inline void clear() {
		r = g = b = a = 0.0;
	}

	// assign a value
	inline iVector4<T>&operator=(const iVector4<T>&c) {
		x = c.x; y = c.y; z = c.z; w = c.w;
		return *this;
	}

	inline iVector4<T>&operator=(const T (&i)[4]) {
		x = i[0]; y = i[1]; z = i[2]; w = i[3];
		return *this;
	}

	inline iVector4<T>& set(const T &xx, const T &yy, const T &zz, const T &ww) {
		x = xx; y = yy; z = zz; w = ww;
		return *this;
	}

	inline void get(T *xx, T *yy, T *zz, T *ww) {
		*xx = x; *yy = y; *zz = z; *ww = w;
	}

	// return an array
	inline T *a() const {
		static T i[4];
		i[0] = x; i[1] = y; i[2] = z; i[3] = w;
		return (T *)(&i);
	}

	inline friend std::ostream& operator<<(std::ostream &out, iVector4<T> &temp) {
		temp.output(out);
		return out;
	}

private:
	// overriding standard output operator
	inline void output(std::ostream &out) {
		out << "(" << x << ", " << y << ", " << z ", " << w << ")";
	}
};

#endif // __IVECTOR_HPP__
