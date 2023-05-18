#include <tinyformat/tinyformat.h>
#include <sstream>
#include <iostream>
#include <string>
#include <mutex>
#include <fstream>
#include <future>
#include <functional>
#include <vector>
#include <map>
#include <filesystem>
#include <vector>
#include <type_traits>
#if defined(_MSC_VER)
#include <io>
#define PREV_LINE "\u001B[A"
#elif defined(__linux__)
#include <stdio.h>
#define PREV_LINE "\033[F"
#endif
namespace some
{

    #define print(str) console::Print(__LINE__,__FILE__,str)
    #define printf(format, ...) console::Print(__LINE__,__FILE__,format,__VA_ARGS__)

enum class CLEAR_TYPE
{
    Line,
    Console
};

class console
{
    public:

    static void Init(CLEAR_TYPE type)
    {
        std::cout << "\033[2J";
        _type = type;
    }

    static void Init(CLEAR_TYPE type, std::string file)
    {
        _type = type;
        _fOut = new std::ofstream(file,std::ios_base::out);
    }
    
    private:

    typedef std::map<int, std::string> sentenceMap;
    typedef std::map<int, int> idxSizeMap;

    
    static std::stringstream _ss;
    static CLEAR_TYPE _type;
    static std::ofstream* _fOut;
    static std::vector<std::future<void>> _tasks;
    static std::mutex  _mtxMap;
    static std::mutex  _mtxVec;
    static sentenceMap _outputStorage;
    static idxSizeMap  _idxSize;
    static std::map<std::pair<int,std::string>,int> _mapIdLine;
    static int rows;

    static void CountRows()
    {
        std::unique_lock<std::mutex> lck (_mtxMap);
        for( sentenceMap::const_iterator cit=_outputStorage.cbegin(), cend = _outputStorage.cend(); cit != cend; cit++)
        {
            size_t idx = 0;
            while( std::string::npos != idx)
            {
                idx =  cit->second.find('\n', idx);
                rows++;
            }
            _idxSize[cit->first]=std::max(_idxSize[cit->first],rows);
            
        }
    }


    public: 
    console()=default;

    template<int N>
    static void Print(const std::string str)
    {
        std::unique_lock<std::mutex>lck(_mtxMap);
        _outputStorage[N] = str;
    }

    template<int N,typename ... Args>
    static void Print(const std::string format, const Args... args)
    {
        
        std::unique_lock<std::mutex> lck (_mtxVec);
        _tasks.emplace_back(
            std::async(std::launch::async, [=](){
                std::string ss =  tfm::format(format.c_str(), args...);
                std::unique_lock<std::mutex>lck(_mtxMap);
                _outputStorage[N]=ss;

            })
        );
    }
    template<typename ... Args>
    static void Print(const int line, const std::string file, const std::string format, const Args... args)
    {
        
        std::unique_lock<std::mutex> lck (_mtxVec);
        _tasks.emplace_back(
            std::async(std::launch::async, [=](){
                std::string ret = tfm::format(format.c_str(), args...);
                std::unique_lock<std::mutex>lck(_mtxMap);
                int N = 0;
                if(_mapIdLine.find(std::make_pair(line,file)) == _mapIdLine.cend())
                {
                    N = std::prev(_outputStorage.cend())->first+1;
                    _mapIdLine[std::make_pair(line,file)] = N;
                }
                else
                {
                    N = _mapIdLine[std::make_pair(line,file)];
                }
                _outputStorage[N]= ret;

            })
        );

    }

    static void Print(const int line, const std::string file, const std::string str)
    {
        std::unique_lock<std::mutex>lck(_mtxMap);
        int N = 0;
        if(_mapIdLine.find(std::make_pair(line,file)) == _mapIdLine.cend())
        {
            N = std::prev(_outputStorage.cend())->first+1;
            _mapIdLine[std::make_pair(line,file)] = N;
        }
        else
        {
            N = _mapIdLine[std::make_pair(line,file)];
        }
        _outputStorage[N] = str;
    }

    static std::string ClearConsole()
    {
        std::string ret = "";
        switch(_type)
        {
            case CLEAR_TYPE::Console:
                ret = "\033[2J";
                break;

            case CLEAR_TYPE::Line:
                int n = 0;
                ret = "";
                for( sentenceMap::const_iterator cit=_outputStorage.cbegin(), cend = _outputStorage.cend(); cit != cend; cit++)
                {
                    size_t pose = 0;
                    do{
                        ret += PREV_LINE;
                        pose = cit->second.find('\n',pose); 
                    }
                    while(pose != std::string::npos);

                }
                ret += PREV_LINE;
                break;
        }
        return ret;
    }
    
    static void Spin(bool erase_prev = false)
    {

        std::stringstream out;
        // print to console  TODO: More than one line, map indicate positions
        {
            std::unique_lock<std::mutex>lck(_mtxVec);
            for(auto t = _tasks.cbegin(), cend = _tasks.cend(); t != cend; t++)
                t->wait();
            _tasks.clear();
        
        }
        for( sentenceMap::const_iterator cit=_outputStorage.cbegin(), cend = _outputStorage.cend(); cit != cend; cit++)
        {
            out << cit->second << "\n";   
        }
        if(erase_prev)
            _outputStorage.clear();

        std::cout << ClearConsole();
        std::cout << out.str();
        (*_fOut) << out.str();
        // log output if pointed

    }

};

std::map<int, std::string> console::_outputStorage = std::map<int,std::string>();
std::mutex console::_mtxMap, console::_mtxVec;
std::map<int, int>  console::_idxSize = std::map<int,int>();
std::map<std::pair<int,std::string>,int> console::_mapIdLine = std::map<std::pair<int,std::string>,int> ();
std::vector<std::future<void>> console::_tasks = std::vector<std::future<void>>();
std::ofstream* console::_fOut = nullptr;
CLEAR_TYPE console::_type = CLEAR_TYPE::Console;
int console::rows = 0;


}
