#include <iostream>
#include <some.hpp>
#include <chrono>



int main()
{
    some::Init(some::CLEAR_TYPE::Console);
    some::print("-----------------------------------------");
    some::pbar h  = some::pbar(0,1000,1,"Steps ");
    for(int i = 0; i < 1000; i += 1)
    {
        h.update();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        some::printf("Value: %5d", i);
    }


}