#pragma once

#include <string>

using namespace std;

class Line : public wstring
{
public:
	friend std::wistream& operator>>(std::wistream& is, Line& l)
	{
		std::getline(is, l, L'\n');
		return is;
	}
};
