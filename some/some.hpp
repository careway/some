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


class some
{

    public:

    enum class CLEAR_TYPE
    {
        Line,
        Console
    };

    typedef std::map<int, std::string> sentenceMap;
    typedef std::map<int, size_t> idxSizeMap;

    //Dump to folder
    static std::ofstream _fOut;
    static std::string _header;

    //General
    static CLEAR_TYPE  _type;
    static size_t      _rows;
    
    //Storing async tasks
    static std::mutex  _mtxTasks;
    static std::vector<std::future<void>> _tasks;

    //

    static std::mutex  _mtxMap;
    static sentenceMap _outputStorage;

    static std::mutex  _mtxSize;
    static idxSizeMap  _idxSize;
    
    static std::mutex  _mtxId;
    static std::map<std::pair<int,std::string>,int> _mapIdLine;

    public: 

    #define print(str) Print(__LINE__,__FILE__,str)
    #define printf(format, ...) Print(__LINE__,__FILE__,format,__VA_ARGS__)

    static void Init(CLEAR_TYPE type)
    {
        _type = type;
    }

    static void Init(CLEAR_TYPE type, std::string file)
    {
        // std::cout << "\033[2J";
        _type = type;
        _fOut.open(file,std::ios_base::app);
    }
    
    template<int N>
    static void printn(const std::string str)
    {
        {
            std::unique_lock<std::mutex>lck(_mtxMap);
            _outputStorage[N] = str;
        }
        CountRows();
    }

    template<int N,typename ... Args>
    static void printfn(const std::string format, const Args... args)
    {
        
        std::unique_lock<std::mutex> lck (_mtxTasks);
        _tasks.emplace_back(
            std::async(std::launch::async, [=](){
                std::string ss =  tfm::format(format.c_str(), args...);
                {
                    std::unique_lock<std::mutex>lck(_mtxMap);
                    _outputStorage[N]=ss;
                }
                CountRows();
            })
        );
    }

    static void Spin(bool erase_prev = false)
    {

        std::stringstream out;
        {
            std::unique_lock<std::mutex>lck(_mtxTasks);
            for(auto t = _tasks.cbegin(), cend = _tasks.cend(); t != cend; t++)
                t->wait();
            _tasks.clear();
        
        }
        for( sentenceMap::const_iterator cit=_outputStorage.cbegin(), cend = _outputStorage.cend(); cit != cend; cit++)
        {
            out << cit->second << "\n";   
        }

        if(_fOut.is_open()){
            _fOut << _header;
            _fOut << out.str();
        }
        std::cout << ClearOutput();
        std::cout << out.str();

    }

    static void DeInit()
    {
        if (_fOut)
            _fOut.close();
    }


    template<typename ... Args>
    static void Print(const int line, const std::string file, const std::string format, const Args... args)
    {
        
        std::unique_lock<std::mutex> lck (_mtxTasks);
        _tasks.emplace_back(
            std::async(std::launch::async, [=](){
                std::string ret = tfm::format(format.c_str(), args...);
                int N = getN(line,file);
                {
                    std::unique_lock<std::mutex>lck(_mtxMap);
                    _outputStorage[N]= ret;
                }
                CountRows();
            })
        );


    }

    static void Print(const int line, const std::string file, const std::string str)
    {
        int N = getN(line, file);
        {
            std::unique_lock<std::mutex>lck(_mtxMap);
            _outputStorage[N] = str;
        }
        CountRows();
    }

    private:
    static std::string ClearOutput()
    {
        static int ml = 0;
        std::string ret = "";
        switch(_type)
        {
            case CLEAR_TYPE::Console:
                ret = "\033[2J";
                break;

            case CLEAR_TYPE::Line:
                int n = 0;
                ret = "";
                std::string endl = "";
               
                for(idxSizeMap::const_iterator cit = _idxSize.cbegin(), cend = _idxSize.cend(); cit != cend; cit++ )
                    for(size_t i = 0 ; i < cit->second; i++)
                    {
                        n++;
                        ret += PREV_LINE;
                    }
                
                for(int i = ml; i< n; i++)
                    endl +="\n";
                ret = endl + ret;
                ml = std::max(ml,n);
                break;

        }
        return ret;
    }

    static inline size_t getN(const int& line, const std::string& file)
    {
        
        std::unique_lock<std::mutex> lck (_mtxId);
        int N = 0;
        if(_mapIdLine.find(std::make_pair(line,file)) == _mapIdLine.cend())
        {
            
            std::unique_lock<std::mutex> lck (_mtxMap);
            if(_outputStorage.size()>0)
                N = std::prev(_outputStorage.cend())->first+1;
            else 
                N = 0;
            _mapIdLine[std::make_pair(line,file)] = N;
        }
        else
        {
            N = _mapIdLine[std::make_pair(line,file)];
        }
        return N;
    }

    
    static void CountRows()
    {
        sentenceMap copy;
        {
            std::unique_lock<std::mutex> lck (_mtxMap);
            copy = _outputStorage;
        }
        for( sentenceMap::const_iterator cit=copy.cbegin(), cend = copy.cend(); cit != cend; cit++)
        {
            size_t idx = -1;
            size_t rows = 0;
            do
            {
                idx =  cit->second.find('\n', idx+1);
                rows++;
            } while( std::string::npos != idx);
            
            std::unique_lock<std::mutex> lck (_mtxId);
            _idxSize[cit->first]=std::max(_idxSize[cit->first],rows);
        }
    }
    
};

std::map<int, std::string> some::_outputStorage = std::map<int,std::string>();
std::mutex some::_mtxMap, some::_mtxTasks, some::_mtxId, some::_mtxSize;
some::idxSizeMap some::_idxSize = some::idxSizeMap();
std::map<std::pair<int,std::string>,int> some::_mapIdLine = std::map<std::pair<int,std::string>,int> ();
std::vector<std::future<void>> some::_tasks = std::vector<std::future<void>>();
std::ofstream some::_fOut = std::ofstream();
some::CLEAR_TYPE some::_type = some::CLEAR_TYPE::Console;
std::string some::_header= "----------\n";


