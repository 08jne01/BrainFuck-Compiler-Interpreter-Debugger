#include "BFParser.h"
#include <iostream>
#include <algorithm>
bool isInstruction(char c)
{
	return c == '<' || c == '>' || c == '+' || c == '-' || c == ',' || c == '.' || c == '[' || c == ']';
}


BFParser::BFParser(const std::string& file):
m_file(file)
{
	
}

BFParser::~BFParser()
{
	for (int i = 0; i < m_tokens.size(); i++)
	{
		//Will seg fault here if someone has only deleted some of the tokens and not removed them from the list
		delete m_tokens[i];
	}
	m_tokens.clear();
}

void BFParser::tokenise()
{
	std::vector<Token*> bracketStack;

	int fileSize = m_file.size();
	std::string currentToken = "";
	char current = 0;
	int lineNumber = 0;
	for (int i = 0; i < fileSize; i++)
	{
		current = m_file[i];
		lineNumber += current == '\n' ? 1 : 0;
		if (isInstruction(current))
		{
			currentToken = "";
			currentToken += current;
			
			if (multiplicible(current) && m_optimise)
			{	for (; (i+1) < fileSize && m_file[i+1] == current; i++)
				{
					currentToken += m_file[i+1];
				}
			}
			int tokenSize = currentToken.size();
			Token::Type type = Token::tokenType(current);
			if (type == Token::Type::PTR_LEFT || type == Token::Type::MINUS)
			{
				tokenSize = -tokenSize;
			}
			//This is unsafe af. Will change to shared_ptr when I can be bothered.
			//std::cout << currentToken << std::endl;
			m_tokens.push_back(new Token(currentToken, lineNumber, tokenSize, i, type));
			
			
			switch (current)
			{
			case '[':
				//std::cout << "pushing: " << m_tokens.back() << std::endl;
				bracketStack.push_back(m_tokens.back());
				break;
			case ']':
				Token* open = bracketStack.back();
				open->opposite(m_tokens.back());
				m_tokens.back()->opposite(open);
				//std::cout << "popping: " << open << " with " << m_tokens.back() << std::endl;
				bracketStack.pop_back();
				break;
			}
		}
	}
}

bool BFParser::validateTokens()
{
	std::vector<Token*> openList;
	std::vector<Token*> closeList;
	
	for (int i = 0; i < m_tokens.size(); i++)
	{
		if (m_tokens[i]->m_type == Token::LEFT_BRACKET)
		{
			openList.push_back(m_tokens[i]);
		}
		else
		{
			if (openList.size() > 0)
			{
				openList.pop_back();
			}
			else
			{
				closeList.push_back(m_tokens[i]);
			}
		}
	}

	for (int i = 0; i < openList.size(); i++)
	{
		//add errors
	}

	for (int i = 0; i < closeList.size(); i++)
	{
		//add errors
	}
}

bool BFParser::parse()
{
	bool emptyMem = true;
	//I hate std iterators but it makes sense to use them here with std find.
	for (std::vector<Token*>::iterator it = m_tokens.begin(); it != m_tokens.end(); it++)
	{
		if ((*it)->m_type == Token::Type::PLUS || (*it)->m_type == Token::Type::MINUS)
		{	
			emptyMem = false;
		}
		if (emptyMem && (*it)->m_type == Token::LEFT_BRACKET)
		{
			Token* token = (*it)->opposite();
			//This would take us to the end of the array if there is no opposite bracket but the parse has failed anyways.
			it = std::find(m_tokens.begin(), m_tokens.end(), token);
			continue;
		}
		//std::cout <<  (*it)->text() << std::endl;
		int count = (*it)->repeats();
		Token* token = *it;
		if (multiplicible(token->text()[0]) && m_optimise)
		{
			std::vector<Token*>::iterator itNext = it;
			for (itNext++; itNext != m_tokens.end() &&  Token::sameTypeOrOpposite((*itNext)->m_type, token->m_type); itNext++, it++)
			{
				count += (*itNext)->repeats();
			}
			std::cout << (*it)->text() << std::endl;
			switch (token->m_type)
			{
				case Token::Type::PLUS:
				case Token::Type::MINUS:
				{
					char c = '+';
					if (count < 0)
					{
						c = '-';
					}
					token->text() = "";
					token->repeats(count);
					for (int i = 0; i < std::abs(count); i++)
					{
						token->text() += c;
					}
					break;
				}
				case Token::Type::PTR_RIGHT:
				case Token::Type::PTR_LEFT:
				{
					char c = '>';
					if (count < 0)
					{
						c = '<';
					}
					token->text() = "";
					token->repeats(count);
					for (int i = 0; i < std::abs(count); i++)
					{
						token->text() += c;
					}
					break;
				}
				default:
					break;

			}
		}
		m_final.push_back(*token);
		std::cout << token->text() << " size: " << token->repeats() << " line: " << token->lineNumber() << std::endl;
		
	}
}


