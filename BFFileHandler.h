#include <vector>
#include <string>

#ifndef BFFILEHANDLER_H
#define BFFILEHANDLER_H


extern bool isSpecialInstruction(char c);

enum ErrorReason
{
	NONE,
	NO_INSTRUCTIONS_IN_FILE,
	FILE_NOT_FOUND,
	MISSING_CLOSING_BRACKET,
	MISSING_OPENING_BRACKET,
	MISSING_END,
	MISSING_DEFINE,
	NO_MACRO_BY_NAME,
	SYNTAX_ERROR
};

struct Error
{
	Error() {}

	Error(int lineNumber, ErrorReason reason):
	m_lineNumber(lineNumber), m_reason(reason)
	{
	}

	int m_lineNumber = 0;
	ErrorReason m_reason = NONE;
};

class BFFileHandler
{
public:
	BFFileHandler(std::string& fileName, std::string& fileText);
	void loadFile();
	void checkErrorsAndParse();
	void pruneCode();
	bool parseSuccess();
	void outputErrors() const;
	const std::string& getOriginalText() const;
	const std::vector<int>& getOriginalTextIndices() const;
private:
	struct DefPair
	{
		std::string name = "";
		int startIndex = -1;
		int endIndex = -1;
	};

	void printError(const Error& error) const;
	void spliceText(const DefPair& pair, std::string& instructions);
	void getName(std::string& name, int index);

	int m_currentBracket = 0;
	std::string& m_fileText;
	std::string& m_fileName;
	std::string m_fileWithFormatting;
	std::vector<int> m_instructionIndex;
	std::vector<Error> m_errors;
};

inline bool BFFileHandler::parseSuccess()
{
	return m_errors.size() == 0;
}

#endif
