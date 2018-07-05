#pragma once

#include <cmath>

class Vec3
{
public:
	float x{}, y{}, z{};

	inline float& operator[](int i);
	inline float operator[](int i) const;
	inline const float* data() const;
};

class Vec4
{
public:
	float x{}, y{}, z{}, w{};

	inline float& operator[](int i);
	inline float operator[](int i) const;
	inline const float* data() const;
};

// column major, post multiply
// m[col][row]
// [x, y, z]
//  ^  ^  ^
//  c0 c1 c2
class Mat33
{
public:
	Vec3 cols[3]{};

	inline Vec3& operator[](int col);
	inline const Vec3& operator[](int col) const;
	inline const float* data() const;
};

// column major, post multiply
// m[col][row]
// [x, y, z, T]
//  ^  ^  ^  ^
//  c0 c1 c2 c3
class Mat44
{
public:
	Vec4 cols[4]{};

	inline Vec4& operator[](int col);
	inline const Vec4& operator[](int col) const;
	inline const float* data() const;
};

class Quat
{
public:
	float x{}, y{}, z{}, w{1.f};
};

class Pose
{
public:
	Vec3 pos;
	Quat rot;
};


// Vec3
inline Vec3 operator-(const Vec3& v);
inline Vec3 operator+(const Vec3& v0, const Vec3& v1);
inline Vec3 operator-(const Vec3& v0, const Vec3& v1);
inline Vec3 operator*(const Vec3& v, float s);
inline float length(const Vec3& v);
inline Vec3 normal(const Vec3& v);
inline Vec3 cross(const Vec3& v0, const Vec3& v1);
inline Vec4 to_vec4(const Vec3& v3, float w);

// Vec4
inline Vec4 operator+(const Vec4& v0, const Vec4& v1);
inline Vec4 operator*(const Vec4& v, float s);

// Mat33
inline Quat to_quat(const Mat33& m);

// Mat44
inline Vec4 operator*(const Mat44& m, const Vec4& v);
inline Mat44 operator*(const Mat44& m0, const Mat44& m1);

// Quat
inline Quat make_quat_angle_axis(float angle, const Vec3& axis);
inline Vec3 operator*(const Quat& quat, const Vec3& v);
inline Quat operator*(const Quat& quat, float s);
inline Quat operator*(const Quat& q0, const Quat& q1);
inline Quat operator/(const Quat& quat, float d);
inline float dot(const Quat& q0, const Quat& q1);
inline Quat conjugate(const Quat& quat);
inline Quat inverse(const Quat& quat);
inline Mat33 to_mat33(const Quat& quat);

// Pose
inline Pose inverse(const Pose& pose);
inline Mat44 to_mat44(const Pose& pose);


// Vec3 Impl

inline float& Vec3::operator[](int i)
{
	return reinterpret_cast<float*>(&x)[i];
}

inline float Vec3::operator[](int i) const
{
	return reinterpret_cast<const float*>(&x)[i];
}

inline const float* Vec3::data() const
{
	return reinterpret_cast<const float*>(&x);
}

inline Vec3 operator-(const Vec3& v)
{
	return v * -1.f;
}

inline Vec3 operator+(const Vec3& v0, const Vec3& v1)
{
	return { v0.x + v1.x, v0.y + v1.y, v0.z + v1.z };
}

inline Vec3 operator-(const Vec3& v0, const Vec3& v1)
{
	return { v0.x - v1.x, v0.y - v1.y, v0.z - v1.z };
}

inline Vec3 operator*(const Vec3& v, float s)
{
	return { v.x * s, v.y * s, v.z * s };
}

