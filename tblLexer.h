#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
using namespace std;

struct LexTok
{
	string lexeme;
	string token;
};

//columns = 0-alpha,1-digit,2-undrscr,3-period,4-grtr,5-less,6-equal,7-colon,8-semi,
//	9-comma,10-openP,11-closeP,12-plus,13-minus,14-times,15-div,16-eof,17-other
int findCol(char ch)
{
	char op[15] = {'_','.','>','<','=',':',';',',','(',')','+','-','*','/','"'};
	vector<char> ops ;//= {'_','.','>','<','=',':',';',',','(',')','+','-','*','/','"'};
	
	for(int i = 0; i < 15 ; ++i)
		ops.push_back(op[i]);

	if(isalpha(ch))
		return 0;
	if(isdigit(ch))
		return 1;
	if(ch == EOF)
	{
		return 17;
	}
	vector<char>::iterator it;
	it = find(ops.begin(),ops.end(),ch);
	if(it == ops.end())
		return 18;
	int pos = int(it - ops.begin())+2;
	return pos;
}

int lexSt[22][19] = {
/*					l   d   _	.	>	<	=	:	;	,	(	)	+	-	*	/	qts	eof othr bk
/*0 start*/			1,	3,	17,	16,	9,	10,	16,	13,	16,	16,	16,	16,	16,	16,	16,	16,	19,	18,	17,
/*1 in id */		1,	1,	1,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,
/*2 *end id */		0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	//y
/*3 in int */		4,	3,	4,	5,	4,	4,	4,	4,	4,	4,	4,	4,	4,	4,	4,	4,	4,	4,	4,
/*4 *end int*/		0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	//y
/*5 int . */		7,	6,	7,	7,	7,	7,	7,	7,	7,	7,	7,	7,	7,	7,	7,	7,	7,	7,	7,	
/*6 in real i.i*/	8,	6,	8,	8,	8,	8,	8,	8,	8,	8,	8,	8,	8,	8,	8,	8,	8,	8,	8,	
/*7 *int with . */	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	//yy	
/*8 *end real*/		0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	//y
/*9 grtr found*/	11,	11,	11,	11,	11,	11,	12,	11,	11,	11,	11,	11,	11,	11,	11,	11,	11,	11,	11,
/*10 less found*/	11,	11,	11,	11,	12,	11,	12,	11,	11,	11,	11,	11,	11,	11,	11,	11,	11,	11,	11,
/*11 *relOp-1ch*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	//y
/*12 *relOp-2ch*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	//n
/*13 colon found*/	15,	15,	15,	15,	15,	15,	14,	15,	15,	15,	15,	15,	15,	15,	15,	15,	15,	15,	15,
/*14 *colon equal*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	//n
/*15 *colon only*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	//y
/*16 *op - 1ch*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	//n
/*17 *unknwn */		0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	//n
/*18 *eof found*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	//n
/*19 in strConst*/	19,	19,	19,	19,	19,	19,	19,	19,	19,	19,	19,	19,	19,	19,	19,	19,	20,	21,	19,
/*20 *end strCnst*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	//n
/*21 *eof in str*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0 	//n
};

vector<string> keywords ;//= {"program","begin","end","function","read","write","if","elsif",
							//"else","while","do","until","return"};




// type names = int, real, string

struct LexTok getLexTok(ifstream & f)
{
	keywords.push_back("program");
	keywords.push_back("begin");
	keywords.push_back("end");
	keywords.push_back("function");
	keywords.push_back("read");
	keywords.push_back("write");
	keywords.push_back("if");
	keywords.push_back("elsif");
	keywords.push_back("else");
	keywords.push_back("while");
	keywords.push_back("do");
	keywords.push_back("until");
	keywords.push_back("return");

	LexTok cur;
	vector<string>::iterator it;
	int state = 0;
	int col;
	char ch;
	if( f >> ch)
	{
		f.putback(ch);  // making this act like peek()
		col = findCol(ch);
		state = lexSt[state][col];
		//cout << "next st: " << state << endl;
		while(state != 0)
		{
			switch(state)
			{
				case 2:		// end of identifier
						it = find(keywords.begin(),keywords.end(),cur.lexeme);
						if(it != keywords.end())
						{
							cur.token = "Keyword";
						}
						else if(cur.lexeme == "int"||cur.lexeme=="real"||cur.lexeme=="string")
						{
							cur.token = "Type";
						}
						else
						{
							cur.token = "Identifier";
						}
						return cur;
						break;
				case 4:		// end of integer
						cur.token = "IntConst";
						return cur;
						break;
				case 7:		// int followed by .
						f.putback('.');
						cur.token = "IntConst";
						return cur;
						break;						
				case 8:		// end of real
						cur.token = "RealConst";
						return cur;
						break;				
				case 11:	// single char relOp
						cur.token = "RelOp";
						return cur;
						break;				
				case 12:	// 2-char relOp
						ch = f.get();
						cur.lexeme += ch;
						cur.token = "RelOp";
						return cur;
						break;				
				case 14:	// :=			
						ch = f.get();
						cur.lexeme += ch;
						cur.token = "Operator";
						return cur;
						break;	
				case 15:	// : only
						cur.token = "Operator";
						return cur;
						break;					
				case 16:	// single char operator
						ch = f.get();
						cur.lexeme += ch;
						cur.token = "Operator";
						if(cur.lexeme == "=")
							cur.token = "RelOp";
						return cur;
						break;				
				case 17:	// unknown input, error
						ch = f.get();
						cur.lexeme += ch;
						cur.token = "error";
						return cur;
						break;							
				//case 18:	// eof found -- handled by else outside while loop
						// cur.lexeme = "";
						// cur.token = "eof";
						// return cur;
						// break;
				case 20:	// end strConst
						ch = f.get();
						cur.lexeme += ch;
						cur.token = "StrConst";
						return cur;
						break;
				case 21:	// eof in strConst - unmatched " "
						cur.token = "error";
			}
			ch = f.get();
			cur.lexeme += ch;
			//cout << cur.lexeme << endl;
			ch = f.peek();
			col = findCol(ch);
			state = lexSt[state][col];
			//cout << "next st: " << state << endl;
		}
	}
	else
	{
		cur.lexeme = "";
		cur.token = "eof";
		return cur;
	}
}




