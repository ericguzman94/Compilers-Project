#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <deque>
#include <stack>

using namespace std;

#include "tblLexer.h"

// Global so all functions can use without passing them in param
LexTok curLexTok;

void LexMatch		(string s, string lexeme, ifstream &fin);
void TokMatch		(string s, string token, ifstream &fin);
void Program		(ifstream &fin);
void OptDeclList	(ifstream &fin);
void OptFuncList	(ifstream &fin);
void OptParamList	(ifstream &fin);
void OptStmtList	(ifstream &fin);
void OptParams		(ifstream &fin);
void OptVars		(ifstream &fin,string call);
void Decl			(ifstream &fin);
void VarList		(ifstream &fin,string call);
void FuncList		(ifstream &fin);
void Func			(ifstream &fin);
void ParamList		(ifstream &fin);
void Param			(ifstream &fin);
void Type			(ifstream &fin);
void StmtList		(ifstream &fin);
void Stmt			(ifstream &fin);
void Assign			(ifstream &fin);
void Read			(ifstream &fin);
void Write			(ifstream &fin);
void If				(ifstream &fin);
void While			(ifstream &fin);
void Do				(ifstream &fin);
void Return			(ifstream &fin);
void Expr			(ifstream &fin);
void OptExpr		(ifstream &fin);
void Cond			(ifstream &fin);
void RelOp			(ifstream &fin);
void EPrime			(ifstream &fin);
void TPrime			(ifstream &fin);
void Factor			(ifstream &fin);
void FuncCall		(ifstream &fin);
void OptArgList		(ifstream &fin);
void OptArgs		(ifstream &fin);
void Term			(ifstream &fin);
void OptElseIf		(ifstream &fin); 
void OptElse		(ifstream &fin);
void optimize();

bool isInList(void);

vector<string> vList; // vector that holds all declared variables
stack<string> regStack; // stack that holds avaliable registers

string instructionArray[150];	//Array to hold instructions.
int d = 0;	

struct varData
{
	string var;
	string regAssigned;
	bool isAssigned;
};

vector<struct varData> varReg;

//While-loop stacks:
stack <int> forwardStack;		//Stack to hold addresses to jump forward to, thus skipping a block of code.
stack <int> backwardStack;		//Stack to hold addresses to jump backward to, thus repeating a block of code.

//If/Ifelse/Else stacks:
stack <int> ifStack;			
stack <int> ifJUMPStack;

// gobal varibles to hold the registers being used currently
string reg;
string reg1;

string lhsReg;
string regDest;
string regOne;
string regTwo;

string prevLex;
string prevSymbol;

vector<string> instructionArr;

// gobal varibles to hold the relop being used currently
string relop;
string relOpMip;

// varibles to keep track of the amount of if statements, elseifs and elses.
int ifCount		= 0;
int elIfCount	= 1;
int elseCount	= 1;

// keep track of whiles
int whileCount = 0;

void Program(ifstream &fin)
{
    LexMatch("program", curLexTok.lexeme, fin);

	instructionArray[d] = "	.data" ;
	d++;

    OptDeclList(fin);
    OptFuncList(fin);


	instructionArray[d] =  "	.text"  ;
	d++;

    LexMatch("begin", curLexTok.lexeme, fin);

	// start of the program instuctions
	instructionArray[d] =  "main:" ;
	d++;

	OptStmtList(fin);
    LexMatch("end", curLexTok.lexeme, fin);
    LexMatch(".", curLexTok.lexeme, fin);

	// end of program instructions
	instructionArray[d] = "	li $v0 , 10";
	d++;
	instructionArray[d] = "	syscall";
	d++;

    return;
}

void LexMatch(string s, string currentLexeme, ifstream &fin)
{
    if(currentLexeme != s)
    {
        cout<<"ERROR: current lexeme ( " << currentLexeme << " ) does not match with \""<<s<<"\".  Exiting program."<<endl;
        exit(1);
    }
    else curLexTok = getLexTok(fin);
}

void TokMatch(string s, string currentToken, ifstream &fin)
{
    if(currentToken != s)
    {
        cout<<"ERROR: current token ("<<currentToken<<") does not match with \""<<s<<"\".  Exiting program."<<endl;
        exit(1);
    }
    else curLexTok = getLexTok(fin);
}

bool isInList(void)
{
	for(unsigned int index = 0; index < vList.size(); ++index)
		if(curLexTok.lexeme == vList[index])
			return true;
	return false;
}

void OptDeclList(ifstream &fin)
{
	
    if(curLexTok.lexeme == "function" || curLexTok.lexeme == "begin")
        return;

    Decl(fin);
    OptDeclList(fin);
    
    return;
}

