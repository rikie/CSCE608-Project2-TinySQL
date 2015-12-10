#include<iostream>
#include<iterator>
#include<cstdlib>
#include<ctime>
#include<string>
#include<list>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <stack>
#include "parserLib.h"

void processNot();

using namespace std;

// function to strip " from "C" <string> in where condition
vector<string> splitString(string str, string delimiter);
// function to evaluate binary comparisons
bool evalBinaryArgs(double arg1, double arg2, string operArg) {
  if(operArg.compare("=") == 0)
    return arg1 == arg2;
  else if(operArg.compare(">") == 0)
    return arg1 > arg2;
  else if(operArg.compare("<") == 0)
    return arg1 < arg2;
}

bool evaluateBoolOperand(vector<string> operandVec) {
  double res1=0.0;
  double res2=0.0;
  bool foundOperator = false;
  string operArg;
  string evalStr1 = "(";
  string evalStr2 = "(";
  vector<string> tokens;
  vector<string> rpn;
  vector<string>::iterator it;
  if(operandVec.size() > 3) {
	for(it=operandVec.begin(); it!=operandVec.end(); ++it) {
	  //if((*it).compare("OR")
	  //cout << "Inside " << *it << endl;
	  if((*it).compare("=") == 0 || (*it).compare(">") == 0 || (*it).compare("<") == 0) {
	    foundOperator = true;
		operArg = *it;
		continue;
	  }
	  if(!foundOperator) {
		evalStr1 += *it;
	  }
	  if(foundOperator) {
	    evalStr2 += *it;
	  }
	}
	evalStr1 += ")";
	evalStr2 += ")";
	
	tokens = getExpressionTokens(evalStr1);
	if (infixToRPN(tokens, tokens.size(), rpn)) {       
      res1 = RPNtoDouble( rpn );        
      //cout << "Result = " << res1 << std::endl;        
  }        
  else {       
      cout << "Mis-match in parentheses" << std::endl;        
  }
	tokens.clear();
	rpn.clear();
    tokens = getExpressionTokens(evalStr2);
	if (infixToRPN(tokens, tokens.size(), rpn)) {       
      res2 = RPNtoDouble( rpn );        
      //cout << "Result = " << res2 << std::endl;        
  }        
  else {       
      cout << "Mis-match in parentheses" << std::endl;        
  }	
	
	
  //cout << "The eval string 1 is " << evalStr1 << endl;
  //cout << "The eval string 2 is " << evalStr2 << endl;
  //cout << "The operator is " << operArg << endl;
  //cout << "HAHAHAHA " << res1 << operArg << res2 << endl;
  }
  else {
    if((isalpha((operandVec[0]).at(0)) || operandVec[0].at(0) == '"')&& operandVec[1].compare("=") == 0) {
	  //cout << "Please please please" << operandVec[2] << " and " << operandVec[0] << endl;
	  //return operandVec[0].compare(splitString(operandVec[2], "\"")[0]) == 0 ? true : false ;
	  bool result = operandVec[0].compare(operandVec[2]) == 0 ? true : false;
	  //cout << "The danger result is " << result << endl;
	  return result;
	  }
    else {
      res1 = (double)atoi(operandVec[0].c_str());
	    res2 = (double)atoi(operandVec[2].c_str());
	    operArg = operandVec[1];
	//cout << "HAHAHAHA " << res1 << operArg << res2 << endl;
    }
  }
  return evalBinaryArgs(res1, res2, operArg);
}

// give arg2 as "unary" in case of NOT
bool evalBoolArgs(string arg1, string arg2, string operArg) {
  if (operArg.compare("NOT") == 0) {
    cout << "Inside the NOT operator in evalBoolArgs, arg1 is " << arg1 << endl;
    if(arg1.compare("true") == 0) return false;
    else return true;
  }
  else if(operArg.compare("AND") == 0) {
    if(arg1.compare("true") == 0 && arg2.compare("true") == 0) return true;
	else return false;
  }
  else if(operArg.compare("OR") == 0) {
    if(arg1.compare("false") == 0 && arg2.compare("false") == 0) return false;
	else return true;
  }
}

// implement the RPN evaluator here
bool evaluateBoolRPN(vector<string> rpnVec) {
  cout << "evaluateBoolRPN is called " << endl;
  vector<string>::iterator vit;
  stack<string> rpnEvalStack;
  bool result=false;
  string arg1, arg2, operArg;
  for(vit = rpnVec.begin(); vit != rpnVec.end(); ++vit) {
	if((*vit).compare("OR") != 0 && (*vit).compare("AND") != 0) {
	  rpnEvalStack.push(*vit);
	}
	else {
    if((*vit).compare("NOT") == 0) {
      arg1 = rpnEvalStack.top();
      rpnEvalStack.pop();
      operArg = "NOT";
      result = evalBoolArgs(arg1, "NOT", operArg);  
    }
	  else if((*vit).compare("OR") == 0 || (*vit).compare("AND") == 0) {
	    if((*vit).compare("OR") == 0) operArg = "OR";
		else operArg = "AND";
	    arg1 = rpnEvalStack.top();
	    rpnEvalStack.pop();
	    arg2 = rpnEvalStack.top();
	    rpnEvalStack.pop();
	    result = evalBoolArgs(arg1, arg2, operArg);
		//cout << arg1 << arg2 << " with " << operArg << " gave result " << result << endl;
	    if(result == true) rpnEvalStack.push("true");
	    else rpnEvalStack.push("false");	  
	  }
	}
	//result = evaluateBoolOperand(*vvit);
	//cout << "Returned here in evaluateBoolRPN" << *vit << endl;
  }
  //cout << " Size fo the stack is " << rpnEvalStack.size() << endl;
  if(rpnEvalStack.top().compare("true") == 0) { return true;}
  else { return false;}
}

