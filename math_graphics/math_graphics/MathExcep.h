#pragma once

#include <string>
#include <iostream>

namespace gmexception
{

#ifdef UNICODE

	class MainMathException
	{
	protected:
		std::wstring message;
	public:
		MainMathException() {}
		MainMathException(const std::wstring& m) :message(m) {}
		MainMathException(const MainMathException& m) { this->message = m.message; }
		virtual std::wstring what() = 0;
	};

	class VectorIndexException : public MainMathException
	{
	public:
		VectorIndexException() {};
		VectorIndexException(const std::wstring& m) :MainMathException(m) {};
		VectorIndexException(const VectorIndexException& m) : MainMathException(m) {};
		VectorIndexException& operator=(const VectorIndexException& m) { if (this != &m) this->message = m.message; return *this; }
		std::wstring what() { return this->message; }
	};

	class MatrixIndexLineException : public MainMathException
	{
	public:
		MatrixIndexLineException() {};
		MatrixIndexLineException(const std::wstring& m) : MainMathException(m) {};
		MatrixIndexLineException(const MatrixIndexLineException& m) : MainMathException(m) {};
		MatrixIndexLineException& operator=(const MatrixIndexLineException& m) { if (this != &m) this->message = m.message; return *this; }
		std::wstring what() { return this->message; }
	};

	class MatrixIndexElementException : public MainMathException
	{
	public:
		MatrixIndexElementException() {};
		MatrixIndexElementException(const std::wstring& m) : MainMathException(m) {  };
		MatrixIndexElementException(const MatrixIndexElementException& m) { this->message = m.message;};
		MatrixIndexElementException& operator=(const MatrixIndexElementException& m) { if (this != &m) this->message = m.message; return *this; }
		std::wstring what() { return this->message; }
	};

	class MatrixDeterminantException : public MainMathException
	{
	public:
		MatrixDeterminantException() {};
		MatrixDeterminantException(const std::wstring& m) : MainMathException(m) {};
		MatrixDeterminantException(const MatrixDeterminantException& m) : MainMathException(m) {};
		MatrixDeterminantException& operator=(const MatrixDeterminantException& m) { if (this != &m) this->message = m.message; return *this; }
		std::wstring what() { return this->message; }
	};

#else
	
	class MainMathException
	{
	protected:
		std::string message;
	public:
		MainMathException() {}
		MainMathException(const std::string& m) :message(m) {}
		MainMathException(const MainMathException& m) { this->message = m.message; }
		virtual std::string what() = 0;
	};

	class VectorIndexException : public MainMathException
	{
	public:
		VectorIndexException() {};
		VectorIndexException(const std::string& m) :MainMathException(m) {};
		VectorIndexException(const VectorIndexException& m) : MainMathException(m) {};
		VectorIndexException& operator=(const VectorIndexException& m) { if (this != &m) this->message = m.message; return *this; }
		std::string what() { return this->message; }
	};

	class MatrixIndexLineException : public MainMathException
	{
	public:
		MatrixIndexLineException() {};
		MatrixIndexLineException(const std::string& m) : MainMathException(m) {};
		MatrixIndexLineException(const MatrixIndexLineException& m) : MainMathException(m) {};
		MatrixIndexLineException& operator=(const MatrixIndexLineException& m) { if (this != &m) this->message = m.message; return *this; }
		std::string what() { return this->message; }
	};

	class MatrixIndexElementException : public MainMathException
	{
	public:
		MatrixIndexElementException() {};
		MatrixIndexElementException(const std::string& m) : MainMathException(m) {  };
		MatrixIndexElementException(const MatrixIndexElementException& m) { this->message = m.message; };
		MatrixIndexElementException& operator=(const MatrixIndexElementException& m) { if (this != &m) this->message = m.message; return *this; }
		std::string what() { return this->message; }
	};

	class MatrixDeterminantException : public MainMathException
	{
	public:
		MatrixDeterminantException() {};
		MatrixDeterminantException(const std::string& m) : MainMathException(m) {};
		MatrixDeterminantException(const MatrixDeterminantException& m) : MainMathException(m) {};
		MatrixDeterminantException& operator=(const MatrixDeterminantException& m) { if (this != &m) this->message = m.message; return *this; }
		std::string what() { return this->message; }
	};

#endif

}