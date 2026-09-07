#pragma once
#include <iostream>
#include <cmath>
#include <type_traits>




//==============================================================
// Vector2 data type template
//==============================================================
template<typename DataType>
struct Vector2
{
	DataType x;
	DataType y;

	// constructors
	constexpr Vector2() : x(0), y(0) {}
	constexpr Vector2(DataType x, DataType y) : x(x), y(y) {}


	// operator overload template

	//==============================================================
	// Vector Arithmetic Operators
	//==============================================================
	template<typename OtherDataType>
	constexpr auto operator+(const Vector2<OtherDataType>& other) const
	{
		using ResultType = std::common_type_t<DataType, OtherDataType>;

		return Vector2<ResultType>(x + other.x, y + other.y);
	}

	template<typename OtherDataType>
	constexpr auto operator-(const Vector2<OtherDataType>& other) const
	{
		using ResultType = std::common_type_t<DataType, OtherDataType>;

		return Vector2<ResultType>(x - other.x, y - other.y);
	}


	template<typename OtherDataType>
	constexpr Vector2<DataType>& operator+=(const Vector2<OtherDataType>& other)
	{
		x += other.x;
		y += other.y;
		return *this;
	}

	template<typename OtherDataType>
	constexpr Vector2<DataType>& operator-=(const Vector2<OtherDataType>& other)
	{
		x -= other.x;
		y -= other.y;
		return *this;
	}

	// Scalar multiplication and division with scalar on the right
	template<typename OtherDataType>
	constexpr auto operator*(const OtherDataType& scalar) const
	{
		using ResultType = std::common_type_t<DataType, OtherDataType>;

		return Vector2<ResultType>(x * scalar, y * scalar);
	}

	template<typename OtherDataType>
	constexpr auto operator/(const OtherDataType& scalar) const
	{
		using ResultType = std::common_type_t<DataType, OtherDataType>;

		return Vector2<ResultType>(x / scalar, y / scalar);
	}

	template<typename OtherDataType>
	constexpr Vector2<DataType>& operator*=(const OtherDataType& scalar)
	{
		x *= scalar;
		y *= scalar;
		return *this;
	}

	template<typename OtherDataType>
	constexpr Vector2<DataType>& operator/=(const OtherDataType& scalar)
	{
		x /= scalar;
		y /= scalar;
		return *this;
	}

	//==============================================================
	// Utility Operators
	//==============================================================

	// Comparison operators
	template<typename OtherDataType>
	constexpr bool operator==(const Vector2<OtherDataType>& other) const
	{
		return x == other.x && y == other.y;
	}

	template<typename OtherDataType>
	constexpr bool operator!=(const Vector2<OtherDataType>& other) const
	{
		return !(*this == other);
	}

	// Unary operators
	constexpr auto operator+() const
	{
		return *this;
	}

	constexpr auto operator-() const
	{
		return Vector2<DataType>(-x, -y);
	}

	//==============================================================
	// Vector Operators
	//==============================================================
	constexpr auto LengthSquared() const
	{
		return x * x + y * y;
	}

	auto Magnitude() const
	{
		return std::sqrt(LengthSquared());
	}


	Vector2<DataType> Normalized() const		// Intended primarily for floating-point vectors
	{
		auto mag = Magnitude();

		constexpr float EPSILON = 1e-6f; // Small threshold to avoid division by zero

		if (mag <= EPSILON) return Vector2<DataType>(0, 0);

		return Vector2<DataType>(x / mag, y / mag);
	}


	template<typename DataTypeA, typename DataTypeB>
	constexpr static auto Dot(const Vector2<DataTypeA>& Param1, const Vector2<DataTypeB>& Param2)
	{
		return Param1.x * Param2.x + Param1.y * Param2.y;
	}

	template<typename DataTypeA, typename DataTypeB>
	constexpr static auto Cross(const Vector2<DataTypeA>& Param1, const Vector2<DataTypeB>& Param2)
	{
		return Param1.x * Param2.y - Param1.y * Param2.x;

	}

};