int getPrecedence(string operArg) {
  if(operArg.compare("[") == 0 ) return -1;
  else if(operArg.compare("OR") == 0 ) return 0;
  else if(operArg.compare("AND") == 0 ) return 1;
  else if(operArg.compare("NOT") == 0 ) return 2;
}

// returns true if infix scanned element has higher precedence
bool hasPrecedence(string tos, string infix) {
  if (getPrecedence(infix) > getPrecedence(tos))
    return true;
  else
    return false;
}

void printVector(vector<string> vec) {
  vector<string>::iterator vit;
  for(vit=vec.begin(); vit != vec.end(); ++vit) {
    cout << "Vector Content is " << *vit << endl;
  }
}

void printStack(stack<string> stk) {
  for (stack<string> dump = stk; !dump.empty(); dump.pop()) {
    cout << dump.top() << endl;
  }
}

bool doesTupleSatisfyWhere(vector<string> condVec) {
  //cout << "doesTupleSatisfyWhere is called" << endl;
  bool foundNOT = false;
  bool result = false;
  vector<string> notVec;
  vector<string> rpnVec;
  vector<string> operandVec;
  stack<string> rpnStack;
  string notStr;
  //string condArr[] = {"C", "=", "C", "AND", "100", ">", "70", "AND", "100", ">", "80", "AND", "(", "100", "*", "30", "+", "100", "*", "20", "+", "100", "*", "50", ")", "/", "100", "<", "60"};
  //string condArr[] = {"A", "=", "C", "OR", "100", ">", "70", "AND", "100", ">", "80"};
  //vector<string> condVec(condArr, condArr +  sizeof(condArr) / sizeof(condArr[0]));

  vector<string>::iterator vit;
  vector<string>::iterator vit2;
  /*
  for(vit=condVec.begin(); vit != condVec.end(); ++vit) {
    cout << *vit << endl;
  } */
  for(vit=condVec.begin(); vit != condVec.end(); ++vit) {
    if((*vit).compare("OR") != 0 && (*vit).compare("AND") != 0 && (*vit).compare("[") != 0 && 
	(*vit).compare("]") != 0) {
	  operandVec.push_back(*vit);
	  //cout << endl << endl ;
	  //printVector(operandVec);
	  //cout << endl << endl ;
    }
    else {
	  //cout << "Operator is " << *vit << endl << endl;
	  if(operandVec.size() > 1) {
	    //cout << " The operandVec is " << endl;
	    //printVector(operandVec);
	    result = evaluateBoolOperand(operandVec);
	    if(result == true)
          rpnVec.push_back("true");
	    else
	      rpnVec.push_back("false");
        operandVec.clear();
	  }
	  //cout << "Printing rpn Vector" << endl;
	  //printVector(rpnVec);
	  //cout << "Printing the rpn Stack" << endl;
	  //printStack(rpnStack);
	  //cout << " The operator is " << *vit << endl;
      // check if stack is empty, no need to check operator precedence
      if(rpnStack.empty()) {
        rpnStack.push(*vit);
      }
      // else we have seen a operator check the top stack, pop it if necessary by checking precedence until no tos is problem
      //TODO the below is not necessary, if same precedence, then pop()?
	  //else if((*vit).compare(rpnStack.top()) == 0){
        //rpnStack.push(*vit);
      //}  
      else if((*vit).compare("[") == 0){
        rpnStack.push(*vit);
      }
      else if((*vit).compare("]") == 0) {
        while(!rpnStack.empty() && rpnStack.top().compare("[") != 0) {
		  rpnVec.push_back(rpnStack.top());
          rpnStack.pop();
        }
        rpnStack.pop();
      }
	  else if(hasPrecedence(rpnStack.top(), *vit)) {
	    rpnStack.push(*vit);
	  }
	  else if(!hasPrecedence(rpnStack.top(), *vit)) {
	    do {
          rpnVec.push_back(rpnStack.top());
          rpnStack.pop();	
		} while(!rpnStack.empty() && !hasPrecedence(rpnStack.top(), *vit));
		rpnStack.push(*vit);
      }
    }
  }
  // The last operand was never evaluated, so evaluate here
  if(operandVec.size() > 1) {
	result = evaluateBoolOperand(operandVec);
	if(result == true)
      rpnVec.push_back("true");
	else
	  rpnVec.push_back("false");
    operandVec.clear();
  }
  while(!rpnStack.empty()) {
    rpnVec.push_back(rpnStack.top());
    rpnStack.pop();
  }
  //cout << "The reverse polish notation is " << endl;
  /*
  for(vit = rpnVec.begin(); vit != rpnVec.end(); ++vit) {
		cout << *vit << " ";
  }
  */
  result = evaluateBoolRPN(rpnVec);
  return result;
}
