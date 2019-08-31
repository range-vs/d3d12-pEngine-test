#pragma once

#include "mathematics_typedef_vector.h"
#include "mathematics_typedefs_matrix.h"
#include "mathematics_typedef_planes.h"
#include "Common.h"

// Описание:
// математическая библиотека, предоставляющая набор классов векторов и матриц
// также есть вспомогательные классы математики
// поставляется с юникодом, для аски надо переключить в свойствах проекта используемую
// кодировку и пересобрать библиотеку

// Пространства имён
// gmath - graphics math
// gmtypes - graphics math new types(const and typedef)
// gmexception - graphics math exception

// Состав аргументов
// 1 - тип хранимых данных
// 2 - граница выравнивания памяти
// 3 - количество знаков после запятой

// Типы(работа с графикой):
// Vector4 - четырехмерный вектор
// Vector3 - трехмерный вектор
// Vector - четырехмерный вектор
// Point3 - трехмерная точка
// Point4 - четырехмерная точка
// Point - трехмерная точка
// Color - цвет, 4 компонента
// UVCoordinate - двумерная структура цвета
// Matrix4 - матрица 4 х 4
// Matrix3 - матрица 3 х 3
// Matrix - матрица 4 х 4
