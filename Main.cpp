#include "BFCompiler.h"
#include "BFInterpreter.h"
#include <iostream>
#include <cstring>
#include <stdlib.h>
static void printHelp()
{
	std::cout << "Usage ./bfc <filename> [optional params]" << std::endl;
	std::cout << "Params:" << std::endl << "-h -> help" << std::endl;
	std::cout << "-k -> keep asm file" << std::endl;
	std::cout << "-i -> run file in the interpreter" << std::endl;
	std::cout << "-s -> if running as interpreter this will enter single step mode" << std::endl;
	std::cout << "-d -> debug, no optimisation" << std::endl;
	std::cout << "-a64 -> 64 bit architecture" << std::endl;
}

int main(int argc, char** argv)
{
//	system("cat ./Main.cpp");
	//std::string env = getenv("XTERM");
	putenv("TERM=rxvt-unicode");
//	std::cout << getenv("XTERM") << std::endl;
	bool interpret = false;
	bool singleStep = false;
	char* fileName = "";
	bool fileNameSet = false;
	bool keepASM = false;
	bool debug = false;
	bool is64 = false;
	for (int i = 1; i < argc; i++)
	{

		for (int j = 0; argv[i][j] != 0 && !fileNameSet; j++)
		{
			if (j != 0 && argv[i][0] != '-')
			{
				fileName = argv[i];
				fileNameSet = true;
				break;
			}
		}

		if (strcmp("-h", argv[i]) == 0)
		{
			printHelp();
			//Early return just to print help
			return 0;
		}
		else if (strcmp("-i", argv[i]) == 0)
		{
			interpret = true;			
		}
		else if (strcmp("-s", argv[i]) == 0)
		{
			singleStep = true;	
		}
		else if (strcmp("-k", argv[i]) == 0)
		{
			keepASM = true;
		}
		else if (strcmp("-d", argv[i]) == 0)
		{
			debug = true;
		}
		else if (strcmp("-a64", argv[i]) == 0)
		{
			is64 = true;
		}
	}
	
	if (!interpret)
	{
		BFCompiler compiler(fileName, keepASM, debug, is64);
		compiler.translate();
		compiler.assemble();

		if (compiler.succeeded())
		{
			std::cout << "Finished without Errors" << std::endl;
		}
	}
	else
	{
		bool exit = false;
		do
		{
			BFInterpreter interpreter(fileName, singleStep, debug);
			if (!interpreter.parseSuccess())
			{
				break;
			}
			interpreter.writeInstructions();
			interpreter.run();
			exit = interpreter.exit();
		} 
		while (!exit && singleStep);
	}

	return 0;
}
