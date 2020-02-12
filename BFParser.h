#include <string>
#include <list>
#include <vector> //for stacks
#ifndef BFPARSER_H
#define BFPARSER_H

extern bool isInstruction(char c);

class Token
{
public:

	enum Type
	{
		LEFT_BRACKET,
		RIGHT_BRACKET,
		COMMA,
		DOT,
		PLUS,
		MINUS,
		PTR_LEFT,
		PTR_RIGHT,
		MACRO,
		NONE
	} m_type;


	Token(std::string& text, int line, int rep, int fileIndex, Type type):
	m_text(text), m_lineNumber(line), m_repeats(rep), m_type(type), m_fileIndex(fileIndex)
	{

	}

	static inline bool sameTypeOrOpposite(Type m1, Type m2)
	{
		switch (m1)
		{
		case PLUS:
		case MINUS:
			if (m2 == PLUS)
			{
				return true;
			}
			break;
		case PTR_LEFT:
		case PTR_RIGHT:
			if (m2 == PTR_RIGHT)
			{
				return true;
			}
			break;
		default:
			return false;
		}
	}

	static inline bool matching(Type m1, Type m2)
	{
		return sameTypeOrOpposite(m1, m2) || sameTypeOrOpposite(m2, m1);
	}

	static Type tokenType(char c)
	{
		switch (c)
		{
		case '[':
			return LEFT_BRACKET;
		case ']':
			return RIGHT_BRACKET;
		case ',':
			return COMMA;
		case '.':
			return DOT;
		case '+':
			return PLUS;
		case '-':
			return MINUS;
		case '<':
			return PTR_LEFT;
		case '>':
			return PTR_RIGHT;
		case '%':
			return MACRO;
		default:
			return NONE;	
		}
	}

	inline const std::string& text() const
	{
		return m_text;
	}
	inline int lineNumber() const
	{
		return m_lineNumber;
	}
	inline int repeats() const
	{
		return m_repeats;
	}

	inline Token* opposite()
	{
		return m_opposite;
	}
	inline void opposite(Token* token)
	{
		m_opposite = token;
	}
private:
	std::string m_text = "";
	Token* m_opposite = nullptr; //This is the opposite thing.
	int m_lineNumber = -1;
	int m_fileIndex = -1;
	int m_repeats = 0;
};

class BFParser
{
public:
	BFParser(const std::string& file);
	~BFParser();
	void tokenise();
	bool validateTokens();
	bool parse();
	inline bool multiplicible(char c)
	{
		return c == '+' || c == '-' || c == '>' || c == '<';
	}
	void outputCode(std::string& code);
private:
	bool m_optimise = true;
	const std::string m_file;
	std::vector<Token*> m_tokens;
	std::vector<Token> m_final;
};
#endif
