
#include <HelloWorld/HelloWorld.h>
#include <MathLibrary/MathLibrary.h>

#include <iostream>  
#include <filesystem> 


//$(ProjectDir)include\ 其他包含目錄 加入這句
#include "File.h"

void TestHelloWorld()
{
	HelloWorld();
}


void TestMathLibrary()
{
	std::cout << "1 add 5 = " << Add(1, 5) << "\n";
	std::cout << "10 Div 2 = " << Div(10, 2) << "\n";
} 

 
 

 
int main()
{
	TestHelloWorld();
	TestMathLibrary();
	std::cout << "Hello World\n";
	  
	std::cout << "Hello test  not use" << std::endl;
	 
	std::cout << "File::MorkPath"<< File::GetWorkPath() << std::endl;
    
	std::cout << "Get CPU ID : "<< File:: << std::endl;

	return 0;
}