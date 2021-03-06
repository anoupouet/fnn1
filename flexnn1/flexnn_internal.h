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
#include <fstream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <locale>         // std::locale, std::toupper
#include <iterator>
#include <algorithm>
#include <array>
#include <vector>
using namespace std;


class flexParam;


#define NUM_TYPES   5
enum {INT8, INT16, INT32, FP16, FP32};
static
const char *precisionName[NUM_TYPES] = {"INT8", "INT16", "INT32", "FP16", "FP32"};


typedef std::vector<std::string> string_array;


struct flexPowerArea{
    float powerAdd;
    float powerMul;
    int areaAdd;
    int areaMul;
    
    int numElementsPerWord32;
    flexPowerArea()
    {
        powerAdd = 0;
        powerMul =0;
        areaAdd = 0;
        areaMul = 0;
        
        numElementsPerWord32 = 0;
        
    }
    
} ;

typedef std::vector<flexPowerArea> power_area_array;


enum {OP_CONV, OP_MM, OP_PULL};

struct flexLayer{
    
    int id;
    int op;
    int batch;
    int n_inputs;
    int n_outputs;
    int in_height;
    int in_width;
    int out_height;
    int out_width;
    int conv_height;
    int conv_width;
    int precision;
    
    int convMethod;
    size_t numMuls;
    size_t numAdds;
    size_t numSRAMread;
    size_t numSRAMwrite;
    size_t numDRAMread;
    size_t numDRAMwrite;
public:
    flexLayer()
    {
        
        id = 0;
        op = 0;
        batch = 0;
        n_inputs = 0;
        n_outputs = 0;
        in_width = 0;
        in_height = 0;
        out_height = 0;
        out_width = 0;
        conv_height = 0;
        conv_width = 0;
        precision = 0;
        
        convMethod = 0;
        numMuls = 0;
        numAdds = 0;
        numSRAMread = 0;
        numSRAMwrite = 0;
        numDRAMread = 0;
        numDRAMwrite = 0;
    }

} ;

typedef std::vector<flexLayer> layer_array;

#define NUM_LAYERS  10

class flexNet {
 
public:
    
    layer_array net;
    
public:
    flexNet()
    {
        
    }
    
public:
 
};




class flexModelSpace {
public:
    string_array precisions;
    
public:
    power_area_array powerArea;
 
    double powerReadSRAM32;
    double powerReadDRAM32;
    
    double powerWriteSRAM32;
    double powerWriteDRAM32;
    
    size_t areaSRAM;
    int Init(int argc, const char * argv[], flexParam & params);
    
protected:
    int InitPowerArrea();
public:
    flexModelSpace()
    {
        precisions = string_array(precisionName, precisionName + NUM_TYPES);
 
        
        powerReadSRAM32 = 5;
        powerReadDRAM32 = 640;
        
        powerWriteSRAM32 = 0;
        powerWriteDRAM32 = 0;
        
        areaSRAM = 0;
    }
};

class flexParam {
public:
    int unitPrecision;
    size_t targetNumClocks;
    std::string model_file;
    
    flexNet net_model;
    
public:
    int Init(const flexModelSpace & model_space);

public:
    flexParam()
    {
        unitPrecision = FP32;
        targetNumClocks = 1000;
        model_file = "flexnn.ad";
        
    }
protected:
    int parseNetModelFile(void);
};

class flexModel{
    
    
public:

    size_t numExecUnits;
    size_t totalClks;
    double power;
    size_t area;
    
public:
    flexModel()
    {
        numExecUnits = 0;
        totalClks = 0;
        power = 0;
        area = 0;
  
        maxLayerOps = 0;
        maxLayer = -1;
        
    }
public:
    
    int Init(void);
    int searchHWConfig(const flexModelSpace & flex_space, const flexParam & flex_params);
    int computePower(const flexModelSpace & flex_space, const flexParam & flex_params);
    int computeArea(const flexModelSpace & flex_space, const flexParam & flex_params);
    
    int calcPower(void);
    
    int calcArea(void);
    
    
protected:
    size_t maxLayerOps;
    int maxLayer;
    
    int calcMaxNumOpPerLayer(const flexParam & flex_params);
    size_t clocksPerLayer(size_t numOps, size_t numExecUnits);
    size_t clocksPerNetwork(const flexParam & flex_params, size_t numExecUnits);
    size_t selectNumExecUnits(const flexParam & flex_params, size_t oldNumUnits );

};

int flexNNAnaliticalModel(int argc, const char * argv[]);


#endif /* flexnn_internal_h */