// Non-member operator overloads

// float scalar multiplication and division with scalar on the left
template <typename DataType>
constexpr Vector2<DataType> operator*(const DataType& scalar, const Vector2<DataType>& vec)
{
	using ResultType = std::common_type_t<DataType, DataType>;
	return Vector2<ResultType>(scalar * vec.x, scalar * vec.y);
}


// stream operators
template<typename DataType>
std::ostream& operator<<(std::ostream& os, const Vector2<DataType>& vec)
{
	os << "(" << vec.x << ", " << vec.y << ")";
	return os;
}

template<typename DataType>
std::istream& operator>>(std::istream& is, Vector2<DataType>& vec)
{
	char openParen;
	char comma;
	char closeParen;

	is >> openParen
		>> vec.x
		>> comma
		>> vec.y
		>> closeParen;

	if (openParen != '(' ||
		comma != ',' ||
		closeParen != ')')
	{
		is.setstate(std::ios::failbit);
	}

	return is;
}


// Type aliases for common vector types
using Int2 = Vector2<int>;
using Float2 = Vector2<float>;
using Double2 = Vector2<double>;




//============================================================================================================================================================================================
//============================================================================================================================================================================================


//==============================================================
// Vector3 data type template
//==============================================================
template<typename DataType>
struct Vector3
{
	DataType x;
	DataType y;
	DataType z;

	// constructors
	constexpr Vector3() : x(0), y(0), z(0) {}
	constexpr Vector3(DataType x, DataType y, DataType z) : x(x), y(y), z(z) {}

	// operator overload template

	//==============================================================
	// Vector Arithmetic Operators
	//==============================================================
	template<typename OtherDataType>
	constexpr auto operator+(const Vector3<OtherDataType>& other) const
	{
		using ResultType = std::common_type_t<DataType, OtherDataType>;

		return Vector3<ResultType>(x + other.x, y + other.y, z + other.z);
	}

	template<typename OtherDataType>
	constexpr auto operator-(const Vector3<OtherDataType>& other) const
	{
		using ResultType = std::common_type_t<DataType, OtherDataType>;

		return Vector3<ResultType>(x - other.x, y - other.y, z - other.z);
	}


	template<typename OtherDataType>
	constexpr Vector3<DataType>& operator+=(const Vector3<OtherDataType>& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;

		return *this;
	}

	template<typename OtherDataType>
	constexpr Vector3<DataType>& operator-=(const Vector3<OtherDataType>& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;

		return *this;
	}

	// Scalar multiplication and division with scalar on the right
	template<typename OtherDataType>
	constexpr auto operator*(const OtherDataType& scalar) const
	{
		using ResultType = std::common_type_t<DataType, OtherDataType>;

		return Vector3<ResultType>(x * scalar, y * scalar, z * scalar);
	}

	template<typename OtherDataType>
	constexpr auto operator/(const OtherDataType& scalar) const
	{
		using ResultType = std::common_type_t<DataType, OtherDataType>;

		return Vector3<ResultType>(x / scalar, y / scalar, z / scalar);
	}

