#pragma once
#include <concepts>

template<typename V>
concept FieldValue = requires(V a, V b, float s) {
    { a + b }   -> std::convertible_to<V>;
    { a - b }   -> std::convertible_to<V>;
    { a += b }  -> std::convertible_to<V&>;
    { a -= b }  -> std::convertible_to<V&>;
    { a* s }    -> std::convertible_to<V>;
    { s* a }    -> std::convertible_to<V>;
};

template<typename T>
concept FieldClass = requires(T field, int x, int y) {
    typename T::ValueType;

	{ field.Get(x, y) }                             ->   std::convertible_to<typename T::ValueType>;
    { field.GetClamped(x, y) }                      ->   std::convertible_to<typename T::ValueType>;
    { field.GetWidth() }                            ->   std::convertible_to<int>;
	{ field.GetHeight() }                           ->   std::convertible_to<int>;
	{ field.Set(x, y, typename T::ValueType{}) }    ->   std::same_as<void>;
	{ field.Fill(typename T::ValueType{}) }         ->   std::same_as<void>;
};

template<typename T>
concept ArithmeticField = FieldClass<T> && FieldValue<typename T::ValueType>;

template<typename T>
concept ScalarField = ArithmeticField<T> && std::same_as<typename T::ValueType, float>;

template<typename T>
concept VectorField = ArithmeticField<T> && std::same_as<typename T::ValueType, Float2>;