void Decl(ifstream &fin)
{	
	string call = "decl";

	Type(fin);
	VarList(fin,call);
	LexMatch(";", curLexTok.lexeme, fin);
}

void OptFuncList(ifstream &fin)
{
	if(curLexTok.lexeme == "begin")
		return;

	Func(fin);
	OptFuncList(fin);
}

void Func(ifstream &fin)
{
	LexMatch("function", curLexTok.lexeme, fin);
    TokMatch("Identifier", curLexTok.token, fin);
    LexMatch("(", curLexTok.lexeme, fin);
    OptParamList(fin);
    LexMatch(")", curLexTok.lexeme, fin);
    LexMatch(":", curLexTok.lexeme, fin);
    Type(fin);
    LexMatch(";", curLexTok.lexeme, fin);
    OptDeclList(fin);
    LexMatch("begin", curLexTok.lexeme, fin);
    OptStmtList(fin);
    LexMatch("end", curLexTok.lexeme, fin);
}

void OptParamList(ifstream &fin) 
{
    if(curLexTok.lexeme == ")")
        return;
   
    Param(fin);
    OptParams(fin);
}

void OptParams(ifstream &fin) 
{
    if(curLexTok.lexeme == ")")
        return;

    LexMatch(",", curLexTok.lexeme, fin);
    Param(fin);
    OptParams(fin);
}

void Param(ifstream &fin)
{
    Type(fin);
    TokMatch("Identifier", curLexTok.token, fin);  
}

void Type(ifstream &fin)
{
	if( curLexTok.lexeme == "int" || curLexTok.lexeme == "real" || curLexTok.lexeme == "string" )
	{
		curLexTok = getLexTok(fin);
		return;
	}

	return;
}

void VarList(ifstream &fin,string call)
{
	// declare the varibles if the cal is decl and check if there are already decleared
	if (call=="decl")
	{
		if(isInList())
		{
			cout <<"Error: " << curLexTok.lexeme << " is declared more than once"<< endl;
			exit(1);
		}

		instructionArray[d] = curLexTok.lexeme + ":		.word 0";
		d++;

		varData data;
		data.regAssigned = regStack.top();
		regStack.pop();
		data.var = curLexTok.lexeme;
		varReg.push_back(data);
		data.isAssigned = false;

		vList.push_back(curLexTok.lexeme);
	}

	// read in varibales if the call is read and check if there are already decleared
	else if (call=="read")
	{
		if(isInList())
		{
			for(int unsigned index = 0; index < varReg.size() ; ++index)
			{
				if (curLexTok.lexeme == varReg[index].var)
				{
					reg = varReg[index].regAssigned;
					varReg[index].isAssigned = true;
				}
			}
			instructionArray[d] = "	move " + reg + " , $v0";
			d++;
		}
		else 
		{
			cout << "Error: Using " << curLexTok.lexeme << " without declaring first." << endl;
			exit(1);
		}
	}

	TokMatch("Identifier", curLexTok.token, fin);
	OptVars(fin,call);
}

void OptVars(ifstream &fin, string call)
{
    if(curLexTok.lexeme == ";" || curLexTok.lexeme == ")")
        return;

	LexMatch(",", curLexTok.lexeme, fin);
	
	if (call=="decl")
	{
		if(isInList())
		{
			cout <<"Error: " << curLexTok.lexeme << " is declared more than once"<< endl;
			exit(1);
		}

		instructionArray[d] = curLexTok.lexeme + ":		.word 0";
		d++;

		varData data;
		data.regAssigned = regStack.top();
		regStack.pop();
		data.var = curLexTok.lexeme;
		varReg.push_back(data);
		data.isAssigned = false;

		vList.push_back(curLexTok.lexeme);
	}
	else if (call=="read")
	{
		if(isInList())
		{
			instructionArray[d] = "	li $v0 , 5";
			d++;
			instructionArray[d] = "	syscall";
			d++;

			for(int unsigned index = 0; index < varReg.size() ; ++index)
			{
				if (curLexTok.lexeme == varReg[index].var)
				{
					reg = varReg[index].regAssigned;
					varReg[index].isAssigned = true;
				}
			}

			instructionArray[d] = "	move " + reg + " , $v0";
			d++;
		}
		else 
		{
			cout << "Error: Using " << curLexTok.lexeme << " without declaring first." << endl;
			exit(1);
		}
	}

    TokMatch("Identifier", curLexTok.token, fin);
	OptVars(fin, call);
}

void OptStmtList(ifstream &fin) 
{
	if(curLexTok.lexeme == "end" || curLexTok.lexeme == "until")
        return;

    Stmt(fin);
    OptStmtList(fin);
}

