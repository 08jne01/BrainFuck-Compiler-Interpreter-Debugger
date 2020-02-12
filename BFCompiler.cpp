#include "BFCompiler.h"
#include <sstream>
#include <iostream>
#include <fstream>

BFCompiler::BFCompiler(char* fileName, bool keepASM, bool debug, bool is64Bit):
m_keepASM(keepASM), m_debug(debug), m_is64Bit(is64Bit)
{
	inputSource(fileName);
	m_reg = is64Bit ? "r" : "e";
}

void BFCompiler::inputSource(char* fileName)
{
	m_fileName = fileName;
	BFFileHandler fileHandler(m_fileName, m_fileText);
	//fileHandler.loadFile();
	fileHandler.checkErrorsAndParse();
	if (!fileHandler.parseSuccess())
	{
		m_failed = true;
		fileHandler.outputErrors();
	}
	else
	{
		if (!m_debug)
		{
			fileHandler.pruneCode();
		}
	}
}

/*BrainFuck Operators
> increment data pointer
< decrement data pointer
+ increment value
- decrement value
. output byte as ascii char at data pointer
, accept one byte as input at data pointer
[ if byte is zero jump to closing ] otherwise continue
] if byte is non-zero jump to opening [ otherwise continue
*/
void BFCompiler::translate()
{
	//scanForErrors();
	if (m_failed)
	{
		return;
	}
	
	setupASM();

	int spRight = 0;
	int spLeft = 0;
	int inc = 0;
	int dec = 0;
	char prev = 's';
	bool nextAllowed = true;
	for (int i = 0; i < m_fileText.size(); i++)
	{
	//	std::cout << m_fileText[i] << std::endl;
		nextAllowed = i+1 < m_fileText.size();
		switch (m_fileText[i])
		{
		case '>':
			//Go further into the free zeroed stack
			spRight++;
			if (nextAllowed)
			{
				if (m_fileText[i+1] != '>')
				{
					decrementSP(spRight);
					spRight = 0;
				}
			}
			else
			{
				decrementSP(spRight);
				spRight = 0;
			}
			break;
		case '<':
			//Get closer to the original top of the stack
			spLeft++;
			if (nextAllowed)
			{
				if (m_fileText[i+1] != '<')
				{
					incrementSP(spLeft);
					spLeft = 0;
				}
			}
			else
			{
				incrementSP(spLeft);
				spLeft = 0;
			}
			break;
		case '+':
			inc++;
			if (nextAllowed)
			{
				if (m_fileText[i+1] != '+')
				{
					incrementValue(inc);
					inc = 0;
				}
			}
			else
			{
				incrementValue(inc);
				inc = 0;
			}
			break;
		case '-':
			dec++;
			if (nextAllowed)
			{
				if (m_fileText[i+1] != '-')
				{
					decrementValue(dec);
					dec = 0;
				}
			}
			else
			{
				decrementValue(dec);
				dec = 0;
			}
			break;
		case '.':
			outputChar();
			break;
		case ',':
			readKeyInput();
			break;
		case '[':
			openBracket();
			break;
		case ']':
			closeBracket();
			break;
		case '0':
			writeValueZero();
			break;
		default:
			break;
		}
	}

	finishASM();

	m_outputFileName = "";
	for (int i = 0; i < m_fileName.size(); i++)
	{
		if (m_fileName[i] == '.')
		{
			break;
		}
		m_outputFileName += m_fileName[i];
	}

	m_outputFileName;

	std::ofstream file(m_outputFileName + ".asm");
	if (file.is_open())
	{

		file << m_outputAssembly;
		file.close();
	}
	else
	{
		std::cout << "Failed to write assembly to file" << std::endl;
	}
}

void BFCompiler::assemble()
{
	if (m_failed)
	{
		return;
	}
	std::string command = "";
	if (!m_is64Bit)
	{
		command = "nasm -f elf32 ";
		command += m_outputFileName + ".asm";
		system(command.c_str());
		command = "ld -m elf_i386 -s -o ";
		command += m_outputFileName + " " + m_outputFileName + ".o";
		system(command.c_str());
		command = "rm ";
		command += m_outputFileName + ".o";
	}
	else
	{
		command = "nasm -f elf64 ";
		command += m_outputFileName + ".asm";
		system(command.c_str());
		command = "ld -s -o ";
		command += m_outputFileName + " " + m_outputFileName + ".o";
		system(command.c_str());
		command = "rm ";
		command += m_outputFileName + ".o";
	}
	if (!m_keepASM)
	{
		command += " " + m_outputFileName + ".asm";
	}
	system(command.c_str());
}

