#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <regex>
#include <map>
#include <array>
#include <functional>
#include <experimental/filesystem>

#include "pObjConstants.h"
#include "Utils.h"
#include "general_types.h"

using namespace std;
namespace fs = experimental::filesystem;

struct PiplineStateObjectProperties
{
	struct InputLayout
	{
		string semantics;
		size_t countBytes;
		size_t shiftBytes;
	};
	size_t number;
	wstring name;
	vector< InputLayout> inputLayouts;
	wstring vsPath;
	wstring psPath;
	bool pathShader = false;
	vector<wstring> cBuffNames;
	vector<wstring> sBuffNames;
};

struct MaterialProperties
{
	wstring name;
	array<float, 4> ambient;
	array<float, 4> diffuse;
	array<float, 4> specular;
	float shininess;

};

class PSOLoader
{
	void writeError(wstring* errBuff, const wstring& error)
	{
		if (!errBuff)
			return;
		*errBuff = error;
	}
public:
	vector<PiplineStateObjectProperties> load(bool& status, const wstring& cfg, wstring* errBuff = nullptr)
	{
		status = false;
		wifstream psoCfg(cfg);
		if (!psoCfg)
		{
			writeError(errBuff, L"Error open file pso config");
			return {};
		}
		map<wstring, function<bool(PiplineStateObjectProperties&, const wstring&)>> sectionsParsers =
		{
			{
				L"number", [](PiplineStateObjectProperties& psoProp, const wstring& data)
				{
					wstringstream wstringToSizeT(data);
					wstringToSizeT >> psoProp.number;
					if (!wstringToSizeT)
					{
						// error
						return false;
					}
					return true;
				}
			},
			{
				L"name", [](PiplineStateObjectProperties& psoProp, const wstring& data)
				{
					psoProp.name = data;
					return true;
				}
			},
			{
				L"vs", [](PiplineStateObjectProperties& psoProp, const wstring& data)
				{
					psoProp.vsPath = data;
					return true;
				}
			},
			{
				L"ps", [](PiplineStateObjectProperties& psoProp, const wstring& data)
				{
					psoProp.psPath = data;
					return true;
				}
			},
			{
				L"path-shader", [](PiplineStateObjectProperties& psoProp, const wstring& data)
				{
					wstringstream flag(data);
					flag >> std::boolalpha >> psoProp.pathShader;
					if (!flag)
					{
						//error
						return false;
					}
					return true;
				}
			},
			{
				L"cBuffers", [](PiplineStateObjectProperties& psoProp, const wstring& data)
				{
					wregex regular(L"\\s+");
					vector<wstring> results;
					for_each(wsregex_token_iterator(data.begin(), data.end(), regular, -1), wsregex_token_iterator(), [&results](const wstring& data)
						{
							results.push_back(regex_replace(data, wregex(L"^ +| +$|( ) +"), L""));
						}
					);
					copy(results.begin(), results.end(), back_inserter(psoProp.cBuffNames));
					return true;
				}
			},
			{
				L"sBuffers", [](PiplineStateObjectProperties& psoProp, const wstring& data)
				{
					wregex regular(L"\\s+");
					vector<wstring> results;
					for_each(wsregex_token_iterator(data.begin(), data.end(), regular, -1), wsregex_token_iterator(), [&results](const wstring& data)
						{
							results.push_back(regex_replace(data, wregex(L"^ +| +$|( ) +"), L""));
						}
					);
					copy(results.begin(), results.end(), back_inserter(psoProp.sBuffNames));
					return true;
				}
			},
			{
				L"inputLayout", [](PiplineStateObjectProperties& psoProp, const wstring& data)
				{
					wregex regular(L"\\s+");
					vector<wstring> results;
					for_each(wsregex_token_iterator(data.begin(), data.end(), regular, -1), wsregex_token_iterator(), [&results](const wstring& data)
						{
							results.push_back(regex_replace(data, wregex(L"^ +| +$|( ) +"), L""));
						}
					);
					if (results.size() != 3)
					{
						// error
						return false;
					}
					array<wstringstream, 2> wstringToSizeT{ wstringstream(results[1]), wstringstream(results[2]) };
					PiplineStateObjectProperties::InputLayout il;
					il.semantics = string(results[0].begin(), results[0].end());
					array<size_t*, 2> sizeTInInputLayout{ &il.countBytes, &il.shiftBytes };
					for (int i(0); i < wstringToSizeT.size(); i++)
					{
						wstringToSizeT[i] >> *sizeTInInputLayout[i];
						if (!wstringToSizeT[i])
						{
							// error
							return false;
						}
					}
					psoProp.inputLayouts.push_back(il);
					return true;
				}
			},
		};
		vector<PiplineStateObjectProperties> psoProp;
		vector<wstring> test;
		for (auto iter = istream_iterator<Line, wchar_t>(psoCfg); iter != istream_iterator<Line, wchar_t>(); iter++)
		{
			wstring data(*iter);
			wregex regular(L"=");
			vector<wstring> results;
			for_each(wsregex_token_iterator(data.begin(), data.end(), regular, -1), wsregex_token_iterator(), [&results](const wstring& data)
				{
					results.push_back(regex_replace(data, wregex(L"^ +| +$|( ) +"), L""));
				}
			);
			if (results.size() != 2)
			{
				writeError(errBuff, L"Error parsing line in pso config");
				return psoProp;
			}
			if (sectionsParsers.find(results[0]) != sectionsParsers.end())
			{
				if (results[0] == L"name")
					psoProp.push_back({});
				wstring _data(results[1]);
				if (!sectionsParsers[results[0]](psoProp.back(), _data))
				{
					writeError(errBuff, L"Error parsing value in pso config: section " + results[0]);
					return psoProp;
				}
			}
		}
		status = true;
		return psoProp;
	}
};