void Stmt(ifstream &fin)
{
	if(curLexTok.token == "Identifier")
		Assign(fin);

	else if(curLexTok.lexeme == "read")
		Read(fin);

	else if(curLexTok.lexeme == "write")
		Write(fin);

	else if(curLexTok.lexeme == "if")
		If(fin);
	
	else if(curLexTok.lexeme == "while")
		While(fin);

	else if(curLexTok.lexeme == "do")
		Do(fin);
	
	else if(curLexTok.lexeme == "return")
		Return(fin);

	else
    {
        cout << "Error: expected ident, read, write, while, do, if, return" << endl;
        exit(1);
    }
}


void If(ifstream &fin)
{
	int addressOfBranch;

	ifCount++;

	// get next lex tok pair
	curLexTok = getLexTok(fin);
	LexMatch("(",curLexTok.lexeme, fin);
	Cond(fin);
	LexMatch(")",curLexTok.lexeme, fin);
	
	instructionArray[d] = " Branch ???"; //Jump to next else-if or else statement if condition is false.
	ifStack.push(d);
	d++;

	string oldReg1 = reg1;
	string oldReg = reg;

	// push back register for others to use
	int firstRegCont = 0;
	int secondRegCount = 0;
	for(int unsigned index = 0; index < varReg.size() ; ++index)
	{
		if ( reg1 == varReg[index].regAssigned )
			firstRegCont++;
		if ( reg == varReg[index].regAssigned )
			secondRegCount++;
	}

	if ( secondRegCount == 0 )
		regStack.push(reg);
	if ( firstRegCont == 0 )
		regStack.push(reg1);

	LexMatch("begin",curLexTok.lexeme, fin);

	// if there is a nested if this keeps track of the registers and relops from before.
	int oldIfCount = ifCount;
	string oldRelopMip = relOpMip;

	Stmt(fin);
	OptStmtList(fin);
	LexMatch("end",curLexTok.lexeme, fin);
	
	// branch to endif if the next lexeme is else or elsif
	if (curLexTok.lexeme=="elsif"||curLexTok.lexeme=="else")
	{
		instructionArray[d] = "	b endif" + to_string(static_cast<long long>(oldIfCount)); 
		d++;
	}

	if (curLexTok.lexeme=="elsif")
	{
		//Patch addressOfBranch to skip over if-statement.
		addressOfBranch = ifStack.top();
		ifStack.pop();
		instructionArray[addressOfBranch] = "	" + oldRelopMip + " " + oldReg1 + " , " + oldReg + " , " + "elsif" + to_string(static_cast<long long>(elIfCount));
	}
	else if (curLexTok.lexeme=="else")
	{
		
		//Patch addressOfBranch to skip over if-statement.
		addressOfBranch = ifStack.top();
		ifStack.pop();
		instructionArray[addressOfBranch] = "	" + oldRelopMip + " " + oldReg1 + " , " + oldReg + " , " + "else" + to_string(static_cast<long long>(oldIfCount));
	}
	else
	{
		//Patch addressOfBranch to skip over if-statement.
		addressOfBranch = ifStack.top();
		ifStack.pop();
		instructionArray[addressOfBranch] = "	" + oldRelopMip + " " + oldReg1 + " , " + oldReg + " , " + "endif" + to_string(static_cast<long long>(ifCount));
	}

	elseCount = oldIfCount;

	OptElseIf(fin); 
	OptElse(fin);

	instructionArray[d] = "endif" + to_string(static_cast<long long>(oldIfCount))+":"; 
	d++;
}

