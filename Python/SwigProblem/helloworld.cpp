
#include <stdio.h>
#include <iostream>
#include "helloworld.h"
#include "dataformatters.h"


void HelloWorld() {

    double dummy = 5.7;

    std::cout << "Hello, world" << std::endl;

// uncomment the following line to corrupt the SWIG Python extension
//    std::cout << ("Hello, world," + QuantLib::DoubleFormatter::toString(dummy) + " is a number") << std::endl;

}

