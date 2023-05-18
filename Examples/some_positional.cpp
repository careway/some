#include <iostream>
#include <some.hpp>
#include <chrono>


int main(){

    some::Init(some::CLEAR_TYPE::Line, "positional.txt");
    
    std::thread t1 = std::thread([](){
        int i = 0;
        while(true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(600));
            some::printf("%4d  lazy", i++);
        }
    });
    std::thread t2 = std::thread([](){
        int i = 0;
        while(true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            some::printf("%4d  fast ", i++);
        }
    }); 
    std::thread t3 = std::thread([](){
        int i = 0;
        while(true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            some::printf("Hola : %4d", i++);
        }
    }); 
    
    auto start = std::chrono::steady_clock::now();
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        some::some::Spin();
        if(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now()-start).count()>15)
            break;
    }

    t3.join();
    t1.join();
    t2.join();
    some::some::DeInit();
    return 0;
}
