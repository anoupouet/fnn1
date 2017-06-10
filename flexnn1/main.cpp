//
//  main.cpp
//  flexnn1
//
//  Created by Alexander Lyashevsky on 6/9/17.
//  Copyright © 2017 Alexander Lyashevsky. All rights reserved.
//

#include <iostream>

#define NUM_TYPES   5
enum {INT8, INT16, INT32, FP16, FP32};
extern "C" const char *precisionName[NUM_TYPES] = {"INT8", "INT16", "INT32", "FP16", "FP32"};


extern "C" int mainLoop(int unitPrecision,
             int targetNumClocks);

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    
    int unitPrecision = FP32;
    int targetNumClocks = 1000;
   
    mainLoop(unitPrecision,targetNumClocks);
    
    return 0;
}
