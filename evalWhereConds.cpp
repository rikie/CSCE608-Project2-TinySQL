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

// print a string vector
void printMyVector(vector<string> vstr) {
  vector<string>::iterator vit;
  cout << "Printing the vector string" << endl;
  for(vit = vstr.begin(); vit != vstr.end(); ++vit) {
    cout << *vit << " " << endl;
  }
  cout << endl;
}

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
  bool foundBraceLeftStr1 = false;
  bool foundBraceLeftStr2 = false;
  bool foundBraceRightStr1 = false;
  bool foundBraceRightStr2 = false;
  if(operandVec.size() > 3 && operandVec[1].at(0) != '"') {
  //if(!isalpha((operandVec[0]).at(0)) && operandVec[0].compare("(") != 0) {
	for(it=operandVec.begin(); it!=operandVec.end(); ++it) {
	  if((*it).compare("=") == 0 || (*it).compare(">") == 0 || (*it).compare("<") == 0) {
	    foundOperator = true;
		operArg = *it;
		continue;
	  }
	  if((*it).compare("(") == 0 && !foundOperator) foundBraceLeftStr1 = true;
	  if((*it).compare(")") == 0 && !foundOperator) {
	    if(foundBraceLeftStr1) foundBraceLeftStr1 = false;
	    else foundBraceRightStr1 = true;
	  }
	  if((*it).compare("(") == 0 && foundOperator) foundBraceLeftStr2 = true;
	  if((*it).compare(")") == 0 && foundOperator) {
	    if(foundBraceLeftStr2) foundBraceLeftStr2 = false;
	    else foundBraceRightStr2 = true;
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
	if(foundBraceLeftStr1) evalStr1 += ")";
	if(foundBraceLeftStr2) evalStr2 += ")";
	if(foundBraceRightStr1) evalStr1 = "(" + evalStr1;
	if(foundBraceRightStr2) evalStr2 = "(" + evalStr2;
	//cout << "evalStr1 is " << evalStr1 << endl;
	//cout << "evalStr2 is " << evalStr2 << endl;
	tokens = getExpressionTokens(evalStr1);
	if (infixToRPN(tokens, tokens.size(), rpn)) {       
      res1 = RPNtoDouble( rpn );               
  }        
  else {       
    cout << "Mis-match in parentheses" << std::endl;        
  }
	tokens.clear();
	rpn.clear();
    tokens = getExpressionTokens(evalStr2);
	if (infixToRPN(tokens, tokens.size(), rpn)) {       
    res2 = RPNtoDouble( rpn );   
  }        
  else {       
    cout << "Mis-match in parentheses" << std::endl;        
    }	
  }
  else {
    if(operandVec.size() == 3 && operandVec[0].at(0) == '"') {
	  //cout <<  "The operand[0] is " << operandVec[0] << endl;
	  //cout <<  "The operand[2] is " << operandVec[2] << endl;
	  bool result = operandVec[0].compare(operandVec[2]) == 0 ? true : false;
	  //cout << "Result is " << result << endl;
	  return result;
	}
	else if(operandVec.size() == 3 && !isalpha((operandVec[0]).at(0))) {
	  res1 = (double)atoi(operandVec[0].c_str());
	  res2 = (double)atoi(operandVec[2].c_str());
	  operArg = operandVec[1];
	}
	else
	  return operandVec[1].compare(operandVec[3]) == 0 ? true : false;
	}
  /*
    cout << "The operand[0] is " << operandVec[0] << endl;
    if((isalpha((operandVec[0]).at(0)) || operandVec[0].at(0) == '"')&& operandVec[1].compare("=") == 0) {
	  bool result = operandVec[0].compare(operandVec[2]) == 0 ? true : false;
	  return result;
	  }
    else {
      res1 = (double)atoi(operandVec[0].c_str());
	    res2 = (double)atoi(operandVec[2].c_str());
	    operArg = operandVec[1];
    }
  }
  */
  return evalBinaryArgs(res1, res2, operArg);
}

bool evalBoolArgs(string arg1, string arg2, string operArg) {
  if (operArg.compare("NOT") == 0) {
    //cout << "Inside the NOT operator in evalBoolArgs with arg " << arg1 << endl;
    if(arg1.compare("true") == 0) return false;
    else return true;
  }
  else if(operArg.compare("AND") == 0) {
    //cout << "Processing the AND statement with " << arg1 << " and " << arg2 << endl;
    if(arg1.compare("true") == 0 && arg2.compare("true") == 0) return true;
	else return false;
  }
  else if(operArg.compare("OR") == 0) {
    //cout << "Processing the OR statement with " << arg1 << " and " << arg2 << endl;
    if(arg1.compare("false") == 0 && arg2.compare("false") == 0) return false;
	else return true;
  }
}

// implement the Reverse Polish Notation evaluator here
bool evaluateBoolRPN(vector<string> rpnVec) {
  //cout << "evaluateBoolRPN is called " << endl;
  vector<string>::iterator vit;
  stack<string> rpnEvalStack;
  bool result=false;
  string arg1, arg2, operArg;
  for(vit = rpnVec.begin(); vit != rpnVec.end(); ++vit) {
	  if((*vit).compare("OR") != 0 && (*vit).compare("AND") != 0 && (*vit).compare("NOT") !=0) {
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
	    } 
      if(result == true) rpnEvalStack.push("true");
      else rpnEvalStack.push("false");
	  }
	//result = evaluateBoolOperand(*vvit);
	//cout << "Returned here in evaluateBoolRPN" << *vit << endl;
  }
  //cout << " Size of the stack is " << rpnEvalStack.size() << endl;
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
/**
void printVector(vector<string> vec) {
  vector<string>::iterator vit;
  for(vit=vec.begin(); vit != vec.end(); ++vit) {
    cout << "Vector Content is " << *vit << endl;
  }
}
*/
void printStack(stack<string> stk) {
  for (stack<string> dump = stk; !dump.empty(); dump.pop()) {
    cout << dump.top() << endl;
  }
}

// Function to extract the join conditions from the condition Vector
vector<string> getJoinConditions (vector<string> &condVec){
  vector<string>::iterator vit;
  vector<string>tempVec,tempVec2;
  vector<string>joinConditions;
  for (vit=condVec.begin();vit<condVec.end();++vit)
  {
    //check to see if we have found an attribute name
    if(isalpha((*vit).at(0)) && (*vit).compare("OR") !=0 && (*vit).compare("AND") !=0
      && (*vit).compare("NOT") !=0 && (vit+1)<condVec.end() && (*vit).compare("true") !=0
      && (*vit).compare("false") !=0) 
    {
      //cout << *vit << endl;
      //we have found an attribute name; need to see if it contains a relation name
      tempVec = splitString(*vit, ".");
      if (tempVec.size() > 1)
      {
        //we have found an relation name and an attribute name
        //cout << tempVec[0] << endl;
        //see if this is part of a comparison between attributes for two different relations
        /**
        if ((*(vit+1)).compare("=") == 0 || 
          (*(vit+1)).compare("<") == 0 ||
          (*(vit+1)).compare(">") == 0)
        */
        if ((*(vit+1)).compare("=") == 0)
        {
          //we have a comparison operator; check to see if the next term is another attribute
          //cout << " test " << *(vit+2) << endl;
          tempVec2 = splitString(*(vit+2), ".");
          //cout << tempVec2[0] << endl;
          if (tempVec2.size() > 1 && tempVec[0].compare(tempVec2[0]) != 0)
          {
            //we have found an attribute from a different table.
            string foundCondition = *vit + " " + *(vit+1) + " " + *(vit+2);
            //cout << "Found at: " << vit - condVec.begin() << endl;
            int joinPos = vit - condVec.begin();
            condVec[joinPos] = "true";
            condVec.erase(condVec.begin() + joinPos + 1, condVec.begin() + joinPos + 3);
            //cout << "After erase the content is: " << endl;
            //printMyVector(condVec);
            //cout << "adding join condition: " << foundCondition << endl;
            joinConditions.push_back(foundCondition);
            //cout << *vit << endl;
            //joinConditions.push_back(*vit + next(vit,1) + next(vit,2));
          }
        }
      }
    }  
  }
  return joinConditions;
}

vector<string> getJoinCompConditions (vector<string> &condVec, string relationName){
  cout << "inside getJoinCompConditions" << endl;
  vector<string>::iterator vit;
  vector<string>tempVec,tempVec2;
  vector<string>joinConditions;
  cout << "printing condVec" << endl;
  printMyVector(condVec);
  for (vit=condVec.begin();vit<condVec.end();++vit)
  {
    //check to see if we have found a comparison string
    if((*vit).compare("OR") !=0 && (*vit).compare("AND") !=0
      && (*vit).compare("NOT") !=0 && (*vit).compare("true") !=0
      && (*vit).compare("false") !=0) 
    {
      cout << "found something not a boolean or boolean operator" << endl;
      cout << *vit << endl;
      //cout << *vit << endl;
      //push back value into tempVec
      tempVec.push_back(*vit);
      /**
      tempVec = splitString(*vit, ".");
      if (tempVec.size() > 1)
      {
        //we have found an relation name and an attribute name
        //cout << tempVec[0] << endl;
        //see if this is part of a comparison between attributes for two different relations
        if ((*(vit+1)).compare("=") == 0 || 
          (*(vit+1)).compare("<") == 0 ||
          (*(vit+1)).compare(">") == 0)
        {
          //we have a comparison operator; check to see if the next term is another attribute
          //cout << " test " << *(vit+2) << endl;
          tempVec2 = splitString(*(vit+2), ".");
          //cout << tempVec2[0] << endl;
          if (tempVec2.size() > 1 && tempVec[0].compare(tempVec2[0]) != 0)
          {
            //we have found an attribute from a different table.
            string foundCondition = *vit + " " + *(vit+1) + " " + *(vit+2);
            //cout << "Found at: " << vit - condVec.begin() << endl;
            int joinPos = vit - condVec.begin();
            condVec[joinPos] = "true";
            condVec.erase(condVec.begin() + joinPos + 1, condVec.begin() + joinPos + 3);
            //cout << "After erase the content is: " << endl;
            printMyVector(condVec);
            //cout << "adding join condition: " << foundCondition << endl;
            joinConditions.push_back(foundCondition);
            //cout << *vit << endl;
            //joinConditions.push_back(*vit + next(vit,1) + next(vit,2));
          }
          
        }
      }
      */
    }
    else{ //check to see what the tempVec contains
      if (tempVec.size() > 0){
        printMyVector(tempVec);
        cout << endl;
        //we are done with tempVec; clear it
        tempVec.clear();
      }
    }
  }
  
  return joinConditions;
}


//vector<tuple> getTuplesSatisfyingJoinCondition(){}
/**
bool doesTupleSatisfyWhereGivenJoin(Tuple tuple, vector<string> relations, vector<string> condVec, 
                                    SchemaManager schema_manager, MainMemory &mem){
  ostringstream convertIntToStr;
  vector<string>evalCondVec;
  vector<string>tempVec,tempVec2;
  vector<string>multiTableComparisons;
  vector<string>::iterator rit;
  vector<string>::iterator vit;
  Relation* relation_ptr;
  Schema tuple_schema;
  bool whereCond = false;
  //look through each relation and get tuples where most conditions apply
  for (rit=relations.begin();rit>relations.end();++rit){
    //get current relation
    relation_ptr = schema_manager.getRelation(*rit);
    tuple_schema = relation_ptr->getSchema();
    //TODO get tuples from relation here
    //TODO set everything in next for loop into another for loop going through tuples
    for (vit=condVec.begin();vit>condVec.end();++vit){
      //check to see if we have found an attribute name
        if(isalpha((*vit).at(0)) && (*vit).compare("OR") !=0 && (*vit).compare("AND") !=0
          && (*vit).compare("NOT") !=0) {
          //we have found an attribute name; need to see if it contains a relation name
          tempVec = splitString(*vit, ".");
          if (tempVec.size() > 1){
            //we have found an relation name and an attribute name
            //check to see if relation name matches the relation we are currently going through
            if (tempVec[0].compare(relation_ptr->getRelationName())){
              //TODO: see if this is part of a comparison between attributes for two different relations
              if ((*(vit.next())).compare("=") == 0){
                tempVec2 = splitString(*(vit.next().next()), ".");
                if (tempVec2.size() > 1 && tempVec[0].compare(tempVec2) != 0){
                  //we have found an attribute from a different table.
                  //TODO determine logic here
                }
              }
              else{
                //we don't have an equality comparison going on.
                if(tuple_schema.getFieldType(*vit)==INT) {
                  //convertIntToStr << tuple.getField(*vit).integer;
                  //evalCondVec.push_back(convertIntToStr.str());
                  //convertIntToStr.str("");
                  //convertIntToStr.clear();
                }
                else {
                  //evalCondVec.push_back(*(tuple.getField(*vit).str));
                }
              }
             
            }
            else{
              //go on to the next element
              //TODO: make sure this fits our desired logic
              continue;
            }
          }
          else{
            //we found an attribute name with no relation name
            //TODO: make sure the attribute is in the current relation's schema
            /**
            pseudocode: 
            if (attribute not in current relation)
              set aside the same as if we found something like "r.a = s.b"?
            else
              try to push it back into the thing?
          }
          
          /**
          if(tuple_schema.getFieldType(*vit)==INT) {
            convertIntToStr << tuple.getField(*vit).integer;
            evalCondVec.push_back(convertIntToStr.str());
            convertIntToStr.str("");
            convertIntToStr.clear();
          }
          else {
            evalCondVec.push_back(*(tuple.getField(*vit).str));
          }
          
          }
        }
        else {
          evalCondVec.push_back(*vit);
        }
      }
      /**
      whereCond = doesTupleSatisfyWhere(evalCondVec);
      evalCondVec.clear();
      return whereCond;
      
      
      
      //TODO handle cases that use both tables
      
    
  }
  return whereCond;
}
}
*/

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
  for(vit=condVec.begin(); vit != condVec.end(); ++vit) {
    if((*vit).compare("OR") != 0 && (*vit).compare("AND") != 0 && (*vit).compare("NOT") != 0 &&
	    (*vit).compare("[") != 0 && (*vit).compare("]") != 0 && (*vit).compare("true") != 0 &&
      (*vit).compare("false") != 0) {
	  operandVec.push_back(*vit);
	  //cout << endl << endl ;
	  //printVector(operandVec);
	  //cout << endl << endl ;
    }
    // what to do if simply we have true or false
    else if((*vit).compare("true") == 0 || (*vit).compare("false") == 0) {
      rpnVec.push_back(*vit);
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
  /*
  cout << "The reverse polish notation is " << endl;
  for(vit = rpnVec.begin(); vit != rpnVec.end(); ++vit) {
		cout << *vit << " ";
  }
  cout << endl;
  */
  result = evaluateBoolRPN(rpnVec);
  return result;
}