inline float length(const Vec3& v)
{
	return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline Vec3 normal(const Vec3& v)
{
	const auto inv_len = 1 / length(v);
	return v * inv_len;
}

inline Vec3 cross(const Vec3& v0, const Vec3& v1)
{
	return 
	{ 
		v0.y * v1.z - v0.z * v1.y,
		v0.z * v1.x - v0.x * v1.z,
		v0.x * v1.y - v0.y * v1.x 
	};
}

inline Vec4 to_vec4(const Vec3& v3, float w)
{
	return Vec4{ v3.x, v3.y, v3.z, w };
}


// Vec4 Impl

inline float& Vec4::operator[](int i)
{
	return reinterpret_cast<float*>(&x)[i];
}

inline float Vec4::operator[](int i) const
{
	return reinterpret_cast<const float*>(&x)[i];
}

inline const float* Vec4::data() const
{
	return reinterpret_cast<const float*>(&x);
}

inline Vec4 operator+(const Vec4& v0, const Vec4& v1)
{
	return { v0.x + v1.x, v0.y + v1.y, v0.z + v1.z, v0.w + v1.w };
}

inline Vec4 operator*(const Vec4& v, float s)
{
	return { v.x * s, v.y * s, v.z * s, v.w * s };
}


// Mat33 Impl

inline Vec3& Mat33::operator[](int col)
{
	return cols[col];
}

inline const Vec3& Mat33::operator[](int col) const
{
	return cols[col];
}

inline const float* Mat33::data() const
{
	return cols[0].data();
}


inline Quat to_quat(const Mat33& m)
{
	Quat q;
	float tr = m[0][0] + m[1][1] + m[2][2];

	if (tr > 0) 
	{ 
		float S = std::sqrt(tr+1.f) * 2; // S=4*qw 
		q.w = 0.25f * S;
		q.x = (m[1][2] - m[2][1]) / S;
		q.y = (m[2][0] - m[0][2]) / S; 
		q.z = (m[0][1] - m[1][0]) / S; 
	} 
	else if ((m[0][0] > m[1][1])&(m[0][0] > m[2][2])) 
	{ 
		float S = std::sqrt(1.f + m[0][0] - m[1][1] - m[2][2]) * 2; // S=4*qx 
		q.w = (m[1][2] - m[2][1]) / S;
		q.x = 0.25f * S;
		q.y = (m[1][0] + m[0][1]) / S; 
		q.z = (m[2][0] + m[0][2]) / S; 
	} 
	else if (m[1][1] > m[2][2]) 
	{ 
		float S = std::sqrt(1.f + m[1][1] - m[0][0] - m[2][2]) * 2; // S=4*qy
		q.w = (m[2][0] - m[0][2]) / S;
		q.x = (m[1][0] + m[0][1]) / S; 
		q.y = 0.25f * S;
		q.z = (m[2][1] + m[1][2]) / S; 
	} 
	else 
	{ 
		float S = std::sqrt(1.f + m[2][2] - m[0][0] - m[1][1]) * 2; // S=4*qz
		q.w = (m[0][1] - m[1][0]) / S;
		q.x = (m[2][0] + m[0][2]) / S;
		q.y = (m[2][1] + m[1][2]) / S;
		q.z = 0.25f * S;
	}
	return q;
}

// Mat44 Impl

inline Vec4& Mat44::operator[](int col)
{
	return cols[col];
}

inline const Vec4& Mat44::operator[](int col) const
{
	return cols[col];
}

inline const float* Mat44::data() const
{
	return cols[0].data();
}

inline Vec4 operator*(const Mat44& m, const Vec4& v)
{
	return m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3] * v.w;
}

inline Mat44 operator*(const Mat44& m0, const Mat44& m1)
{
	Mat44 m;
	m[0] = m0 * m1[0];
	m[1] = m0 * m1[1];
	m[2] = m0 * m1[2];
	m[3] = m0 * m1[3];
	return m;
}


// Quat Impl

inline Quat make_quat_angle_axis(float angle, const Vec3& axis)
{
	const auto s = std::sin(angle * 0.5f);
	const auto c = std::cos(angle * 0.5f);

	return Quat{ axis.x * s, axis.y * s, axis.z * s, c };
}

inline Vec3 operator*(const Quat& quat, const Vec3& v)
{
	const Vec3 u{ quat.x, quat.y, quat.z };
	const auto uv2 = cross(u, v) * 2.f;
	return v + uv2 * quat.w + cross(u, uv2);
}

inline Quat operator*(const Quat& quat, float s)
{
	return Quat{ quat.x * s, quat.y * s, quat.z * s, quat.w * s };
}

inline Quat operator*(const Quat& q0, const Quat& q1)
{
	return Quat
	{
		q0.w * q1.x + q0.x * q1.w + q0.y * q1.z - q0.z * q1.y,
		q0.w * q1.y + q0.y * q1.w + q0.z * q1.x - q0.x * q1.z,
		q0.w * q1.z + q0.z * q1.w + q0.x * q1.y - q0.y * q1.x,
		q0.w * q1.w - q0.x * q1.x - q0.y * q1.y - q0.z * q1.z,
	};
}

inline Quat operator/(const Quat& quat, float d)
{
	return quat * (1.f / d);
}

inline float dot(const Quat& q0, const Quat& q1)
{
	return (q0.x * q1.x + q0.y * q1.y + q0.z * q1.z + q0.w * q1.w);
}

inline Quat conjugate(const Quat& quat)
{
	return Quat{ -quat.x, -quat.y, -quat.z, quat.w };
}

inline Quat inverse(const Quat& quat)
{
	return conjugate(quat) / dot(quat, quat);
}

inline Mat33 to_mat33(const Quat& quat)
{
	const auto x = quat.x;
	const auto y = quat.y;
	const auto z = quat.z;
	const auto w = quat.w;

	const auto xs = x * x;
	const auto ys = y * y;
	const auto zs = z * z;
	const auto ws = w * w;

	const auto xy2 = x * y * 2.f;
	const auto xz2 = x * z * 2.f;
	const auto yz2 = y * z * 2.f;

	const auto xw2 = x * w * 2.f;
	const auto yw2 = y * w * 2.f;
	const auto zw2 = z * w * 2.f;


	Mat33 m;
	m[0] = Vec3{ xs - ys - zs + ws, xy2 + zw2, xz2 - yw2 };
	m[1] = Vec3{ xy2 - zw2, -xs + ys - zs + ws, yz2 + xw2 };
	m[2] = Vec3{ xz2 + yw2, yz2 - xw2, -xs - ys + zs + ws };
	return m;
}

// Pose Impl

inline Pose inverse(const Pose& pose)
{
	const auto inv_rot = inverse(pose.rot);
	return { inv_rot * -pose.pos, inv_rot };
}

inline Mat44 to_mat44(const Pose& pose)
{
	Mat33 mr = to_mat33(pose.rot);
	Mat44 m;
	m[0] = to_vec4(mr[0], 0.f);
	m[1] = to_vec4(mr[1], 0.f);
	m[2] = to_vec4(mr[2], 0.f);
	m[3] = to_vec4(pose.pos, 1.f);
	return m;
}