void OptElseIf(ifstream &fin)
{
	int addressOfBranch;

	if( curLexTok.lexeme == "end" ||
		curLexTok.lexeme == "else" ||
		curLexTok.token == "Identifier" || 
		curLexTok.lexeme == "read" ||
		curLexTok.lexeme == "write" ||
		curLexTok.lexeme == "if" || 
		curLexTok.lexeme == "while" )
		return;

	elIfCount++;

	LexMatch("elsif",curLexTok.lexeme, fin);

	instructionArray[d] = "elsif" + to_string(static_cast<long long>(elIfCount-1))+":"; 
	d++;

	LexMatch("(",curLexTok.lexeme, fin);
	Cond(fin);

	string oldReg1 = reg1;
	string oldReg = reg;

	LexMatch(")",curLexTok.lexeme, fin);
	if( curLexTok.lexeme == ")")
		curLexTok = getLexTok(fin);
	LexMatch("begin",curLexTok.lexeme, fin);

	// push back register for others to use
	int firstRegCont = 0;
	int secondRegCount = 0;
	for(int unsigned index = 0; index < varReg.size() ; ++index)
	{
		if ( reg1 == varReg[index].regAssigned )
			firstRegCont++;
		if ( reg == varReg[index].regAssigned )
			secondRegCount++;
	}

	if ( secondRegCount == 0 )
		regStack.push(reg);
	if ( firstRegCont == 0 )
		regStack.push(reg1);

	//Branch this if the condition is false.
	instructionArray[d] = "	" + relOpMip + " " + oldReg1 + " , " + oldReg + " , " + "endif" + to_string(static_cast<long long>(elIfCount));; //Jump to next else-if or else statement if condition is false.
	ifStack.push(d);
	d++;

	string oRelopMip = relOpMip;
	int OElIfCount = elIfCount;

	Stmt(fin);
	OptStmtList(fin);
	LexMatch("end",curLexTok.lexeme, fin);

	// branch to endif if the next lexeme is else or elsif
	if (curLexTok.lexeme=="elsif"||curLexTok.lexeme=="else")
	{
		instructionArray[d] = "	b endif" + to_string(static_cast<long long>(elseCount)); 
		d++;
	}

	if (curLexTok.lexeme=="elsif")
	{
		//Patch JMPZ to skip over if-statement.
		addressOfBranch = ifStack.top();
		ifStack.pop();
		instructionArray[addressOfBranch] = "	" + oRelopMip + " " + oldReg1 + " , " + oldReg + " , " + "elsif" + to_string(static_cast<long long>(elIfCount));
	}
	else if (curLexTok.lexeme=="else")
	{
		//Patch JMPZ to skip over if-statement.
		addressOfBranch = ifStack.top();
		ifStack.pop();
		instructionArray[addressOfBranch] = "	" + oRelopMip + " " + oldReg1 + " , " + oldReg + " , " + "else" + to_string(static_cast<long long>(elseCount));
	}
    else
	{
		//Patch addressOfBranch to skip over if-statement.
		addressOfBranch = ifStack.top();
		ifStack.pop();
		instructionArray[addressOfBranch] = "	" + oRelopMip + " " + oldReg1 + " , " + oldReg + " , " + "endif" + to_string(static_cast<long long>(OElIfCount-1));
	}

	OptElseIf(fin); 
}

void OptElse(ifstream &fin)
{
	if(curLexTok.lexeme == "end" || 
		curLexTok.token == "Identifier" || 
		curLexTok.lexeme == "read" ||
		curLexTok.lexeme == "write" ||
		curLexTok.lexeme == "if" || 
		curLexTok.lexeme == "while"||
		curLexTok.lexeme == "do" ||
		curLexTok.lexeme == "return" )
		return;

	LexMatch("else", curLexTok.lexeme, fin);

	instructionArray[d] = "else" + to_string(static_cast<long long>(elseCount))+":";
	d++;

	LexMatch("begin", curLexTok.lexeme, fin);
	Stmt(fin);
	OptStmtList(fin);
	LexMatch("end", curLexTok.lexeme, fin);

	// else can not be follwed by another else
	if( curLexTok.lexeme == "else" )
	{
		cout <<"Error: only one ELSE is allowed. Exiting program." << endl;
		exit(1);
	}
}

void optimize()
{
	string operand;
	string firstRegstr;
	string secondRegstr;
	for(int index = d ; index >= 0 ; --index)
	{
		operand = instructionArray[index].substr(0,3);
		firstRegstr = instructionArray[index].substr(0,3);
		//secondRegstr = instructionArray[index].substr(7);
		if (operand == "mov")
		{
			string operand1 = instructionArray[index - 1].substr(0,3);
			string firstRegstr1 = instructionArray[index - 1].substr(4,3);
			//string secondRegstr1 = instructionArray[index - 1].substr(7);
	
			if (operand1 == "add" || operand1 == "sub")
			{
				string newInstruction = operand1 + firstRegstr1;
				instructionArr.push_back(newInstruction);
			}
			else if (operand1 == "mfl")
			{
				string newInstruction = "mflo"+ firstRegstr1;
				instructionArr.push_back(newInstruction);
			}
		}
	 string newInstruction = operand + firstRegstr + secondRegstr;
	 instructionArr.push_back(newInstruction);
	}
}

void Do(ifstream &fin)
{
	// get next lex tok pair
	curLexTok = getLexTok(fin);

	OptStmtList(fin);
	LexMatch("until",curLexTok.lexeme, fin);
	LexMatch("(",curLexTok.lexeme, fin);
	Cond(fin);
	LexMatch(")",curLexTok.lexeme, fin);
	LexMatch(";",curLexTok.lexeme, fin);

	//cout << "Do => do [StmtList] untill ( Cond ) ; " << endl;
}

