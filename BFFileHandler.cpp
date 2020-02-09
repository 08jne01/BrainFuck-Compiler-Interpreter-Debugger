#include "BFFileHandler.h"
#include <sstream>
#include <fstream>
#include <iostream>

bool isInstruction(char c)
{
	return c == '<' || c == '>' || c == '+' || c == '-' || c == '.' || c == ',' || c == '[' || c == ']';	
}

bool isSpecialInstruction(char c)
{
	return c == '0';
}

BFFileHandler::BFFileHandler(std::string& fileName, std::string& fileText):
m_fileName(fileName), m_fileText(fileText)
{
	loadFile();
}

void BFFileHandler::loadFile()
{
	std::ifstream file(m_fileName);
	std::stringstream stream;
	if (file.is_open())
	{
		stream << file.rdbuf();
		m_fileText = stream.str();
		file.close();
	}
	else
	{
		m_errors.push_back(Error(0, FILE_NOT_FOUND));
	}
	
	//std::string instructions = "";
	//m_fileWithFormatting = m_fileText;
//	for (int i = 0; i < m_fileText.size(); i++)
//	{
//		if (isInstruction(m_fileText[i]))
//		{
//			m_instructionIndex.push_back(i);
//			instructions += m_fileText[i];
//		}
//	}
	
//	m_fileText = instructions;
//	instructions.clear();
			
}

void BFFileHandler::checkErrorsAndParse()
{
	std::vector<int> lineNumbersOpen;
	std::vector<int> lineNumbersClose;
	std::vector<DefPair> functions;
	std::vector<int> lineNumbersOpenDef;
	std::vector<int> lineNumbersCloseDef;
	std::vector<int> lineNumbersUse;
	std::vector<int> lineNumberSyntax;

	//std::string instructions = "";

	const std::string def = "def";
	const std::string end = "end";
	const std::string use = "use";
	std::vector<DefPair> stack;
	std::string keyword = "";
	m_fileWithFormatting = m_fileText;
	int lineNumber = 1;
	for (int i = 0; i < m_fileText.size(); i++)
	{
//              std::cout << lineNumbersOpen.size() << " " << lineNumbersClose.size() << std::endl;
		if (m_fileText[i] == '\n')
		{
			lineNumber++;
		}
		else if (m_fileText[i] == '[')
		{
			m_currentBracket++;
			lineNumbersOpen.push_back(lineNumber);
		}
		else if (m_fileText[i] == ']')
		{
			m_currentBracket--;
			if (lineNumbersOpen.size() > 0)
			{
				lineNumbersOpen.pop_back();
			}
			else
			{
				lineNumbersClose.push_back(lineNumber);
			}
		}
		else if (m_fileText[i] == '%')
		{
			if (i + 3 >= m_fileText.size())
			{
				lineNumberSyntax.push_back(lineNumber);
			}
			else
			{
				keyword = "";
				keyword += m_fileText[i+1];
				keyword += m_fileText[i+2];
				keyword += m_fileText[i+3];	
				if (keyword == def)
				{
					DefPair pair;
					getName(pair.name, i + 5);
					pair.startIndex = i + 5 + pair.name.size();
					stack.push_back(pair);
					lineNumbersOpenDef.push_back(lineNumber);
				}
				else if (keyword == end)
				{
					
					if (lineNumbersOpenDef.size() > 0)
					{
						lineNumbersOpenDef.pop_back();
						DefPair pair = stack.back();
						pair.endIndex =  i-1;
						stack.pop_back();
						functions.push_back(pair);
					}
					else
					{
						lineNumbersCloseDef.push_back(lineNumber);
					}
				}
				else if (keyword == use)
				{
					std::string name = "";
					getName(name, i + 5);
					bool found = false;
					for (int j = 0; j < functions.size(); j++)
					{
						if (functions[j].name == name)
						{
							found = true;
							break;
						}
					}
					if (!found)
					{
						lineNumbersUse.push_back(lineNumber);
					}
				}
			}
		}
	}
	
	for (int i = 0; i < lineNumbersOpen.size(); i++)
	{
		m_errors.push_back(Error(lineNumbersOpen[i], MISSING_OPENING_BRACKET));
	}
	
	for (int i = 0; i < lineNumbersClose.size(); i++)
	{
		m_errors.push_back(Error(lineNumbersClose[i], MISSING_CLOSING_BRACKET));
	}

	if (m_currentBracket != 0 && m_errors.size() == 0)
	{
		m_errors.push_back(Error(0, NONE));
	}

	for (int i = 0; i < lineNumbersCloseDef.size(); i++)
	{
		m_errors.push_back(Error(lineNumbersCloseDef[i], MISSING_END));
	}

	for (int i = 0; i < lineNumbersOpenDef.size(); i++)
	{
		m_errors.push_back(Error(lineNumbersOpenDef[i], MISSING_DEFINE));
	}

	for (int i = 0; i < lineNumbersUse.size(); i++)
	{
		m_errors.push_back(Error(lineNumbersUse[i], NO_MACRO_BY_NAME));
	}

	for (int i = 0; i < lineNumberSyntax.size(); i++)
	{
		m_errors.push_back(Error(lineNumberSyntax[i], SYNTAX_ERROR));
	}

	if (m_errors.size() != 0)
	{
		return;
	}

	lineNumbersUse.clear();
	lineNumberSyntax.clear();
	lineNumbersOpenDef.clear();
	lineNumbersCloseDef.clear();
	lineNumbersOpen.clear();
	lineNumbersClose.clear();

	std::string instructions = "";
	std::string name = "";
	for (int i = 0; i < m_fileText.size(); i++)
	{
		if (m_fileText[i] == '%' && (i+3) < m_fileText.size())
		{
			keyword = "";
			keyword += m_fileText[i+1]; 
			keyword += m_fileText[i+2];
			keyword += m_fileText[i+3];
			if (keyword == use)
			{
				getName(name, i + 5);
				bool found = false;
				for (int j = 0; j < functions.size(); j++)
				{
					if (functions[j].name == name)
					{
						spliceText(functions[j], instructions);
						i += 4 + functions[j].name.size(); 
						break;
					}
				}

			}
			else if (keyword == def)
			{
				getName(name, i+5);
				for (int j = 0; j < functions.size(); j++)
				{
					if (functions[j].name == name)
					{
						i = functions[j].endIndex + 4;	
						break;
					}
				}
			}
		}
		else
		{
			m_instructionIndex.push_back(i);
			instructions += m_fileText[i];
		}
	}
	m_fileText = instructions;
	instructions.clear();
	instructions = "";
	std::vector<int> instructionIndex;
	for (int i = 0; i < m_fileText.size(); i++)
	{
		if (isInstruction(m_fileText[i]))
		{
			instructionIndex.push_back(m_instructionIndex[i]);
			instructions += m_fileText[i];
		}
	}
	m_fileText = instructions;
	instructions.clear();
	m_instructionIndex = instructionIndex;

	if (m_fileText.size() == 0)
	{
		m_errors.push_back(Error(0, NO_INSTRUCTIONS_IN_FILE));
	}

	instructionIndex.clear();
}

