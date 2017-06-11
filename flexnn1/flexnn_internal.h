//
//  flexnn_internal.h
//  flexnn1
//
//  Created by Alexander Lyashevsky on 6/11/17.
//  Copyright © 2017 Alexander Lyashevsky. All rights reserved.
//

#ifndef flexnn_internal_h
#define flexnn_internal_h

#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <locale>         // std::locale, std::toupper
#include <iterator>
#include <algorithm>
#include <array>
#include <vector>
using namespace std;





#define NUM_TYPES   5
enum {INT8, INT16, INT32, FP16, FP32};
static
const char *precisionName[NUM_TYPES] = {"INT8", "INT16", "INT32", "FP16", "FP32"};


typedef std::vector<std::string> string_array;

extern "C" int mainLoop(int unitPrecision,
                        int targetNumClocks);


struct flexModelSpace {
    
    string_array precisions;
    
    flexModelSpace()
    {
        precisions = string_array(precisionName, precisionName + NUM_TYPES);
    }
};

struct flexParam {
    
    int unitPrecision;
    int targetNumClocks;
    std::string model_file;
    
    flexParam()
    {
        unitPrecision = FP32;
        targetNumClocks = 1000;
        model_file = "flexnn.ad";
    }
};


#endif /* flexnn_internal_h */