class MaterialLoader
{
	void writeError(wstring* errBuff, const wstring& error)
	{
		if (!errBuff)
			return;
		*errBuff = error;
	}

public:
	vector<MaterialProperties> load(bool& status, const wstring& cfg, wstring* errBuff = nullptr)
	{
		status = false;
		wifstream materialCfg(cfg.c_str());
		if (!materialCfg)
		{
			writeError(errBuff, L"Error open file material config");
			return {};
		}
		map<wstring, function<bool(MaterialProperties&, const wstring&)>> sectionsParsers =
		{
			{
				L"name", [](MaterialProperties& prop, const wstring& data)
				{
					prop.name = data;
					return true;
				}
			},
			{
				L"ambient", [](MaterialProperties& prop, const wstring& data)
				{
					wregex regular(L"\\s+");
					vector<wstring> results;
					for_each(wsregex_token_iterator(data.begin(), data.end(), regular, -1), wsregex_token_iterator(), [&results](const wstring& data)
						{
							results.push_back(regex_replace(data, wregex(L"^ +| +$|( ) +"), L""));
						}
					);
					for (size_t i(0); i < prop.ambient.size(); i++)
					{
						wstringstream stream(results[i]);
						stream >> prop.ambient[i];
						if (!stream)
						{
							// error
							return false;
						}
					}
					return true;
				}
			},
			{
				L"diffuse", [](MaterialProperties& prop, const wstring& data)
				{
					wregex regular(L"\\s+");
					vector<wstring> results;
					for_each(wsregex_token_iterator(data.begin(), data.end(), regular, -1), wsregex_token_iterator(), [&results](const wstring& data)
						{
							results.push_back(regex_replace(data, wregex(L"^ +| +$|( ) +"), L""));
						}
					);
					for (size_t i(0); i < prop.diffuse.size(); i++)
					{
						wstringstream stream(results[i]);
						stream >> prop.diffuse[i];
						if (!stream)
						{
							// error
							return false;
						}
					}
					return true;
				}
			},
			{
				L"specular", [](MaterialProperties& prop, const wstring& data)
				{
					wregex regular(L"\\s+");
					vector<wstring> results;
					for_each(wsregex_token_iterator(data.begin(), data.end(), regular, -1), wsregex_token_iterator(), [&results](const wstring& data)
						{
							results.push_back(regex_replace(data, wregex(L"^ +| +$|( ) +"), L""));
						}
					);
					for (size_t i(0); i < prop.specular.size(); i++)
					{
						wstringstream stream(results[i]);
						stream >> prop.specular[i];
						if (!stream)
						{
							// error
							return false;
						}
					}
					return true;
				}
			},
			{
				L"shininess", [](MaterialProperties& prop, const wstring& data)
				{
					wstringstream stream(data);
					stream >> prop.shininess;
						if (!stream)
						{
							// error
							return false;
						}
						return true;
				}
			}
		};
		vector<MaterialProperties> prop;
		vector<wstring> test;
		for (auto iter = istream_iterator<Line, wchar_t>(materialCfg); iter != istream_iterator<Line, wchar_t>(); iter++)
		{
			wstring data(*iter);
			wregex regular(L"=");
			vector<wstring> results;
			for_each(wsregex_token_iterator(data.begin(), data.end(), regular, -1), wsregex_token_iterator(), [&results](const wstring& data)
				{
					results.push_back(regex_replace(data, wregex(L"^ +| +$|( ) +"), L""));
				}
			);
			if (results.size() != 2)
			{
				writeError(errBuff, L"Error parsing line in material config");
				return prop;
			}
			if (sectionsParsers.find(results[0]) != sectionsParsers.end())
			{
				if (results[0] == L"name")
					prop.push_back({});
				wstring _data(results[1]);
				if (!sectionsParsers[results[0]](prop.back(), _data))
				{
					writeError(errBuff, L"Error parsing value in material config: section " + results[0]);
					return prop;
				}
			}
		}
		status = true;
		return prop;
	}

};