void Return(ifstream &fin)
{
	// get next lex tok pair
	curLexTok = getLexTok(fin);

	Expr(fin);
	LexMatch(";",curLexTok.lexeme, fin);

	//cout << "Return => return Expr ;" << endl;
}

void Assign(ifstream &fin)
{
	string ident;
	int indexLoc;

	// Error check
	if(isInList())
	{
		for(unsigned int index = 0; index < vList.size(); ++index)
			if(curLexTok.lexeme == vList[index])
				ident = vList[index];			
	}
	else
	{
		cout << "Error: Using " << curLexTok.lexeme << " without declaring first." << endl;
		exit(1);
	}

	for(int unsigned index = 0; index < varReg.size() ; ++index)
	{
		if (curLexTok.lexeme == varReg[index].var)
		{
			lhsReg = varReg[index].regAssigned;
			indexLoc = index;
		}
	}

	// get next lex tok pair
	curLexTok = getLexTok(fin);

	prevLex = curLexTok.lexeme;

	LexMatch(":=", curLexTok.lexeme, fin);
	Expr(fin);
	LexMatch(";",curLexTok.lexeme, fin);
	
	// update the variable to initalized
	varReg[indexLoc].isAssigned = true;
}

void Read(ifstream &fin)
{
	string call = "read";
	
	instructionArray[d] = "	li $v0 , 5";
	d++;
	instructionArray[d] = "	syscall";
	d++;

	// get next lex tok pair
	curLexTok = getLexTok(fin);
	LexMatch("(", curLexTok.lexeme, fin);
	VarList(fin, call);
	LexMatch(")", curLexTok.lexeme, fin);
	LexMatch(";", curLexTok.lexeme, fin);
}

void Write(ifstream &fin)
{
	// get next lex tok pair
	curLexTok = getLexTok(fin);
	LexMatch("(", curLexTok.lexeme, fin);
	Expr(fin);

	// Push Wrte Intructions into Array
	instructionArray[d] = "	li $v0 , 1" ;
	d++;
	instructionArray[d] = "	move $a0 , " + reg;
	d++;
	instructionArray[d] = "	syscall";
	d++;

	// push back register for others to use
	int firstRegCont = 0;
	for(int unsigned index = 0; index < varReg.size() ; ++index)
		if ( reg == varReg[index].regAssigned )
			firstRegCont++;
	
	if ( firstRegCont == 0 )
		regStack.push(reg);

	OptExpr(fin);
	LexMatch(")", curLexTok.lexeme, fin);
	LexMatch(";", curLexTok.lexeme, fin);
}

void OptExpr(ifstream &fin)
{
	if(curLexTok.lexeme == ")")
        return;

    LexMatch(",", curLexTok.lexeme, fin);
	Expr(fin);

	// Push Wrte Intructions into Array
	instructionArray[d] = "	li $v0 , 1" ;
	d++;
	instructionArray[d] = "	move $a0 , " + reg;
	d++;
	instructionArray[d] = "	syscall";
	d++;

	// push back register for others to use
	int firstRegCont = 0;
	for(int unsigned index = 0; index < varReg.size() ; ++index)
		if ( reg == varReg[index].regAssigned )
			firstRegCont++;
	
	if ( firstRegCont == 0 )
		regStack.push(reg);

	OptExpr(fin);
}

void While(ifstream &fin)
{
	whileCount++;

	// Put in lable
	instructionArray[d] = "while"+ to_string(static_cast<long long>(whileCount))+":";
	d++;

	// locations
	int jumpBack;
	int jmpzAddress;

	// get next lex tok pair
	curLexTok = getLexTok(fin);

	LexMatch("(", curLexTok.lexeme, fin);
	Cond(fin);

	// get the registers to finish the patch later 
	string oldReg1 = reg1;
	string oldReg = reg;

	LexMatch(")", curLexTok.lexeme, fin);
	LexMatch("begin", curLexTok.lexeme, fin);

	// push back register for others to use
	int firstRegCont = 0;
	int secondRegCount = 0;
	for(int unsigned index = 0; index < varReg.size() ; ++index)
	{
		if ( reg1 == varReg[index].regAssigned )
			firstRegCont++;
		if ( reg == varReg[index].regAssigned )
			secondRegCount++;
	}

	if ( secondRegCount == 0 )
		regStack.push(reg);
	if ( firstRegCont == 0 )
		regStack.push(reg1);


	int oldWhileCount = whileCount;
	string oldRelOp = relOpMip;

	//The GOTO will go here.  This is the point where the loop statements execute.  If the condition was false,
	//we want to skip over these lines.
	instructionArray[d] = " BRANCH ???";
	forwardStack.push(d);
	d++;

	OptStmtList(fin);
	LexMatch("end", curLexTok.lexeme, fin);

	// branch back to while
	instructionArray[d] = "	b while"+ to_string(static_cast<long long>(oldWhileCount));
	d++;

	// endwhile
	jumpBack = backwardStack.top();
	backwardStack.pop();
	instructionArray[d] = "endwhl" + to_string(static_cast<long long>(oldWhileCount)) +":";
	d++;

	// patch 
	jmpzAddress = forwardStack.top();
	forwardStack.pop();
	instructionArray[jmpzAddress] = "	" + oldRelOp + " " + oldReg1 + " , "+ oldReg + " , " +" endwhl" + to_string(static_cast<long long>(oldWhileCount));
}

