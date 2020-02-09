#include "BFInterpreter.h"
#include <iostream>
#include <ctype.h>
BFInterpreter::BFInterpreter(char* fileName, bool singleStep, bool debug):
m_singleStep(singleStep), m_debug(debug)
{
	inputSource(fileName);
}

BFInterpreter::~BFInterpreter()
{
	if (!m_failed)
	{
		delwin(memWin);
		delwin(textWin);
		endwin();
	}

	m_originalIndices.clear();
	m_instructions.clear();
	m_mem.clear();
}

bool BFInterpreter::exit()
{
	return m_exit;
}

void BFInterpreter::inputSource(char* fileName)
{
	m_fileName = fileName;
	BFFileHandler fileHandler(m_fileName, m_fileText);
	fileHandler.loadFile();
	fileHandler.checkErrorsAndParse();
	if (!fileHandler.parseSuccess())
	{
		m_failed = true;
		m_done = true;
		fileHandler.outputErrors();
	}
	else
	{
		if (!m_debug)
		{
			fileHandler.pruneCode();
		}

		if (m_singleStep)
		{
			m_original = fileHandler.getOriginalText();
			m_originalIndices = fileHandler.getOriginalTextIndices();
		}
	}
}

void BFInterpreter::writeInstructions()
{
	if (m_failed)
	{
		return;
	}
		
	for (int i = 0; i < m_fileText.size(); i++)
	{
		char c = m_fileText[i];
		if (isInstruction(c) || isSpecialInstruction(c))
		{
			m_instructions.push_back(c);
			if (c == '[')
			{
				m_loopOpen.push_back(m_instructions.size() - 1);
			}
			else if (c == ']')
			{
				int ip = m_loopOpen.back();
				m_loopOpen.pop_back();
				LoopPair pair;
				pair.open = ip;
				pair.close = m_instructions.size() - 1;
				m_pairs.push_back(pair);
			}
		}
	}

	if (!m_debug)
	{
		m_fileText = "";
	}

	for (int i = 0; i < m_pairs.size(); i++)
	{
		m_instructions[m_pairs[i].open] = -m_pairs[i].close;
		m_instructions[m_pairs[i].close] = -m_pairs[i].open;
	}
	
	m_pairs.clear();

}

void BFInterpreter::run()
{
	if (m_singleStep)
	{
		runSourceSingleStep();
	}
	else
	{
		runSource();
	}
}

void BFInterpreter::singleStep()
{
	switch (m_instructions[m_singleStepIP])
	{
	case '>':
		m_memIndex++;
		break;
	case '<':
		m_memIndex--;
		break;
	case '+':
		m_mem[m_memIndex]++;
		break;
	case '-':
		m_mem[m_memIndex]--;
		break;
	case '.':
		waddch(consoleWin, m_mem[m_memIndex]);
		//std::cout << m_mem[m_memIndex];
		break;
	case ',':
	{
		int c = -1;
		int y, x, h, w;
		getyx(consoleWin, y, x);
		getmaxyx(consoleWin, h, w);
		wmove(consoleWin, 1, w-30);
		wprintw(consoleWin, "Waiting for Keypress...");
		while (!(c >= 0 && c < 255))
		{
			c = wgetch(consoleWin);
		}
		wmove(consoleWin, 1, w-30);
		wclrtoeol(consoleWin);
		wmove(consoleWin, y, x);
		waddch(consoleWin,(unsigned char)c);
		m_mem[m_memIndex] = (unsigned char)c;
		break;
	}
	case '0':
		m_mem[m_memIndex] = 0;
		break;
	default:
		if (-m_instructions[m_singleStepIP] > m_singleStepIP)
		{
			if (!m_mem[m_memIndex])
			{
				m_singleStepIP = -m_instructions[m_singleStepIP];
			}
		}
		else
		{
			if (m_mem[m_memIndex])
			{
				m_singleStepIP = -m_instructions[m_singleStepIP];
			}
		}
		break;
	}
}

