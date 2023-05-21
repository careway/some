# Some 

This project purpose is making simpler to output real-time data, which sometimes can be really complex to show or debug with console.

The class is an static class which needs to be initialized to point if you want a clear console or to start on the current line declaring `some::CLEAR_TYPE::[Console|Line]`. In addition you can indicate an output file to get a copy of the info:

```cpp
some::Init(some::CLEAR_TYPE)
some::Init(some::CLEAR_TYPE, "file_name");
```

Once you have defined the behaviour of some. You can start using it by using the print functions: 
-  the priority for the cmd output (N)

    ```cpp
    some::printfn<N>("This is an example of a Random number %4d", std::rand()%10);
    some::printn<N>("This is an example of a Random number 5");
    ```
-  or and it will take the line and the file of where the function was call.

    ```cpp
    some::printf("This is an example of a Random number %4d", std::rand()%10);
    some::print("This is an example of a Random number 5");
    ```

## Take into account
1. Any of the functions are thread safe, so they can be called from any thread.
2. Mixing both types of functions can create undesirable behaviours.


# Example

There are two different examples showing up how both methods can be used, they are not mixed but feel free to mix them as you want:

At this example you can observe how each function is working in different threads but the output keeps consistency, making easy and accessible to print lines with static content that only modifies determined values. 


![](docs/example_output.gif)



In addition we obtain if pointed an output file with the historic of the values with the frecuency of the spin thread:

```
----------
Hola :  875
   1  fast 
   0  lazy
----------
Hola :  900
   1  fast                  ############################################
   0  lazy                  ###                                      ###
----------                  ###                                      ###
Hola :  926                 ###         Usefull to look at the       ###
   1  fast                  ###             output history           ###
   0  lazy                  ###                                      ###
----------                  ############################################
Hola :  951
   1  fast 
   0  lazy
----------
Hola :  976
   1  fast 
   0  lazy
----------
```

In this way you can keep a clean output while storing the valuable info.