void Cond(ifstream &fin)
{

	Expr(fin);

	// used to save the first register for ifs and whiles
	reg1 = reg;

	// used for the while
	backwardStack.push(d);

	RelOp(fin);

	Expr(fin);

	if(relop == ">")
		relOpMip = "ble";
	
	else if(relop == "<")
		relOpMip = "bge";
	
	else if(relop == "=")
		relOpMip = "bne";
	
	else if(relop == ">=")
		relOpMip = "blt";
	
	else if(relop == "<=")
		relOpMip = "bgt";

	else if(relop == "<>")
		relOpMip = "beq";
	
}

void RelOp(ifstream &fin)
{
	if(	curLexTok.token == "RelOp")
	{
		relop = curLexTok.lexeme;
		curLexTok = getLexTok(fin);
	}
	else
	{
		cout<<"ERROR: Expected a relational operator, but received ["<<curLexTok.lexeme<<"]"<<endl;
		exit(1);
	}
}

void Expr(ifstream &fin)
{
    Term(fin); 
    EPrime(fin);
}

void EPrime(ifstream &fin)
{
    if(curLexTok.lexeme == ";" || 
		curLexTok.lexeme == ")" ||
		curLexTok.lexeme == "<" ||
		curLexTok.lexeme == ">" ||
		curLexTok.lexeme == "<=" ||
		curLexTok.lexeme == ">=" ||
		curLexTok.lexeme == "=" ||
		curLexTok.lexeme == "," ||
		curLexTok.lexeme == "<>")
		return;

	else if(curLexTok.lexeme == "+")
	{
		prevLex = curLexTok.lexeme;
		curLexTok = getLexTok(fin);

		// used to save the first register
		reg1 = reg; 
		string firstReg  = reg;

		Term(fin);

		// second
		string secondReg = reg;

		if (curLexTok.lexeme == ";")
		{

			// add
			instructionArray[d] = "	add " + lhsReg + " , " + firstReg + " , " + secondReg + "";
			d++;

			int firstRegCont = 0;
			int secondRegCount = 0;
			for(int unsigned index = 0; index < varReg.size() ; ++index)
			{
				if ( firstReg == varReg[index].regAssigned )
					firstRegCont++;
				if ( secondReg == varReg[index].regAssigned )
					secondRegCount++;
			}

			if ( firstRegCont == 0 )
				regStack.push(firstReg);
			if ( secondRegCount == 0 )
				regStack.push(secondReg);

			reg = lhsReg;

			prevSymbol = "+";
			EPrime(fin);		
		}
		else
		{
			string regValue = regStack.top();
			regStack.pop();

			// add
			instructionArray[d] = "	add " + regValue + " , " + firstReg + " , " + secondReg + "";
			d++;

			// push back register for others to use
			int firstRegCont = 0;
			int secondRegCount = 0;
			for(int unsigned index = 0; index < varReg.size() ; ++index)
			{
				if ( firstReg == varReg[index].regAssigned )
					firstRegCont++;
				if ( secondReg == varReg[index].regAssigned )
					secondRegCount++;
			}

			if ( firstRegCont == 0 )
				regStack.push(firstReg);
			if ( secondRegCount == 0 )
				regStack.push(secondReg);

			// update the global reg 
			reg = regValue;

			prevSymbol = "+";
			EPrime(fin);
		}
	}
	else if(curLexTok.lexeme == "-")
	{
		// used to save the first register
		string firstReg = reg;

		prevLex = curLexTok.lexeme;

		curLexTok = getLexTok(fin);

		Term(fin);

		// second
		string secondReg = reg;

		if (curLexTok.lexeme == ";")
		{
			// sub
			instructionArray[d] = "	sub " + lhsReg + " , " + firstReg + " , " + secondReg + "";
			d++;

			// push back register for others to use
			int firstRegCont = 0;
			int secondRegCount = 0;
			for(int unsigned index = 0; index < varReg.size() ; ++index)
			{
				if ( firstReg == varReg[index].regAssigned )
					firstRegCont++;
				if ( secondReg == varReg[index].regAssigned )
					secondRegCount++;
			}

			if ( firstRegCont == 0 )
				regStack.push(firstReg);
			if ( secondRegCount == 0 )
				regStack.push(secondReg);

			// update the global reg 
			reg = lhsReg;

			prevSymbol = "-";
			EPrime(fin);
		}
		else
		{
			// sub
			string regValue = regStack.top();
			regStack.pop();

			instructionArray[d] = "	sub " + regValue + " , " + firstReg + " , " + secondReg + "";
			d++;

			// push back register for others to use
			int firstRegCont = 0;
			int secondRegCount = 0;
			for(int unsigned index = 0; index < varReg.size() ; ++index)
			{
				if ( firstReg == varReg[index].regAssigned )
					firstRegCont++;
				if ( secondReg == varReg[index].regAssigned )
					secondRegCount++;
			}

			if ( firstRegCont == 0 )
				regStack.push(firstReg);
			if ( secondRegCount == 0 )
				regStack.push(secondReg);

			// update the global reg 
			reg = regValue;

			prevSymbol = "-";
			EPrime(fin);
		}
	}
	else
	{
		cout << "ERROR: Expected + or -, but received [" << curLexTok.lexeme<<"]" << endl;
		exit(1);
	}
}

