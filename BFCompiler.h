#include "BFFileHandler.h"

class BFCompiler
{
public:
	BFCompiler(char* fileName, bool keepASM, bool debug, bool is64Bit);
	void translate();
	void assemble();
	bool succeeded();
private:

	void inputSource(char* fileName);
	
	void setupASM();
	void finishASM();

	void incrementSP(int v);
	void decrementSP(int v);
	
	void writeValueZero();

	void incrementValue(int v);
	void decrementValue(int v);

	void readKeyInput();
	void outputChar();

	void openBracket();
	void closeBracket();

	int m_currentBracket = 0;
	const bool m_keepASM;

	std::string m_reg = "e";

	std::string m_fileText;
	std::string m_fileName;
	std::string m_outputFileName;
	std::string m_outputAssembly = "";
	std::vector<Error> m_errors;
	std::vector<int> m_bracketID;
	bool m_failed = false;
	bool m_debug = false;
	bool m_is64Bit = false;
};

inline bool BFCompiler::succeeded()
{
	return !m_failed;
}
