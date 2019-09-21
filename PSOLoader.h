#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <regex>
#include <map>

#include "Utils.h"

using namespace std;

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
	vector<wstring> cBuffNames;
	vector<wstring> sBuffNames;
};


class PSOLoader
{
	void writeError(wstring* errBuff, const wstring& error)
	{
		if (errBuff)
			return;
		*errBuff = error;
	}
public:
	vector<PiplineStateObjectProperties> load(bool& status, const wstring cfg, wstring* errBuff = nullptr)
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
				if (results[0] == L"number")
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