void Term(ifstream &fin) 
{
    Factor(fin);
    TPrime(fin);
}

void TPrime(ifstream &fin)
{
    if(curLexTok.lexeme == "+" || 
		curLexTok.lexeme == "-" || 
		curLexTok.lexeme == ";" || 
		curLexTok.lexeme == ")" ||
		curLexTok.lexeme == "<" ||
		curLexTok.lexeme == ">" ||
		curLexTok.lexeme == "<=" ||
     	curLexTok.lexeme == ">=" ||
		curLexTok.lexeme == "=" ||
		curLexTok.lexeme == "," ||
		curLexTok.lexeme == "<>")
		return;
	
	else if(curLexTok.lexeme == "*")
	{
		prevLex = curLexTok.lexeme;
		curLexTok = getLexTok(fin);

		// used to save the first register
		string firstReg = reg;

		Factor(fin);

		// save the second variable
		string secondReg = reg;

		if(curLexTok.lexeme == ";")
		{
			// multiply
			instructionArray[d] = "	mult " + firstReg + " , " + secondReg;
			d++;

			if (prevSymbol == "+" || prevSymbol == "-" )
			{
				reg1 = regStack.top();
				regStack.pop();

				instructionArray[d] = "	mflo " + reg1;
				d++;

				reg = reg1;
			}
			else
			{
				instructionArray[d] = "	mflo " + lhsReg;
				d++;

				reg = lhsReg;
			}

			// push back register for others to use
			int firstRegCont = 0;
			int secondRegCount = 0;
			for(int unsigned index = 0; index < varReg.size() ; ++index)
			{
				if ( firstReg == varReg[index].regAssigned )
					firstRegCont++;
				if ( secondReg == varReg[index].regAssigned )
					secondRegCount++;
			}

			if ( firstRegCont == 0 )
				regStack.push(firstReg);
			if ( secondRegCount == 0 )
				regStack.push(secondReg);

			prevSymbol = "*";

			TPrime(fin);
		}
		else
		{
			// multiply
			instructionArray[d] = "	mult " + firstReg + " , " + secondReg;
			d++;

			// push back register for others to use
			int firstRegCont = 0;
			int secondRegCount = 0;
			for(int unsigned index = 0; index < varReg.size() ; ++index)
			{
				if ( firstReg == varReg[index].regAssigned )
					firstRegCont++;
				if ( secondReg == varReg[index].regAssigned )
					secondRegCount++;
			}

			if ( firstRegCont == 0 )
				regStack.push(firstReg);
			if ( secondRegCount == 0 )
				regStack.push(secondReg);

			reg1 = regStack.top();
			regStack.pop();

			instructionArray[d] = "	mflo " + reg1;
			d++;

			// update the global reg 
			reg = reg1;

			prevSymbol = "*";

			TPrime(fin);
		}
	}
	else if(curLexTok.lexeme == "/")
	{
		prevLex = curLexTok.lexeme;

		curLexTok = getLexTok(fin);

		// used to save the first register
		string firstReg = reg;

		Factor(fin);

		// save the second variable
		string secondReg = reg;

		if(curLexTok.lexeme == ";")
		{
			// multiply
			instructionArray[d] = "	div " + firstReg + " , " + secondReg;
			d++;

			if (prevSymbol == "+" || prevSymbol == "-" )
			{
				reg1 = regStack.top();
				regStack.pop();

				instructionArray[d] = "	mflo " + reg1;
				d++;

				reg = reg1;
			}
			else
			{
				instructionArray[d] = "	mflo " + lhsReg;
				d++;

				reg = lhsReg;
			}

			// push back register for others to use
			int firstRegCont = 0;
			int secondRegCount = 0;
			for(int unsigned index = 0; index < varReg.size() ; ++index)
			{
				if ( firstReg == varReg[index].regAssigned )
					firstRegCont++;
				if ( secondReg == varReg[index].regAssigned )
					secondRegCount++;
			}

			if ( firstRegCont == 0 )
				regStack.push(firstReg);
			if ( secondRegCount == 0 )
				regStack.push(secondReg);

			prevSymbol = "/";

			TPrime(fin);
		}
		else
		{

			// divide
			instructionArray[d] = "	div " + firstReg + " , " + secondReg;
			d++;

			// push back register for others to use
			int firstRegCont = 0;
			int secondRegCount = 0;
			for(int unsigned index = 0; index < varReg.size() ; ++index)
			{
				if ( firstReg == varReg[index].regAssigned )
					firstRegCont++;
				if ( secondReg == varReg[index].regAssigned )
					secondRegCount++;
			}

			if ( firstRegCont == 0 )
				regStack.push(firstReg);
			if ( secondRegCount == 0 )
				regStack.push(secondReg);

			reg1 = regStack.top();
			regStack.pop();

			instructionArray[d] = "	mflo " + reg1;
			d++;

			// update the global reg 
			reg = reg1;

			TPrime(fin);

			prevSymbol = "/";
		}

	}
	else
	{
		cout<<"ERROR: Expected a '*' or '/' (if factor) or  but received ["<<curLexTok.lexeme<<"]\n";
		exit(1);
	}
}