enum sizesByte : size_t
{
	sizeInt = sizeof(int),
	sizeFloat = sizeof(float),
	sizeBoolean = sizeof(bool),
	sizeDataTime = sizeof(DateTime),
	sizeChar = sizeof(char),
	sizeVertex = sizeof(Vertex)
};

class ModelLoader
{

	static bool  loadLineFromFile(string& line, std::ifstream& file)
	{
		char s(' ');
		int size(0);
		line.resize(0);
		while (true)
		{
			file.read((char*)&s, sizeChar);
			if (!file)
			{
				// TODO: print error message
				return false;
			}
			if (s == '\0')
				break;
			line += s;
		}
		return true;
	}

	static bool loadFromFileProObject(const string& path, BlankModel* data)
	{
		std::ifstream file(path, std::ios_base::binary);
		if (!file)
		{
			// TODO: print error message
			return false;
		}
		// переменные для модели
		int countMesh(0); // количество мешей в модели
		Vector center; // центр BBox
		BoundingBoxData bBoxData; // данные для конструирования BBox
		BlankModel _data; // выходной буфер

		while (true)
		{
			int block(0);
			file.read((char*)&block, sizeInt);
			if (file.eof())
				break;
			if (!file)
			{
				// TODO: print error message
				return false;
			}
			switch (block)
			{
			case pObjFormatBlocks::serviceInfo:// блок служебной информации
			{
				string nameAutor;
				if (!loadLineFromFile(nameAutor, file)) // имя автора
				{
					// TODO: print error message
					return false;
				}
				DateTime dateCreate;
				file.read((char*)&dateCreate, sizeDataTime); // дата создания
				if (!file)
				{
					// TODO: print error message
					return false;
				}
				DateTime dateModify;
				file.read((char*)&dateModify, sizeDataTime); // дата модификации
				if (!file)
				{
					// TODO: print error message
					return false;
				}
				file.read((char*)&countMesh, sizeInt); // количество мешей в модели
				if (!file)
				{
					// TODO: print error message
					return false;
				}
				// читаем центр модели
				float arr[6];
				for (int i(0); i < 3; i++)
				{
					file.read((char*)&arr[i], sizeFloat);
					if (!file)
					{
						// TODO: print error message
						return false;
					}
				}
				center = Vector(arr[0], arr[1], arr[2], 1);
				// читаем BBox модели(ограничивающие точки)
				for (int i(0); i < 6; i++)
				{
					file.read((char*)&arr[i], sizeFloat);
					if (!file)
					{
						// TODO: print error message
						return false;
					}
				}
				bBoxData = BoundingBoxData(arr[0], arr[1], arr[2], arr[3], arr[4], arr[5]);
				// TODO: bbox пока не нужен
				//_data.bboxData = bBoxData;
				//_data.bboxData.center = center; // записываем центр
				string unit;
				if (!loadLineFromFile(unit, file)) // читаем единицы измерения
				{
					// TODO: print error message
					return false;
				}
				break;
			}

			case pObjFormatBlocks::materialsInfo: // блок информации о материалах
			{
				int count(0);
				file.read((char*)&count, sizeInt); // количество материалов
				if (!file)
				{
					// TODO: print error message
					return false;
				}
				for (int i(0); i < count; i++)
				{
					string path;
					if (!loadLineFromFile(path, file)) // читаем название материала (пока не нужно)
					{
						// TODO: print error message
						return false;
					}
					if (!loadLineFromFile(path, file)) // читаем путь к текстуре
					{
						// TODO: print error message
						return false;
					}
					wstring newPath(wstring(path.begin(), path.end()));
					if(newPath != wstring(1, ModelConstants::isColor)) 
						newPath.insert(0, L"engine_resource/");
					_data.texturesName.push_back(newPath);
					if (!loadLineFromFile(path, file)) // читаем материал
					{
						// TODO: print error message
						return false;
					}
					_data.materials.push_back(wstring(path.begin(), path.end()));
					bool dSides(false);
					file.read((char*)&dSides, sizeBoolean); // читаем дабл сайдс
					if (!file)
					{
						// TODO: print error message
						return false;
					}
					_data.doubleSides.push_back(dSides);
				}
				break;
			}

			case pObjFormatBlocks::meshesInfo: // блок информации о каждом меше в модели
			{
				// TODO: пока не делаем разделение на меши
				for (int i(0); i < countMesh; i++) // перебираем все меши
				{
					file.read((char*)&block, sizeInt); // id подблока
					if (!file)
					{
						// TODO: print error message
						return false;
					}
					int countIndBuffers(0);
					file.read((char*)&countIndBuffers, sizeInt); // количество индексных буферов для меша
					if (!file)
					{
						// TODO: print error message
						return false;
					}
					string nameMesh;
					if (!loadLineFromFile(nameMesh, file))  // читаем имя меша
					{
						// TODO: print error message
						return false;
					}

					for (int l(0); l < countIndBuffers; l++)
					{
						file.read((char*)&block, sizeInt); // id подблока
						if (!file)
						{
							// TODO: print error message
							return false;
						}
						int ci(0); file.read((char*)&ci, 4); // читаем количество индексов
						if (!file)
						{
							// TODO: print error message
							return false;
						}
						for (int j(0); j < ci; j++) // перебираем все индексы
						{
							int tmpIndex(-1); // временная переменная
							file.read((char*)&tmpIndex, sizeInt); // читаем индекс
							if (!file)
							{
								// TODO: print error message
								return false;
							}
							_data.indexs.push_back((Index)tmpIndex); // пишем считанный индекс в буфер
						}
						_data.shiftIndexs.push_back(ci); // добавляем смешения индексов в буферы
					}
				}
				break;
			}

			case pObjFormatBlocks::vertexInfo: // блок информации о вершинах
			{
				int vCount(0);
				file.read((char*)&vCount, sizeInt); // количество вершин
				if (!file)
				{
					// TODO: print error message
					return false;
				}
				for (int i(0); i < vCount; i++) // перебираем все вершины
				{
					Vertex v;// временная переменная
					file.read((char*)&v, sizeof(Vertex)); // читаем вершину
					if (!file)
					{
						// TODO: print error message
						return false;
					}
					_data.vertexs.push_back(v); // пишем считанную вершину в буфер
				}
				break;
			}

			case pObjFormatBlocks::optionsInfo: // блок настроек
			{
				string pso;
				if (!loadLineFromFile(pso, file))  // читаем имя меша
				{
					// TODO: print error message
					return false;
				}
				_data.psoName = wstring(pso.begin(), pso.end());
				break;
			}

			default:
			{
				file.close();
				return false;
			}

			}
		}
		file.close();
		*data = _data;
		return true;
	}