	template<typename OtherDataType>
	constexpr Vector3<DataType>& operator*=(const OtherDataType& scalar)
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}

	template<typename OtherDataType>
	constexpr Vector3<DataType>& operator/=(const OtherDataType& scalar)
	{
		x /= scalar;
		y /= scalar;
		z /= scalar;
		return *this;
	}

	//==============================================================
	// Utility Operators
	//==============================================================

	// Comparison operators
	template<typename OtherDataType>
	constexpr bool operator==(const Vector3<OtherDataType>& other) const
	{
		return x == other.x && y == other.y && z == other.z;
	}

	template<typename OtherDataType>
	constexpr bool operator!=(const Vector3<OtherDataType>& other) const
	{
		return !(*this == other);
	}

	// Unary operators
	constexpr auto operator+() const
	{
		return *this;
	}

	constexpr auto operator-() const
	{
		return Vector3<DataType>(-x, -y, -z);
	}

	//==============================================================
	// Vector Operators
	//==============================================================
	constexpr auto LengthSquared() const
	{
		return x * x + y * y + z * z;
	}

	auto Magnitude() const
	{
		return std::sqrt(LengthSquared());
	}


	Vector3<DataType> Normalized() const		// Intended primarily for floating-point vectors
	{
		auto mag = Magnitude();

		constexpr float EPSILON = 1e-6f; // Small threshold to avoid division by zero

		if (mag <= EPSILON) return Vector3<DataType>(0, 0, 0);

		return Vector3<DataType>(x / mag, y / mag, z / mag);
	}


	template<typename DataTypeA, typename DataTypeB>
	constexpr static auto Dot(const Vector3<DataTypeA>& Param1, const Vector3<DataTypeB>& Param2)
	{
		return Param1.x * Param2.x + Param1.y * Param2.y + Param1.z * Param2.z;
	}

	template<typename DataTypeA, typename DataTypeB>
	constexpr static auto Cross(const Vector3<DataTypeA>& Param1, const Vector3<DataTypeB>& Param2)
	{
		return Vector3<std::common_type_t<DataTypeA, DataTypeB>>(
			Param1.y * Param2.z - Param1.z * Param2.y,
			Param1.z * Param2.x - Param1.x * Param2.z,
			Param1.x * Param2.y - Param1.y * Param2.x
		);
	}
};

// Non-member operator overloads

// float scalar multiplication and division with scalar on the left
template <typename DataType>
constexpr Vector3<DataType> operator*(const DataType& scalar, const Vector3<DataType>& vec)
{
	using ResultType = std::common_type_t<DataType, DataType>;
	return Vector3<ResultType>(scalar * vec.x, scalar * vec.y, scalar * vec.z);
}


// stream operators
template<typename DataType>
std::ostream& operator<<(std::ostream& os, const Vector3<DataType>& vec)
{
	os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
	return os;
}

template<typename DataType>
std::istream& operator>>(std::istream& is, Vector3<DataType>& vec)
{
	char openParen;
	char comma1;
	char comma2;
	char closeParen;

	is >> openParen
		>> vec.x
		>> comma1
		>> vec.y
		>> comma2
		>> vec.z
		>> closeParen;

	if (openParen != '(' ||
		comma1 != ',' ||
		comma2 != ',' ||
		closeParen != ')')
	{
		is.setstate(std::ios::failbit);
	}

	return is;
}


// Type aliases for common vector types
using Int3 = Vector3<int>;
using Float3 = Vector3<float>;




//============================================================================================================================================================================================
//============================================================================================================================================================================================


//==============================================================
// Vector4 data type template
//==============================================================
template<typename DataType>
struct Vector4
{
	DataType x;
	DataType y;
	DataType z;
	DataType w;

	// constructors
	constexpr Vector4() : x(0), y(0), z(0), w(0){}
	constexpr Vector4(DataType x, DataType y, DataType z, DataType w) : x(x), y(y), z(z), w(w) {}

	// operator overload template

	//==============================================================
	// Vector Arithmetic Operators
	//==============================================================
	template<typename OtherDataType>
	constexpr auto operator+(const Vector4<OtherDataType>& other) const
	{
		using ResultType = std::common_type_t<DataType, OtherDataType>;

		return Vector4<ResultType>(x + other.x, y + other.y, z + other.z, w + other.w);
	}

	template<typename OtherDataType>
	constexpr auto operator-(const Vector4<OtherDataType>& other) const
	{
		using ResultType = std::common_type_t<DataType, OtherDataType>;

		return Vector4<ResultType>(x - other.x, y - other.y, z - other.z, w - other.w);
	}


	template<typename OtherDataType>
	constexpr Vector4<DataType>& operator+=(const Vector4<OtherDataType>& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		w += other.w;

		return *this;
	}