void Factor(ifstream &fin)
{
	if(curLexTok.token == "Identifier")
	{
		string value = curLexTok.lexeme;
	
		// check if variable is in the list
		if (isInList())
		{

			for(int unsigned index = 0; index < varReg.size() ; ++index)
			{
				if (curLexTok.lexeme == varReg[index].var)
				{
					reg = varReg[index].regAssigned;

					// check to see if the var is initialized
					if ( varReg[index].isAssigned != true )
					{
						cout << "Error: Using variable '" << curLexTok.lexeme << "' before initializing." << endl;
						exit(1);
					}
				}
			}

			curLexTok = getLexTok(fin);

			// spacial case where a var eqauls another var: 
			// example a := b;
		    if(curLexTok.lexeme == ";" && prevLex == ":=")
			{
				instructionArray[d] = "	move " + lhsReg + " , " + reg;
				d++;
			}
		}
		else
		{
			// Error check is variable has not been declared
			cout << "Error: Using " << curLexTok.lexeme << " without declaring first." << endl;
			exit(1);
		}

        if(curLexTok.lexeme == "(")
        {
            curLexTok = getLexTok(fin);
            FuncCall(fin);
			return;
        }

		return;
	}
	else if(curLexTok.token == "IntConst") 
	{
		string value = curLexTok.lexeme;
		curLexTok = getLexTok(fin);

		// handels initalizing a integer to a var 
		// ex a := 5;
		if(curLexTok.lexeme == ";" && prevLex == ":=")
		{
			instructionArray[d] = "	li " + lhsReg + " , " + value;
			d++;
		}
		else
		{
			// get a register
			reg = regStack.top();

			// load constant into register
			instructionArray[d] = "	li " + reg + " , " + value;
			d++;

			// pop register off the stack
			regStack.pop();
		}

		// update prevLex
		prevLex = curLexTok.lexeme;

		return;
	}
	else if(curLexTok.lexeme == "(") 
	{
		LexMatch("(", curLexTok.lexeme, fin);
		Expr(fin);
		LexMatch(")", curLexTok.lexeme, fin);
		return;
	}
	else if(curLexTok.token == "RealConst") 
	{
		curLexTok = getLexTok(fin);
		return;
	}
	else if(curLexTok.token == "StrConst") 
	{
		curLexTok = getLexTok(fin);
		return;
	}
	else
    {
        cout << "ERROR: Expected ident, int, real, string or (Expr) Received [" << curLexTok.lexeme << "] "<< endl;
		exit(1);
    }
}

void FuncCall(ifstream &fin)
{
    OptArgList(fin);
    LexMatch(")", curLexTok.lexeme, fin);
}

void OptArgList(ifstream &fin) 
{
    if (curLexTok.lexeme == ")")
        return;
 
    Expr(fin);
    OptArgs(fin);
}

void OptArgs(ifstream &fin)
{
    if (curLexTok.lexeme == ")")
        return;

    LexMatch(",", curLexTok.lexeme, fin);
    Expr(fin);
    OptArgs(fin);
}
