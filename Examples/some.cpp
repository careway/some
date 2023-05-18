#include <iostream>
#include <some.hpp>
#include <chrono>


int main(){

    some::Init(some::CLEAR_TYPE::Line, "index.txt");
    
    std::thread t1 = std::thread([](){
        for ( int i = 0 ; i < 5; i++)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1200));
            some::printfn<2>("%4d  lazy", i);;
        }
    });
    std::thread t2 = std::thread([](){
        for ( int i = 0 ; i < 5; i++)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            some::printfn<3>("%4d  fast ", i);;
        }
    }); 

    for ( int i = 0 ; i < 5; i++)
    {
        some::printfn<0>("Hola : %4d", i);
        some::printn<1>("Que tal");

        some::some::Spin();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    t1.join();
    t2.join();
    some::some::DeInit();
    return 0;
}
