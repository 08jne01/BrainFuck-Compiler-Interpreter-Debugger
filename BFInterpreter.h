#include "BFFileHandler.h"
#include <ncurses.h>
class BFInterpreter
{
public:
	BFInterpreter(char* fileName, bool singleStep, bool debug);
	~BFInterpreter();
	void writeInstructions();
	void run();
	bool exit();
	bool parseSuccess();
private:
	void initCurses();
	void updateWindows();
	void updateConsole();
	void updateMemWin();
	void updateTextWin();
	void addCharTextWin(char c, bool highlight);
	void runSource();
	void runSourceSingleStep();
	void singleStep();
	void inputSource(char* fileName);
	void zeroMem();
	int handleKey(char c);

	std::string m_fileText;
	std::string m_fileName;
	std::string m_original;

	struct LoopPair
	{
		int open;
		int close;
	};

	int m_memIndex = 0;
	int m_maxMemIndex = 0;
	int m_stopIP = 0;
	
	int m_currentLine = 1;
	int m_currentMemLine = 0;

	std::vector<int> m_originalIndices;
	std::vector<LoopPair> m_pairs;
	std::vector<int> m_loopOpen;
	std::vector<int> m_loopClose;
	std::vector<int> m_instructions;
	std::vector<int> m_openBracketsStack;
	std::vector<unsigned char> m_mem;
	bool m_failed = false;
	bool m_singleStep = false;
	bool m_debug = false;
	bool m_exit = false;
	bool m_done = false;
	bool m_continue = false;
	int m_singleStepIP = 0;
	
	const int m_memWinWidth = 47;
	const int m_consoleWinHeight = 5;

	WINDOW* memWin;
	WINDOW* textWin;
	WINDOW* consoleWin;
};

inline void BFInterpreter::zeroMem()
{
	m_mem.resize(500000);
	std::fill(m_mem.begin(), m_mem.end(), 0);
}

inline bool BFInterpreter::parseSuccess()
{
	return !m_failed;
}
