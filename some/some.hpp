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

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NOMINMAX
#define PREV_LINE "\u001B[A"
#include <Windows.h>
#elif defined(__linux__)
#include <sys/ioctl.h>
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

    #define print(str) Print(__LINE__,__FILE__,str)
    #define printf(format, ...) Print(__LINE__,__FILE__,format,__VA_ARGS__)
    #define MakeAllPermanent() MakePermanent(-1)
    static void Init(CLEAR_TYPE type)
    {
        _type = type;
        if(_type == CLEAR_TYPE::Console)
            std::cout << "\033[2J";
    }

    static void Init(CLEAR_TYPE type, std::string file)
    {
        _type = type;
        if(_type == CLEAR_TYPE::Console)
            std::cout << "\033[2J";
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

    static void Spin(bool erase_prev = false)
    {

        std::stringstream out;
        {
            std::unique_lock<std::mutex>lck(_mtxTasks);
            for(auto t = _tasks.cbegin(), cend = _tasks.cend(); t != cend; t++)
                t->wait();
            _tasks.clear();
        
        }
        sentenceMap copy;
        {
            std::unique_lock<std::mutex> lck (_mtxMap);
            copy = _outputStorage;
        }
        for( sentenceMap::const_iterator cit=copy.cbegin(), cend = copy.cend(); cit != cend; cit++)
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

    static void Clear()
    {
        {
            std::unique_lock<std::mutex>lck(_mtxTasks);
            for(auto t = _tasks.cbegin(), cend = _tasks.cend(); t != cend; t++)
                t->wait();
            _tasks.clear();
            
            std::unique_lock<std::mutex> lck1 (_mtxMap);
            _outputStorage.clear();
        }
        
        CountRows();
    }

    static void DeInit()
    {
        if (_fOut)
            _fOut.close();
    }


 
    private:


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


    static std::string ClearOutput()
    {
        static int ml = 0;
        std::string ret = "";

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
        ml = n;

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
            _outputStorage[N]="";
            _mapIdLine[std::make_pair(line,file)] = N;
        }
        else
        {
            N = _mapIdLine[std::make_pair(line,file)];
        }
        return N;
    }

    static inline size_t getN()
    {
        std::unique_lock<std::mutex> lck (_mtxId);
        int N = 0;
            
        std::unique_lock<std::mutex> lck1 (_mtxMap);
        if(_outputStorage.size()>0)
            N = std::prev(_outputStorage.cend())->first+1;
        else 
            N = 0;
        _outputStorage[N]="";
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
            _idxSize[cit->first]=rows;
        }
    }

    static void _print(const int N, const std::string str)
    {
        {
            std::unique_lock<std::mutex>lck(_mtxMap);
            _outputStorage[N] = str;
        }
        CountRows();
    }

public:
    static void MakePermanent(int N)
    {

        {
            std::unique_lock<std::mutex> lck (_mtxMap);
            
            std::cout << ClearOutput() ;
            if(N == -1){
                for(auto c : _outputStorage)
                    std::cout << c.second << "\n\n"  ;
                
                _outputStorage.clear();
                _idxSize.clear();
            }
            else
            {
                std::cout << _outputStorage[N] <<  "\n\n" ;
                _outputStorage.erase(N);
                _idxSize.erase(N);
            }
        }
        Spin();
    }


class pbar
{

    public:
    pbar():_start(0),_end(100),_jumps(1),_N(getN()),_hi(0),_current(0)
    {
        get_terminal_size(w,h);
        _header = " [";
        w -= _header.size() + 4;
        _tail_max = std::to_string(_end).size()*2 + 6;
        if(w > _tail_max)
            w -= _tail_max;
        else
            _tail_max = -1;
            
        _n_hash = float(_end-_start)/(float)w;
        
        for(int i=0;i<_n_hash;i++)
        {
            _white_spc+=" ";
        }

    }

    pbar(int start, int end, int jumps = 1, std::string header=""):
    _start(start), _end(end), _jumps(jumps), _header(header), _N(getN()), _current(start),_hi(start)
    {
        get_terminal_size(w,h);
        _header += " [";
        w -= _header.size() + 4 ;
        w -= _tail_max = std::to_string(_end).size()*2 + 6;
        if(w + 10  > _tail_max)
            w -= _tail_max;
        else
            _tail_max = -1;

        _n_hash = float(_end-_start)/(float)w;

        for(int i=0;i<w;i++)
        {
            _white_spc+=" ";
        }
    }

    ~pbar()
    {
        some::MakePermanent(_N);
    }

    void update(int step = -1)
    {
        _current += (step==-1)?_jumps:step;
        static std::string end_str = "/"+std::to_string(_end);
        std::string print = _header, tail = std::to_string(_current) + " / " + std::to_string(_end);
        int _w = w; 

        if(_w < 5)
        {
            print += getWaiting() + " ]";
        }
        else
        {

            int c = (_current-_start)/(float)(_n_hash);
            for(; _hi<c ; _hi++)
            {
                _hash+='#';
                
            }
            print += _hash + getWaiting();
            print += _white_spc.substr(_hi);

            print += "] ";
            if(_tail_max != -1)
                print += tail ;
        }

        some::_print(_N,print);
        some::Spin();
        
    }

    private:
    
    int _start;
    int _end;
    int _jumps;
    int _current;
    int _hi;
    int _n_hash;
    int _tail_max;
    std::string _header, _hash, _white_spc;
    int _N;
    int w, h;
        
    static void get_terminal_size(int& width, int& height) {
    #if defined(_WIN32)
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        width = (int)(csbi.srWindow.Right-csbi.srWindow.Left+1);
        height = (int)(csbi.srWindow.Bottom-csbi.srWindow.Top+1);
    #elif defined(__linux__)
        struct winsize w;
        ioctl(fileno(stdout), TIOCGWINSZ, &w);
        width = (int)(w.ws_col);
        height = (int)(w.ws_row);
    #endif // Windows/Linux
    }

    char getWaiting()
    {
        const char c[4]={'/','-', '\\', '|'};
        static char i = 0;
        char ret = c[i++];
        i%=4;
        return ret;

    }

};


};



std::map<int, std::string> some::_outputStorage = std::map<int,std::string>();
std::mutex some::_mtxMap, some::_mtxTasks, some::_mtxId, some::_mtxSize;
some::idxSizeMap some::_idxSize = some::idxSizeMap();
std::map<std::pair<int,std::string>,int> some::_mapIdLine = std::map<std::pair<int,std::string>,int> ();
std::vector<std::future<void>> some::_tasks = std::vector<std::future<void>>();
std::ofstream some::_fOut = std::ofstream();
some::CLEAR_TYPE some::_type = some::CLEAR_TYPE::Line;
std::string some::_header= "----------\n";
