#pragma once
#include <string>
#include <map>
#include <functional>

namespace Code {

	enum TokenType {
		INVALID,
		SEMI,		// semi-colon ';'
		IF,			// if-keyword
		ELSE,		// else-keyword
		LC,			// left curly bracket '{'
		RC,			// right curly bracket '}'
		LB,			// left braces '('
		RB,			// right braces ')'
		EOL,		// end of input
		BOOL,		// boolean value
		EQUAL,		// equal sign
		STRING,		// literal string
		VAR,		// variable
		ASSIGN,		// assignment '='
		FUNCTION	// function call
	};


	class Token
	{
	public:
		Token()
		{
			m_type = INVALID;
		}

		Token(TokenType type, const std::string& value)
		{
			m_type = type;
			m_val = value;
		}

		Token(TokenType type, const char c)
		{
			m_type = type;
			m_val = c;
		}

		Token(const Token& token)
		{
			m_type = token.m_type;
			m_val = token.m_val;
		}

		void operator = (const Token& token)
		{
			m_type = token.m_type;
			m_val = token.m_val;
		}

		bool operator == (TokenType type)
		{
			return m_type == type;
		}

		bool operator == (Token& token);

		bool toBool()
		{
			if (m_type == BOOL) return (m_val == "true");
			return false;
		}

	public:
		TokenType		m_type;
		std::string		m_val;
	};

	typedef std::function<void()> Function;

	class Variable
	{
	public:
		Variable() {}
		virtual ~Variable() {}

		virtual std::string get() = 0;
		virtual void set(const std::string& s) = 0;
	};

class Interpreter
{
public:
	Interpreter();

	void runCode(const std::string& codeBlock);

public:
	static void addVar(const std::string& varName, Variable* var);
	static void clearVars();
	static std::string getVarValue(const std::string& varName);

public:
	static void addFunction(const std::string& fncName, Function f);
	static void clearFunctions();

private:
	void parseCode();
	void IfStatement();
	Token nextToken();
	void eat(TokenType token);
	bool condition();
	void block();
	void statement_list();
	void statement();
	void skipStatement();
	Token expr();
	Token term();
	void readAssignment();
	void callFunction();

private:
	std::string		m_code;
	const char*		m_ch;
	Token			m_currToken;

	static std::map<std::string, Variable*>		m_var;
	static std::map<std::string, Function>		m_fnc;
};

}
