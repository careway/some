#include <iostream>
#include "some/some.hpp"
#include <chrono>


void Hola()
{
    if(std::rand()%2)
        some::console::Print<3>("No me aburro");
    else
        some::console::Print<3>("Si me aburro");
}

int main(){

    some::console::Init(some::CLEAR_TYPE::Line);
    
    for ( int i = 0 ; i < 5; i++)
    {
        
        Hola();
        some::printf("Hola : %d", std::rand()%10);
        some::print("Que tal");

        std::this_thread::sleep_for(std::chrono::seconds(1));


        some::console::Spin();
    }
    return 0;
}
