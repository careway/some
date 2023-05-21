#include <iostream>
#include <some.hpp>
#include <chrono>



int main()
{
    some::Init(some::CLEAR_TYPE::Console);
    some::print("-----------------------------------------");
    some::pbar h  = some::pbar(0,10'0000,1,"Steps ");
    for(int i = 0; i < 10'0000; i ++)
    {
        h.update(100);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        some::printf("Value: %5d", i);
    
    }


}