void BFFileHandler::spliceText(const DefPair& pair, std::string& instructions)
{
	for (int i = pair.startIndex; i < pair.endIndex && i < m_fileText.size(); i++)
	{
		m_instructionIndex.push_back(i);
		instructions += m_fileText[i];
	}
}

void BFFileHandler::getName(std::string& name, int index)
{
	name = "";
	for (int i = index; i < m_fileText.size() && m_fileText[i] != ' ' && m_fileText[i] != '\n' && m_fileText[i] != '\r'; i++)
	{
		if (m_fileText[i] != ' ' && m_fileText[i] != '\n' && m_fileText[i] != '\r')
		{
			name += m_fileText[i];
		}
	}
}

//Add optimisations
//[-] -> '0' -> mov [esp], 0
//{+] -> '0' -> mov [esp], 0
void BFFileHandler::pruneCode()
{
	std::string instructions = "";
	int size = m_fileText.size();
	std::vector<int> buffer;
	char c;
	for (int i = 0; i < m_fileText.size(); i++)
	{
		c = m_fileText[i];	
		if (i < (size - 2) && c == '[' && (m_fileText[i+1] == '-' || m_fileText[i+1] == '+') && m_fileText[i+2] == ']')
		{
			instructions += '0'; //instruction for compiler/interpreter to set current cell to zero
					     //without actually doing the loop
			i += 2;
		}
		else
		{
			instructions += c;
		}
		buffer.push_back(m_instructionIndex[i]);
	}
	m_instructionIndex = buffer;
	buffer.clear();
	m_fileText = instructions;
	instructions.clear();
}

const std::string& BFFileHandler::getOriginalText() const
{
	return m_fileWithFormatting;
}

const std::vector<int>& BFFileHandler::getOriginalTextIndices() const
{
	return m_instructionIndex;
}

void BFFileHandler::printError(const Error& error) const
{
	switch (error.m_reason)
	{
	case FILE_NOT_FOUND:
		std::cout << "ERROR: File \"" << m_fileName << "\" not found" << std::endl;
		break;
	case MISSING_CLOSING_BRACKET:
		std::cout << "ERROR: Missing closing bracket at line " << error.m_lineNumber << std::endl;
		break;
	case MISSING_OPENING_BRACKET:
		std::cout << "ERROR: Missing opening bracket at line " << error.m_lineNumber << std::endl;
		break;
	case MISSING_END:
		std::cout << "ERROR: Missing \%end at line " << error.m_lineNumber << std::endl;
		break;
	case MISSING_DEFINE:
		std::cout << "ERROR: Missing \%def at line " << error.m_lineNumber << std::endl;
		break;
	case NO_MACRO_BY_NAME:
		std::cout << "ERROR: Undefined macro name at line " << error.m_lineNumber << std::endl;
		break;
	case SYNTAX_ERROR:
		std::cout << "ERROR: Incorrect syntax on line " << error.m_lineNumber << std::endl;
		break;
	case NO_INSTRUCTIONS_IN_FILE:
		std::cout << "ERROR: No instructions found in file" << std::endl;
		break;
	case NONE:
	default:
		std::cout << "ERROR: Unknown Reason (possible bug in bf compiler)" << std::endl;
		break;
	}
}

void BFFileHandler::outputErrors() const
{
	std::cout << "Parsing Failed" << std::endl;
	for (int i = 0; i < m_errors.size(); i++)
	{
		printError(m_errors[i]);
	}
}