//setup the required boiler plate
void BFCompiler::setupASM()
{
	m_outputAssembly += "section .text\n\tglobal _start\n_start:\n";
	m_outputAssembly += \ 
"\
	push " + m_reg +"bp\n\
	mov " + m_reg + "bp, " + m_reg +"sp\n\
	mov " + m_reg + "ax, 0\n\
.fillZero:\n\
	mov [" + m_reg + "sp], byte 0\n\
	inc " + m_reg + "ax\n\
	dec " + m_reg + "sp\n\
	cmp "+ m_reg +"ax, 500000; zero out about 500KB\n\
	jg .done\n\
	jmp .fillZero\n\
.done:\n\
	mov " + m_reg + "sp, " + m_reg + "bp\n\n\
";
}

void BFCompiler::finishASM()
{
	m_outputAssembly += "\n\n\tpop " + m_reg + "bp\n\tmov "+m_reg+"ax, 1\n\tmov "+m_reg+"bx, 0\n\tint 0x80";
}

//Special compiler/interpreter instruction to optimise zeroing loops
void BFCompiler::writeValueZero()
{
	m_outputAssembly += "\tmov ["+m_reg+"sp], byte 0\n";
}

//Increment the stack pointer
void BFCompiler::incrementSP(int v)
{
	m_outputAssembly += v > 1 ? "\tadd "+m_reg+"sp, " + std::to_string(v) + "\n" : "\tinc "+m_reg+"sp\n";
}
//Decrement the stack pointer
void BFCompiler::decrementSP(int v)
{
	m_outputAssembly += v > 1 ? "\tadd "+m_reg+"sp, " + std::to_string(-v) + "\n" : "\tdec "+m_reg+"sp\n";
}
//Increment the value at the current stack location
void BFCompiler::incrementValue(int v)
{
	m_outputAssembly += v > 1 ? "\tadd byte ["+m_reg+"sp], " + std::to_string(v) + "\n" :  "\tinc byte ["+m_reg+"sp]\n";
}
//Decrement the value at the current stack location
void BFCompiler::decrementValue(int v)
{
	m_outputAssembly += v > 1 ? "\tadd byte ["+m_reg+"sp], " + std::to_string(-v) + "\n" : "\tdec byte ["+m_reg+"sp]\n";
}
//Print one byte to console as ascii char
void BFCompiler::outputChar()
{
	if (m_is64Bit)
	{
		m_outputAssembly += "\tmov rax, 1\n\tmov rdi, 1\n\tmov rsi, 1\n\t mov rsi, rsp\n\tmov rdx, 1\n\tsyscall\n";
	}
	else
	{
		m_outputAssembly += "\tmov eax, 4\n\tmov ebx, 1\n\tmov ecx, esp\n\tmov edx, 1\n\tint 0x80\n";
	}
}
//Read one key press as a char into the current stack location
void BFCompiler::readKeyInput()
{
	if (m_is64Bit)
	{
		m_outputAssembly += "\txor rax, rax\n\txor rdi, rdi\n\tmov rsi, rsp\n\tmov rdx, 1\n\tsyscall\n";
	}
	else
	{
		m_outputAssembly += "\tmov eax, 3\n\tmov ebx, 0\n\tmov ecx, esp\n\tmov edx, 1\n\tint 0x80\n";
	}
}
//Open Bracket if byte is zero jump to closing bracket
void BFCompiler::openBracket()
{
	std::string current = std::to_string(m_currentBracket);
	m_outputAssembly += ".loop";
	m_outputAssembly += current;
	m_outputAssembly += ":\n\tcmp byte ["+m_reg+"sp], 0\n\tje .loop";
	m_outputAssembly += current;
	m_outputAssembly += "End\n";
	m_bracketID.push_back(m_currentBracket);
	m_currentBracket++;
}

//Close Bracket if byte is non-zero jump to opening bracket
void BFCompiler::closeBracket()
{
	int id = m_bracketID.back();
	m_bracketID.pop_back();
	std::string current = std::to_string(id);
	m_outputAssembly += ".loop";
	m_outputAssembly += current;
	m_outputAssembly += "End:\n\tcmp byte ["+m_reg+"sp], 0\n\tjne .loop";
	m_outputAssembly += current;
	m_outputAssembly += '\n';
}
