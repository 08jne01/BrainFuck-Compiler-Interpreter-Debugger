# BrainFuck-Compiler-Interpreter-Debugger
A compiler, interpreter and brainfuck debugger in one package.

You may need to use `chmod +x ./make` in order to be able to execute the make script.

## Requirements
  * g++ obviously

## Dependencies
  * ncurses to build and link. 
  * nasm to produce binaries. You can still use the interpreter without nasm

## Switches/Params

Usage: `./bfc <filename> [switches]`

`./bfc <filename>` will just compile the file you pass into a 32 bit (why 32bit? idk) binary

### Switches:
  * -h for help, this will stop the program from launching and just display the help.
  * -i to run the file passed in using the interpreter.
  * -s used with -i to run the debugger.
  * -k will keep the asm file that is made for the compilation process.
  * -d will not optimise the code. Will reduce weird code jumps when debugging but is slower of course.
