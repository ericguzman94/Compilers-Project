/*
	CPSC 323 - Fall 2017
	Program #4 -Optimized
	using visual studio 2010

	Eric Guzman
	Michael Perez

*/

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <deque>
#include <stack>

using namespace std;

#include "parser.h"

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		cout << "missing file name" << endl;
		return 1;
	}

	ifstream fin(argv[1]);

	if (!fin)
	{
		cout << "file not found" << endl;
		return 1;
	}

	// push in resisters to stack so parser can use
	regStack.push("$t9");
	regStack.push("$t8");
	regStack.push("$t7");
	regStack.push("$t6");
	regStack.push("$t5");
	regStack.push("$t4");
	regStack.push("$t3");
	regStack.push("$t2");
	regStack.push("$t1");
	regStack.push("$t0");

	// get pair to start parser
	curLexTok = getLexTok(fin);

	// parser start
	Program(fin);

	fin.close();

	// output MIPS instructions
    for(int a = 0; a < d; a++)
	{
		cout << instructionArray[a] <<endl;
	}

	return 0;
}