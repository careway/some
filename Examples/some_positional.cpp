#include <iostream>
#include <some.hpp>
#include <chrono>


int main(){

    some::Init(some::CLEAR_TYPE::Line, "positional.txt");
    
    std::thread t1 = std::thread([](){
        for ( int i = 0 ; i < 5; i++)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1200));
            some::printf("%4d  lazy", i);;
        }
    });
    std::thread t2 = std::thread([](){
        for ( int i = 0 ; i < 5; i++)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            some::printf("%4d  fast ", i);;
        }
    }); 

    for ( int i = 0 ; i < 5; i++)
    {
        some::printf("Hola : %4d", i);
        some::print("Que tal");

        some::some::Spin();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    t1.join();
    t2.join();
    some::some::DeInit();
    return 0;
}
