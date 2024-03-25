
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

/* testing for return a .ts file list from index.m3u8
 * Target :  to  clean the .ts file exclude the index.m3u8 ts list files
*/
bool get_ts_list_file_from_index_m3u8(const std::string& path, std::vector<std::string>& vectorfiles)
{
    if (fs::exists(path))
    {
        std::ifstream file(path);

        std::string line;

        if (file.is_open()) {

            while (std::getline(file, line)) {

                if (!line.empty()) {
                    // Remove trailing spaces
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
    }
    else {
        //no index.m3u8
        std::cout << "no index.m3u8 file in app root" << std::endl;
    }

    // no any ts list or no index.m3u8 files
    return false;
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
    /* testing for return a .ts file list from index.m3u8
     * Target :  to  clean the .ts file exclude the index.m3u8 ts list files
    */
    fs::path path = fs::current_path().append("index.m3u8");
    std::vector<std::string> vec_ts_files_index;
    bool ret = get_ts_list_file_from_index_m3u8(path.string(), vec_ts_files_index);
    std::cout << "get_ts_list_file_from_index_m3u8 : "<< ret << " count = "<< vec_ts_files_index.size() << std::endl;
    // delete ts file not in vec_ts_files_index


    

    return 0;
     
}