void BFInterpreter::initCurses()
{
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	int h, w;
	getmaxyx(stdscr, h, w);//43
	memWin = newwin(h-(m_consoleWinHeight + 1), m_memWinWidth, 0, w - m_memWinWidth);
	textWin = newwin(h-(m_consoleWinHeight+1), w-(m_memWinWidth + 2), 0, 0);
	consoleWin = newwin(m_consoleWinHeight, w, h-m_consoleWinHeight, 0); 
	start_color();
	init_pair(1, COLOR_GREEN, COLOR_BLACK);
	init_pair(2, COLOR_BLUE, COLOR_BLACK);
	init_pair(3, COLOR_RED, COLOR_BLACK);
	init_pair(4, COLOR_YELLOW, COLOR_BLACK);
	init_pair(5, COLOR_CYAN, COLOR_BLACK);
}

void BFInterpreter::updateMemWin()
{
	int h, w;
	getmaxyx(stdscr, h, w);
	h = h - (m_consoleWinHeight+1);
	wresize(memWin, h, m_memWinWidth);
	mvwin(memWin, 0, w-m_memWinWidth);

	getmaxyx(memWin, h, w);
	
	m_maxMemIndex = m_maxMemIndex < m_memIndex ? m_memIndex : m_maxMemIndex;

	wmove(memWin, 1, 0);
	wprintw(memWin, "(x)down, (z)up\n");
	for (int i = 0; i <= h; i++)
	{
		unsigned char c;
		char actual[9] = "........";
		wmove(memWin, i+2, 0);
		wprintw(memWin, "0x%08x: ",8*(i+m_currentMemLine));
		for (int j = 0; j < 8; j++)
		{
			int index = (i+m_currentMemLine)*8 + (j);
			if (index >= m_mem.size() || m_mem.size() < 0)
			{
				c = (unsigned char)0;
			}
			else
			{

				c = m_mem[index];
				if (isprint(m_mem[index]))
				{
					actual[j] = m_mem[index];
				}
			}
			
			if (index == m_memIndex)
			{
				wattron(memWin, A_STANDOUT);
				//wprintw(memWin, "%c", 'c');
				wprintw(memWin, "%02x", c);
				wattroff(memWin, A_STANDOUT);
				waddch(memWin, ' ');
			}
			else if (c == 0 && index > m_maxMemIndex)
			{
				wattron(memWin, COLOR_PAIR(3));
				wprintw(memWin, "%02x", c);
				wattroff(memWin, COLOR_PAIR(3));
				waddch(memWin, ' ');
			}
			else
			{
				wprintw(memWin, "%02x ",c);
			}

		}
		wprintw(memWin, "|%s|", actual);
		wclrtobot(memWin);
		//wprintw(memWin, "%s", actual.c_str());
		//wprintw(memWin, "%s ", "|........|");
		//wattron(memWin, A_BOLD);
		//wprintw(memWin, ": %02x %02x %02x %02x %02x %02x %02x %02x", chars[0] | A_STANDOUT, chars[1], chars[2], chars[3], chars[4], chars[5], chars[6], chars[7]);
		//wattroff(memWin, A_BOLD);
		//wprintw(memWin, " %s", actual);
		//wprintw(memWin, "%#10x: %02x %02x %02x %02x %02x %02x %02x %02x 01234567890", (int)(i*8), (int)chars[0], (int)chars[1], (int)chars[2], (int)chars[3], (int)chars[4], (int)chars[5], (int)chars[6], (int)chars[7]);
		//wprintw(memWin, "12345678901234567890123456789012345678901234567890");
	//	wprintw(memWin, "........");
	//	wprintw(memWin, "........");
	}

	wrefresh(memWin);
	//refresh();
}