	static bool loadFromFileObj(const string& path, BlankModel* data)
	{
		BlankModel loadingsData;

		string temp; // временная строка
		vector<Index> indBuf; // индексы
		vector<Vector3> verBuf; // вершины
		vector<Vector3> normBuf; // нормали
		vector<Color> uv; // текстуры
		map<string, string> predTextures; // буфер текстур до распределения
		map<string, Color> predColors; // буфер цветов до распределения
		ifstream file_model(path); // файл модели
		bool isTexture(false); // есть ли текстуры на моделе
		int k(0); // счётчик индекса
		bool isCurrentColor;
		Color currentColor;
		
		if (!file_model)
		{
			// TODO: error
			return false;
		}

		// cначала грузим файл материалов
		string name_file_material(path); name_file_material.resize(name_file_material.length() - 3);
		std::ifstream file_textures(name_file_material + "mtl");
		if (file_textures)
		{
			while (true)
			{
				std::getline(file_textures, temp, '\n');
				if (file_textures.eof())
					break;
				if (!file_textures)
				{
					// TODO: error
					return false;
				}
				// тут грузим текстуры
				vector<string> lines;
				string mtl;
				if (temp.find("newmtl ") != -1)
				{
					lines.push_back(temp);
					mtl = { temp.begin() + 7, temp.end() };
					while(temp.find("Ni ") == -1)
					{
						std::getline(file_textures, temp, '\n');
						if (!file_textures)
						{
							// TODO: error
							return false;
						}
						lines.push_back(temp);
					}
				}
				// определяем, перед нами текстура или цвет
				if (lines.size() == 7)
				{
					// текстура
					temp = lines[5];
					temp.erase(0, 7);
					temp.insert(0, "engine_resource/textures/");
					predTextures.insert({ mtl, temp });
				}
				else
				{
					// цвет
					temp = lines[2] + " ";
					temp.erase(0, 3);
					string number;
					vector <float> tmp;
					std::for_each(temp.begin(), temp.end(), [&number, &tmp](const char& e) {
						if (e != ' ')
							number += e;
						else
						{
							tmp.push_back(0.f);
							std::stringstream ostream(number);
							ostream >> tmp.back();
							number.clear();
						}
						});
					predColors.insert({ mtl , {tmp[0], tmp[1], tmp[2], 1.f} });
				}
			}
			isTexture = true;
		}
		file_textures.close();


		// считываем данные модели
		while (true)
		{
			std::getline(file_model, temp, '\n');
			if (file_model.eof())
				break;
			if (!file_model)
			{
				// TODO: error
				return false;
			}

			if (isTexture && temp.find("usemtl ") != -1) // добавляем текстуру и количество соответствующих ей вершин
			{
				isCurrentColor = false;
				string mtl(temp.begin() + 7, temp.end());
				auto _mtlPred = predTextures.find(mtl);
				if (_mtlPred != predTextures.end())
					loadingsData.texturesName.push_back(wstring(_mtlPred->second.begin(), _mtlPred->second.end()));
				else
				{
					loadingsData.texturesName.push_back(wstring(1, ModelConstants::isColor));
					currentColor = predColors[mtl];
					isCurrentColor = true;
				}
				loadingsData.shiftIndexs.push_back(0);
				loadingsData.doubleSides.push_back(false);
			}

			std::string number;
			std::vector<float> tmp;
			std::string sel;

			if (temp.length() >= 2)
				sel = temp.substr(0, 2);
			else
				sel = temp;

			if (sel == "v ") // вершины
			{
				temp += " ";
				temp.erase(0, 2);
				std::for_each(temp.begin(), temp.end(), [&number, &tmp](const char& e) { // итератор вылетает за границы
					if (e != ' ')
						number += e;
					else
					{
						tmp.push_back(0.f);
						std::stringstream ostream(number);
						ostream >> tmp.back();
						number.clear();
					}
					});
				// переворачиваем отзеркаленную модель
				// добавляем в буфер вершин
				verBuf.push_back({ tmp[0], tmp[1], -tmp[2] });
			}
			else if (sel == "vn") // нормали
			{
				temp += " ";
				temp.erase(0, 3);
				std::for_each(temp.begin(), temp.end(), [&number, &tmp](const char& e) {
					if (e != ' ')
						number += e;
					else
					{
						tmp.push_back(0.f);
						std::stringstream ostream(number);
						ostream >> tmp.back();
						number.clear();
					}
					});
				normBuf.push_back({ tmp[0], tmp[1], tmp[2] }); // -tmp[2]
			}
			else if (sel == "vt") // текстуры
			{
				temp += " ";
				temp.erase(0, 3);
				std::for_each(temp.begin(), temp.end(), [&number, &tmp](const char& e) {
					if (e != ' ')
						number += e;
					else
					{
						tmp.push_back(0.f);
						std::stringstream ostream(number);
						ostream >> tmp.back();
						number.clear();
					}
					});
				uv.push_back({ tmp[0], 0 - tmp[1], 0, -1 });
			}
			else if (sel == "f ") // фейсы(индексы)
			{
				std::vector<Index> Tmp;
				temp += " ";
				temp.erase(0, 2);
				std::for_each(temp.begin(), temp.end(), [&number, &Tmp](const char& e) {
					if (e != '/' && e != ' ')
						number += e;
					else
					{
						Tmp.push_back(0);
						std::stringstream ostream(number);
						ostream >> Tmp.back();
						Tmp.back()--;
						number.clear();
					}
					});

				for (int i(0); i < 3; i++)
					std::swap(Tmp[i], Tmp[Tmp.size() - 3 + i]);

				for (int i(0); i < Tmp.size(); i += 3)
				{
					//indBuf.push_back(Tmp[i]); // индекс вершины
					//indBuf.push_back(Tmp[i + 1]); // индекст текстурных координат
					//indBuf.push_back(Tmp[i + 2]); // индекс нормали

					Color uvAndIndex;
					if (isCurrentColor)
						uvAndIndex = currentColor;
					else
						uvAndIndex = { uv[Tmp[i + 1]][x], uv[Tmp[i + 1]][y], uv[Tmp[i + 1]][z],uv[Tmp[i + 1]][w] };
					loadingsData.vertexs.push_back({ verBuf[Tmp[i]], normBuf[Tmp[i + 2]], uvAndIndex });
					loadingsData.indexs.push_back(k++);
				}
				loadingsData.shiftIndexs.back() += 3;

			}
		}
		file_model.close();

		//for (int j(0), k(0); j < indBuf.size(); j += 3, k++)
		//{
		//	int _0(indBuf[j]);
		//	int _1(indBuf[j + 1]); 
		//	int _2(indBuf[j + 2]); 
		//	loadingsData.indexs.push_back(k);

		//	Color uvAndIndex = { uv[_1][x], uv[_1][y], uv[_1][z],uv[_1][w] };
		//	loadingsData.vertexs.push_back({ verBuf[_0], normBuf[_2], uvAndIndex });
		//}

		*data = loadingsData; // присваиваем данные

		return true;
	}

public:

	static bool loadFileFromExpansion(const string& fileName, BlankModel* data) // выбор нужного метода загрузки исходя из расширения файла
	{
		map<string, int> commands = { {".obj", 0}, {".pObject", 1} };
		int num(0);
		auto iter = commands.find(fs::path(fileName).extension().string());
		if (iter != commands.end())
			num = iter->second;
		switch (num)
		{
		case 0:
			return loadFromFileObj(fileName, data); // загрузка *.obj

		case 1:
			return loadFromFileProObject(fileName, data); // загрузка *.pObject

		default:
		{
			// TODO: неверное расширения файла, не грузим, пишем сообщение в лог
			return false;
		}
		}
	}
};
