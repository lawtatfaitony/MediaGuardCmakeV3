
#include <HelloWorld/HelloWorld.h>
#include <MathLibrary/MathLibrary.h>

#include <iostream>  
#include <filesystem>  
#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>

//$(ProjectDir)include\ 其他包含目錄 加入這句

namespace fs = std::filesystem;

#include "Time.h" 
#include "ManagerController.h"

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
	
	//=================================

    std::ifstream file("index.m3u8");
    std::vector<std::string> vectorfiles;
    std::string line;

    if (file.is_open()) {
        while (std::getline(file, line)) {
           
            if (!line.empty()) {
                // 去除尾部空格
                while (std::isspace(line.back())) {
                    line.pop_back();
                }

                if (line != "index.m3u8" && line[0] != '#' && line.substr(line.size() - 4) != ".tmp") {

                    fs::path path(line); 
                    if (path.extension() == ".ts") {
                        vectorfiles.push_back(line);
                        //TEST LOG
                        std::cout << "Add a file vectorfiles " << line << std::endl;
                    }
                    else {
                        vectorfiles.push_back(line);
                        //TEST LOG
                        std::cout << "no .ts file contained,discard to add to ts file list" << line << std::endl;
                    }
                   
                }
            }
        }
        file.close();
    }
    else {
        std::cerr << "Unable to open file" << std::endl;
        return 1;
    }

    // 处理lines中的每一个索引，例如下载或处理...

    return 0;
     
}