void BFInterpreter::updateTextWin()
{
	//wprintw(textWin,
	int h, w;
	getmaxyx(stdscr, h, w);
	h = h - (m_consoleWinHeight+1);
	w = w - (m_memWinWidth + 2);
	wresize(textWin, h, w);
	mvwin(textWin, 0, 0);

	getmaxyx(textWin, h, w);
	wmove(textWin, 0,0);
	int fakeX = 0;
	int fakeY = m_currentLine-1;
	for (int i = 0; i < m_currentLine; i++)
	{
		//wmove(textWin,i,0);
		wprintw(textWin, "\n");
		//wclrtoeol(textWin);
	}
	//wmove(textWin, 0,0);
	
	int index = m_originalIndices[m_singleStepIP];

	for (int i = 0; i < m_original.size(); i++)
	{
		fakeX++;
		bool newline = fakeX >= w || m_original[i] == '\n';
		fakeY += newline ? 1 : 0;
		fakeX = newline ? 0 : fakeX;
		if (fakeY < 0 || fakeY >= (h-1) || fakeX < 0 || fakeX >= w)
		{
			//wclrtoeol(textWin);
			continue;
		}
		else
		{
			//wmove(textWin, fakeY, fakeX);
		}
		
		addCharTextWin(m_original[i], index==i);
	}
	wclrtobot(textWin);

	//wmove(textWin, 0,0);
	//h = m_currentLine;
	//for (int i = 0; h >= 0 && h < m_currentLine+1; i++)
	//{
	//	//getyx(textWin, h, w);
	//	addCharTextWin(m_original[i], index == i);
	//	getyx(textWin, h, w);
	//}

	//wprintw(textWin,"%s", m_original.c_str());
	wrefresh(textWin);
}

void BFInterpreter::addCharTextWin(char c, bool highlight)
{
	if (highlight)
	{
	        wattron(textWin, A_STANDOUT);
	        waddch(textWin, c);
	        wattroff(textWin, A_STANDOUT);
	}
	else if (!isInstruction(c))
	{
        	wattron(textWin, COLOR_PAIR(1));
        	waddch(textWin, c);
        	wattroff(textWin, COLOR_PAIR(1));
	}
	else
	{
	        if (c == '[' || c == ']')
       		{
	                wattron(textWin, COLOR_PAIR(4));
	                waddch(textWin, c);
	                wattroff(textWin, COLOR_PAIR(4));
	        }
	        else if (c == '>' || c == '<')
	        {
	                wattron(textWin, COLOR_PAIR(3));
	                waddch(textWin, c);
	                wattroff(textWin, COLOR_PAIR(3));
	        }
	        else if (c == ',' || c == '.')
	        {
	                wattron(textWin, COLOR_PAIR(5));
	                waddch(textWin, c);
	                wattroff(textWin, COLOR_PAIR(5));
	        }
	        else
	        {
	                waddch(textWin, c);
	        }
	}	
}

void BFInterpreter::updateConsole()
{
	int h, w;
	getmaxyx(stdscr, h, w);
	//h = m_consoleWinHeight;
	wresize(consoleWin, m_consoleWinHeight, w);
	mvwin(consoleWin, h - m_consoleWinHeight, 0);

	getmaxyx(consoleWin, h,w);
	int y, x;
	getyx(consoleWin, y, x);
	wmove(consoleWin, 0,0);
	for (int i = 0; i < w; i++)
	{
		waddch(consoleWin,'_');
	}
	wprintw(consoleWin, "(s)tep, (l)oop, (f)ast, (n)ormal, (q)up, (a)down, (r)eload, (e)xit");
	if (m_done)
	{
		wprintw(consoleWin, "     %s finished; (r)estart", m_fileName.c_str());
	}
	wmove(consoleWin, 2, 0);
	if (x != 0 && y != 0)
	{
		wmove(consoleWin, y, x);
	}
	
	wrefresh(consoleWin);//do this first as it refreshes the whole screen
}

void BFInterpreter::updateWindows()
{
	refresh();
	updateMemWin();
	updateTextWin();
	updateConsole();
}

