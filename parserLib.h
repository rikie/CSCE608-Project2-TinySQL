#include <iostream>  
#include <sstream>  
#include <list>  
#include <stack>  
#include <map>  
#include <string>  
#include <vector>  
#include <iterator>  
#include <stdlib.h>  
        
const int LEFT_ASSOC  = 0;      
const int RIGHT_ASSOC = 1;   
        
// Map the different operators: +, -, *, / etc    
typedef std::map< std::string, std::pair< int,int > > OpMap;     
typedef std::vector<std::string>::const_iterator cv_iter;  
typedef std::string::iterator s_iter;  
 
// This function is also used 
// Convert infix expression format into reverse Polish notation          
bool infixToRPN( const std::vector<std::string>& inputTokens,     
                 const int& size,     
                 std::vector<std::string>& strArray ); 
// This function is also used  
double RPNtoDouble( std::vector<std::string> tokens );
// This function is used
std::vector<std::string> getExpressionTokens( const std::string& expression );
// Print iterators in a generic way
// This function is also used
template<typename T, typename InputIterator>  
void Print( const std::string& message,  
             const InputIterator& itbegin,     
             const InputIterator& itend,     
             const std::string& delimiter);  
