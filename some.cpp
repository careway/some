#include <iostream>
#include "some/some.hpp"
#include <chrono>


void Hola()
{
    if(std::rand()%2 != 0)
        some::some::Print<3>("No me \n aburro");
    else
        some::some::Print<3>("Si me aburro");
}

int main(){

    some::some::Init(some::CLEAR_TYPE::Line, "hola.txt");
    
    for ( int i = 0 ; i < 5; i++)
    {
        
        Hola();
        some::some::printf("Hola : %d", std::rand()%10);
        some::some::print("Que tal");

        std::this_thread::sleep_for(std::chrono::seconds(1));
        some::some::Spin();
    }
    some::some::DeInit();
    return 0;
}