	template<typename OtherDataType>
	constexpr Vector4<DataType>& operator-=(const Vector4<OtherDataType>& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		w -= other.w;

		return *this;
	}

	// Scalar multiplication and division with scalar on the right
	template<typename OtherDataType>
	constexpr auto operator*(const OtherDataType& scalar) const
	{
		using ResultType = std::common_type_t<DataType, OtherDataType>;

		return Vector4<ResultType>(x * scalar, y * scalar, z * scalar, w * scalar);
	}

	template<typename OtherDataType>
	constexpr auto operator/(const OtherDataType& scalar) const
	{
		using ResultType = std::common_type_t<DataType, OtherDataType>;

		return Vector4<ResultType>(x / scalar, y / scalar, z / scalar, w / scalar);
	}

	template<typename OtherDataType>
	constexpr Vector4<DataType>& operator*=(const OtherDataType& scalar)
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		w *= scalar;

		return *this;
	}

	template<typename OtherDataType>
	constexpr Vector4<DataType>& operator/=(const OtherDataType& scalar)
	{
		x /= scalar;
		y /= scalar;
		z /= scalar;
		w /= scalar;

		return *this;
	}

	//==============================================================
	// Utility Operators
	//==============================================================

	// Comparison operators
	template<typename OtherDataType>
	constexpr bool operator==(const Vector4<OtherDataType>& other) const
	{
		return x == other.x && y == other.y && z == other.z && w == other.w;
	}

	template<typename OtherDataType>
	constexpr bool operator!=(const Vector4<OtherDataType>& other) const
	{
		return !(*this == other);
	}

	// Unary operators
	constexpr auto operator+() const
	{
		return *this;
	}

	constexpr auto operator-() const
	{
		return Vector4<DataType>(-x, -y, -z, -w);
	}

	//==============================================================
	// Vector Operators
	//==============================================================
	constexpr auto LengthSquared() const
	{
		return x * x + y * y + z * z + w * w;
	}

	auto Magnitude() const
	{
		return std::sqrt(LengthSquared());
	}


	Vector4<DataType> Normalized() const		// Intended primarily for floating-point vectors
	{
		auto mag = Magnitude();

		constexpr float EPSILON = 1e-6f; // Small threshold to avoid division by zero

		if (mag <= EPSILON) return Vector4<DataType>(0, 0, 0, 0);

		return Vector4<DataType>(x / mag, y / mag, z / mag, w / mag);
	}


	template<typename DataTypeA, typename DataTypeB>
	constexpr static auto Dot(const Vector4<DataTypeA>& Param1, const Vector4<DataTypeB>& Param2)
	{
		return Param1.x * Param2.x + Param1.y * Param2.y + Param1.z * Param2.z + Param1.w * Param2.w;
	}
};

// Non-member operator overloads

// float scalar multiplication and division with scalar on the left
template <typename DataType>
constexpr Vector4<DataType> operator*(const DataType& scalar, const Vector4<DataType>& vec)
{
	using ResultType = std::common_type_t<DataType, DataType>;
	return Vector4<ResultType>(scalar * vec.x, scalar * vec.y, scalar * vec.z, scalar * vec.w);
}


// stream operators
template<typename DataType>
std::ostream& operator<<(std::ostream& os, const Vector4<DataType>& vec)
{
	os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
	return os;
}

template<typename DataType>
std::istream& operator>>(std::istream& is, Vector4<DataType>& vec)
{
	char openParen;
	char comma1;
	char comma2;
	char comma3;
	char closeParen;

	is >> openParen
		>> vec.x
		>> comma1
		>> vec.y
		>> comma2
		>> vec.z
		>> comma3
		>> vec.w
		>> closeParen;

	if (openParen != '(' ||
		comma1 != ',' ||
		comma2 != ',' ||
		comma3 != ',' ||
		closeParen != ')')
	{
		is.setstate(std::ios::failbit);
	}

	return is;
}


// Type aliases for common vector types
using Int4 = Vector4<int>;
using Float4 = Vector4<float>;