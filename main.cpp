#include <fstream>
#include <string>
#include <map>
#include <cstring>
#include <iostream>
#include <functional>
#include <filesystem>
#include <cassert>

class Choice
{
public:
    Choice(const std::string& question)
        : m_question(question)
    {}

    ~Choice() 
    {
        assert(m_presented);
    }
    
    template<typename _T>
    void add_choice(char c, _T fnc)
    {
        m_choices.emplace_back(tolower(c), fnc);
    }
    
    void present()
    {
        m_presented = true;
        
        char choice;
        bool eof;
        std::vector<std::pair<char, std::function<void()>>>::iterator pos;
        do
        {
            std::cout << "? " << m_question;
            std::cout << " [";
            int counter = 0;
            for (auto& pair : m_choices)
            {
                std::cout << pair.first;
                if (++counter < m_choices.size())
                    std::cout << "/";
            }
            std::cout << "]: ";             
            std::cin.get(choice);
            std::cin.ignore();
            
            choice = tolower(choice);
            
            pos = std::find_if(m_choices.begin(), m_choices.end(), [&](auto& p) {
                return p.first == choice;
            });
            
        } while (pos == m_choices.end());
        
        pos->second();
    }
    
protected:
    std::string m_question;
    bool m_presented = false;
    std::vector<std::pair<char, std::function<void()>>> m_choices;
};

std::string line_prompt(const std::string& prompt)
{
    std::string s;
    std::cout << prompt << std::endl << "> ";
    std::getline(std::cin, s);
    return s;
}

std::string ask_for_line(const std::string& what, bool make_sure = false)
{
    std::string s = line_prompt("Please enter " + what);
    
    if (make_sure)
    {
        Choice confirm("Continue with this input (y) or retry (n)?");
        confirm.add_choice('y', [&]() { });
        confirm.add_choice('n', [&]() {
            s = ask_for_line(what, make_sure);
        });
        confirm.present();
    }
    
    return s;
}

static const std::string maincppcontents = 
R"(
#include <iostream>

int main(int, char**)
{
}
)";

int main(int argc, char* argv[])
{
    std::string directory = ask_for_line("directory to create project folder in");
    std::string project_name = ask_for_line("project name");
    
    auto project_path = std::filesystem::path(directory) / project_name;
    
    std::cout << "The new project will be created under " << project_path << "." << std::endl;
    
    std::filesystem::create_directory(project_path);
    
    Choice cmakelists("Create CMakeLists.txt?");
    cmakelists.add_choice('y', [&]() {
        std::ofstream cml(project_path / "CMakeLists.txt");
        cml << "project(" << project_name << ")" << std::endl
            << "set(CMAKE_CXX_STANDARD 17)" << std::endl
            << "add_executable(" << project_name << " main.cpp)" << std::endl;
        cml.close();
    });
    cmakelists.add_choice('n', []() {} );
    cmakelists.present();
    
    std::ofstream maincpp(project_path / "main.cpp");
    maincpp << maincppcontents << std::endl;
    maincpp.close();
    
    Choice binmake("Create make.sh and bin directory?");
    binmake.add_choice('y', [&]() {
        std::filesystem::create_directory(project_path / "bin");
        
        std::ofstream make(project_path / "make.sh");
        make << "#!/bin/bash" << std::endl
             << "cd bin" << std::endl
             << "rm -v " << project_name << std::endl
             << "cmake -DCMAKE_BUILD_TYPE=Debug ../" << std::endl
             << "make -j 9" << std::endl
             << "cd .." << std::endl;
        make.close();
        
        std::filesystem::permissions(project_path / "make.sh", 
                                     std::filesystem::perms::owner_exec 
                                   | std::filesystem::perms::others_exec, 
                                     std::filesystem::perm_options::add);
    });
    binmake.add_choice('n', []() {});
    binmake.present();
    
    std::cout << "Project created." << std::endl;
}