int BFInterpreter::handleKey(char c)
{
	switch (c)
	{
	case 'l':
		if (m_openBracketsStack.size() > 0)
		{
			m_stopIP = m_openBracketsStack.back();
			m_continue = m_stopIP > 0;
			if (m_continue)
			{
				cbreak();
			}
			nodelay(stdscr, m_continue);
		}
		else
		{
			m_stopIP = m_singleStepIP + 1;
		}
		return 1;
	case 'e':
		m_exit = true;
		return 2;
	case 'r':
		return 2;
	case 's':
		return 1;
	case 'x':
		m_currentMemLine++;
		return 0;
	case 'z':
		m_currentMemLine--;
		return 0;
	case 'q':
		m_currentLine++;
		return 0;
	case 'a':
		m_currentLine--;
		return 0;
	case 'f':
		m_continue = !m_continue;
		cbreak();
		nodelay(stdscr, m_continue);
		return 1;
	case 'n':
		m_continue = !m_continue;
		nodelay(stdscr, 0);
		if (m_continue)
		{
			halfdelay(1);
		}
		return 1;
	default:
		return 0;
	}
}

void BFInterpreter::runSourceSingleStep()
{
	if (m_failed)
	{
		return;
	}
	initCurses();
	zeroMem();
	for (m_singleStepIP = 0; m_singleStepIP < m_instructions.size(); m_singleStepIP++)
	{
		updateWindows();
		if (m_stopIP == m_singleStepIP)
		{
			m_continue = false;
			nodelay(stdscr, 0);
		}


		if (m_fileText[m_singleStepIP] == ']')
		{
        		//wprintw(consoleWin, "p%i ", -m_openBracketsStack.back());
        		m_openBracketsStack.pop_back();
		}
		int ch = 0;
		if (!m_continue)
		{
			int eventType = 0;
			do
			{
				ch = getch();
				//updateWindows();
				eventType = handleKey(ch);
				updateWindows();
			} while (!eventType);
			if (eventType == 2)
			{
				return;
			}
		}
		else
		{
			ch = getch();
			handleKey(ch);
		}
		singleStep();
		//if (m_fileText[m_singleStepIP] == ']')
		//{
		//	wprintw(consoleWin, "p%i ", -m_openBracketsStack.back());
		//	m_openBracketsStack.pop_back();
		//}
		//singleStep();
		if (m_fileText[m_singleStepIP] == '[')
		{
			//wprintw(consoleWin, "%i ", -m_instructions[m_singleStepIP]);
			m_openBracketsStack.push_back(-m_instructions[m_singleStepIP]);
		}
		//if (handleKey(ch))
		//{
		//	return;
		//}
		
		
	}
	m_done = true;
	updateWindows();
	//wprintw(consoleWin, "%s", stopIP);
	//wrefresh(consoleWin);
	int ch = 0;
	cbreak();
	nodelay(stdscr, 0);
	while (ch != 'r')
	{
		ch = getch();
		updateWindows();
	}
}


//Coppied code because we dont want to create a new stack frame for every instruction. The compiler won't inline
//functions with switch statements
void BFInterpreter::runSource()
{
	if (m_failed)
	{
		return;
	}

	zeroMem();
	char c = 0;
	int end = m_instructions.size();
	for (int ip = 0; ip < end; ip++)
	{
		//std::cout << ip << std::endl;
		switch (m_instructions[ip])
		{
		case '>':
			m_memIndex++;
			break;
		case '<':
			m_memIndex--;
			break;
		case '+':
			m_mem[m_memIndex]++;
			break;
		case '-':
			m_mem[m_memIndex]--;
			break;
		case '.':
			std::cout << m_mem[m_memIndex];
			break;
		case ',':
			std::cin >> c;
			m_mem[m_memIndex] = c;
			break;
		case '0':
			m_mem[m_memIndex] = 0;
			break;
		default:
			//we have open bracket
			if (-m_instructions[ip] > ip)
			{
				if (!m_mem[m_memIndex])
				{
					ip = -m_instructions[ip];
				}
			}
			//otherwise close bracket
			else
			{
				if (m_mem[m_memIndex])
				{
					ip = -m_instructions[ip];
				}
			}
			break;
		}
	}
}
