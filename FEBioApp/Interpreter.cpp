#include "stdafx.h"
#include "Interpreter.h"

std::map<std::string, Code::Variable*> Code::Interpreter::m_var;
std::map<std::string, Code::Function> Code::Interpreter::m_fnc;

void Code::Interpreter::addVar(const std::string& varName, Code::Variable* var)
{
	m_var[varName] = var;
}

void Code::Interpreter::clearVars()
{
	m_var.clear();
}

std::string Code::Interpreter::getVarValue(const std::string& varName)
{
	Variable* v = m_var[varName];
	return v->get();
}

void Code::Interpreter::addFunction(const std::string& fncName, Function f)
{
	m_fnc[fncName] = f;
}

void Code::Interpreter::clearFunctions()
{
	m_fnc.clear();
}

Code::Interpreter::Interpreter()
{
	m_ch = nullptr;
}

bool Code::Token::operator==(Token& token)
{
	if (m_type == VAR)
	{
		if (token.m_type == STRING)
		{
			return (Interpreter::getVarValue(m_val) == token.m_val);
		}
		else return false;
	}

	if (m_type != token.m_type) return false;
	if (m_type == Code::STRING)
	{
		return (m_val == token.m_val);
	}
	return false;
}


void Code::Interpreter::runCode(const std::string& codeBlock)
{
	// keep a copy of the code block
	m_code = codeBlock;
	m_ch = m_code.c_str();

	// parse the code
	parseCode();
}

void Code::Interpreter::parseCode()
{
	nextToken();
	statement_list();
}

void Code::Interpreter::statement_list()
{
	statement();
	while (m_currToken == Code::SEMI)
	{
		eat(Code::SEMI);
		statement();
	}
}

void Code::Interpreter::statement()
{
	switch (m_currToken.m_type)
	{
	case Code::IF: IfStatement(); break;
	case Code::LC: block(); break;
	case Code::RC: break;
	case Code::VAR: readAssignment(); break;
	case Code::FUNCTION: callFunction(); break;
	default:
		throw 1;
	}
}

void Code::Interpreter::readAssignment()
{
	Token var = m_currToken;
	eat(Code::VAR);
	eat(Code::ASSIGN);
	Token v = expr();

	m_var[var.m_val]->set(v.m_val);
}

void Code::Interpreter::callFunction()
{
	std::string fnc = m_currToken.m_val;
	eat(Code::FUNCTION);
	eat(Code::LB);
	fprintf(stderr, "Calling function: %s\n", fnc.c_str());
	m_fnc[fnc]();
	eat(Code::RB);
}

void Code::Interpreter::skipStatement()
{
	eat(Code::LC);
	while (m_currToken.m_type != Code::RC) nextToken();
	eat(Code::RC);
}

void Code::Interpreter::eat(TokenType token)
{
	if (m_currToken == token) m_currToken = nextToken();
	else throw 1;
}

void Code::Interpreter::block()
{
	eat(LC);
	statement_list();
	eat(RC);
}

void Code::Interpreter::IfStatement()
{
	eat(Code::IF);

	// parse the condition
	eat(LB);
	bool b = condition();
	eat(RB);
	if (b)
	{
		statement();
	}
	else skipStatement();

	if (m_currToken == Code::ELSE)
	{
		eat(Code::ELSE);
		if (!b)
			statement();
		else
			skipStatement();
	}
}

bool Code::Interpreter::condition()
{
	Token v = expr();

	return v.toBool();
}

Code::Token Code::Interpreter::expr()
{
	Token left = term();

	if (m_currToken.m_type == Code::SEMI) return left;

	Token op = m_currToken; nextToken();

	Token right = term();

	switch (op.m_type)
	{
		case EQUAL: 
		{
			if (left == right) return Token(BOOL, "true"); else return Token(BOOL, "false");
		}
		break;
	}

	return Token(Code::EOL, "");
}

Code::Token Code::Interpreter::term()
{
	Token token = m_currToken;
	nextToken();
	return token;
}

Code::Token Code::Interpreter::nextToken()
{
	// skip whitespace
	while (iswspace(*m_ch)) ++m_ch;

	// if the next character is a letter, then read the text
	if (isalpha(*m_ch))
	{
		std::string v;
		while (isalpha(*m_ch) || (*m_ch == '.')) v.push_back(*m_ch++);

		// check keywords
		if      (v == "if"  ) m_currToken = Token(Code::IF  , v);
		else if (v == "else") m_currToken = Token(Code::ELSE, v);
		else
		{
			if (*m_ch == '(') m_currToken = Token(Code::FUNCTION, v);
			else m_currToken = Token(Code::VAR, v);
		}
	}
	else if (*m_ch == '\'')
	{
		m_ch++;
		std::string v;
		while (*m_ch != '\'') v.push_back(*m_ch++);
		m_ch++;

		m_currToken = Token(Code::STRING, v);
	}
	else if (*m_ch == '{') m_currToken = Token(Code::LC, *m_ch++);
	else if (*m_ch == '}') m_currToken = Token(Code::RC, *m_ch++);
	else if (*m_ch == '(') m_currToken = Token(Code::LB, *m_ch++);
	else if (*m_ch == ')') m_currToken = Token(Code::RB, *m_ch++);
	else if (*m_ch == ';') m_currToken = Token(Code::SEMI, *m_ch++);
	else if (*m_ch == '=')
	{
		char ch = *(m_ch + 1);
		if (ch == '=')
		{
			m_ch += 2;
			m_currToken = Token(Code::EQUAL, "==");
		}
		else m_currToken = Token(Code::ASSIGN, *m_ch++);
	}
	else m_currToken = Token(Code::EOL, "");

	return m_currToken;
}
