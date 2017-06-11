#include "flexnn_internal.h"


typedef struct{

    int id;
    int op;
    int width;
    int height;
    int convSize;
    int precision;
    
    int convMethod;
    int numMuls;
    int numAdds;
    int numSRAMread;
    int numSRAMwrite;
    int numDRAMread;
    int numDRAMwrite;
    int power;

} Layer;



typedef struct {
    float powerAdd;
    float powerMul;
    int areaAdd;
    int areaMul;

    int numElementsPerWord32;
} PowerArea;

PowerArea powerArea[NUM_TYPES];

float powerReadSRAM32;
float powerReadDRAM32;
float powerWriteSRAM32;
float powerWriteDRAM32;

int areaSRAM;




Layer net[NUM_LAYERS];


int flexModelSpace :: InitPowerArrea()
{
    int ret = 0;
    powerArea.resize(precisions.size());
    
    powerArea[INT8].powerAdd = 0.03f;
    powerArea[INT8].powerMul = 0.2f;
    powerArea[INT8].areaAdd = 36;
    powerArea[INT8].areaMul = 282;
    powerArea[INT8].numElementsPerWord32 = 4;
    
    powerArea[INT16].powerAdd = 0.05f;
    powerArea[INT16].powerMul = 1.5f;   // XXX
    powerArea[INT16].areaAdd = 67;
    powerArea[INT16].areaMul = 1500;    // XXX
    powerArea[INT16].numElementsPerWord32 = 2;
    
    powerArea[INT32].powerAdd = 0.1;
    powerArea[INT32].powerMul = 3.1f;
    powerArea[INT32].areaAdd = 137;
    powerArea[INT32].areaMul = 3495;
    powerArea[INT32].numElementsPerWord32 = 1;
    
    powerArea[FP16].powerAdd = 0.4;
    powerArea[FP16].powerMul = 1.1f;
    powerArea[FP16].areaAdd = 1360;
    powerArea[FP16].areaMul = 1640;
    powerArea[FP16].numElementsPerWord32 = 2;
    
    powerArea[FP32].powerAdd = 0.9;
    powerArea[FP32].powerMul = 3.7f;
    powerArea[FP32].areaAdd = 4184;
    powerArea[FP32].areaMul = 7700;
    powerArea[FP32].numElementsPerWord32 = 1;
    
    return(ret);
}



int flexParam::parseNetModelFile(void)
{
    int ret = 0;
    net_model.net.resize(NUM_LAYERS);
    layer_array & net = net_model.net;
    
    for (int i = 0; i < net.size(); i++)
    {
        net[i].id = i;
        net[i].width = 64;
        net[i].height = 64;
        net[i].op = OP_CONV;
        net[i].convSize = 3 + 2*(i & 1);
        net[i].precision = unitPrecision;
        
        net[i].numMuls = net[i].width * net[i].height * net[i].convSize * net[i].convSize;
        net[i].numAdds = net[i].width * net[i].height * (net[i].convSize * net[i].convSize - 1);
        
    }
    
   
    return(ret);
}

int flexModel::  calcMaxNumOpPerLayer(const flexParam & flex_params)
{
    int ret = 0;
    const layer_array & net = flex_params.net_model.net;
    for (int i = 0; i < net.size(); i++)
    {
        if ( net[i].numMuls > maxLayerOps)
        {
            maxLayerOps = net[i].numMuls;
            maxLayer = i;
        }
    }
    
    return(ret);
}

size_t flexModel ::  clocksPerLayer(size_t numOps, int numExecUnits)
{
    
    size_t clks = (size_t)((double) numOps / (double) numExecUnits + 0.5);
    // printf("numOps %i numExecUnits %i clks %i\n", numOps, numExecUnits, clks);
    return clks;
    
}

size_t flexModel::  clocksPerNetwork(const flexParam & flex_params, int numExecUnits)
{
    size_t clks = 0;
    const layer_array & net = flex_params.net_model.net;
    
    for (int i = 0; i < net.size(); i++)
    {
        clks += clocksPerLayer(net[i].numMuls, numExecUnits);
    }
    return(clks);
}

int flexModel::  selectNumExecUnits(const flexParam & flex_params, int oldNumUnits )
{
    

    int newNumUnits = oldNumUnits / 2;
    const layer_array & net = flex_params.net_model.net;

    for (int i = 0; i < net.size(); i++) {
        
        if ((net[i].numMuls > newNumUnits) && (net[i].numMuls < oldNumUnits)){
            
            newNumUnits = (int)net[i].numMuls;
            
        }
    }
    
    return newNumUnits;
    
}

int flexModel ::  searchHWConfig(const flexModelSpace & flex_space, const flexParam & flex_params)
{
    int ret = 0;
    const layer_array & net = flex_params.net_model.net;
    
    int numExecUnits_t = (int)maxLayerOps;
    size_t totalClks_t = net.size();
    int numExecUnitsPrev = numExecUnits_t;
    size_t totalClksPrev = totalClks_t;

    while (totalClks_t < flex_params.targetNumClocks) {
    
        numExecUnitsPrev = numExecUnits_t;
        totalClksPrev = totalClks_t;
    
        numExecUnits_t = selectNumExecUnits(flex_params, numExecUnits_t );
    
        totalClks_t = clocksPerNetwork(flex_params, numExecUnits_t);
    
    //        printf("maxOps %i num units: %i num clocks %d\n", maxLayerOps, numExecUnits, totalClks);
    }
 
    if (totalClks_t > flex_params.targetNumClocks){
        numExecUnits = numExecUnitsPrev;
        totalClks = totalClksPrev;
    }
    else
    {
        numExecUnits = numExecUnits_t;
        totalClks = totalClks_t;
        
    }
    
    return (ret);
}


int flexModel ::  computePower(const flexModelSpace & flex_space, const flexParam & flex_params)
{
    int ret = 0;
    
    const layer_array & net = flex_params.net_model.net;
    const power_area_array & powerArea = flex_space.powerArea;
    
    for (int i = 0; i < net.size(); i++){
        
        int numElements = net[i].width * net[i].height;
        int numMulAddPerElement = net[i].convSize * net[i].convSize;
        power += numElements * numMulAddPerElement * powerArea[net[i].precision].powerMul;
        power += numElements * numMulAddPerElement * powerArea[net[i].precision].powerAdd;
        power += (numElements / powerArea[net[i].precision].numElementsPerWord32) * flex_space.powerReadSRAM32;
    }
    
    return ret;
}


int flexModel ::  computeArea(const flexModelSpace & flex_space, const flexParam & flex_params)
{
    int ret = 0;
    area = numExecUnits * powerArea[flex_params.unitPrecision].areaMul;
    area += numExecUnits * powerArea[flex_params.unitPrecision].areaAdd;
    
    // XXX SRAM area
    
    return ret;
}



int findMaxNumOps()
{

    return 0;
}

int clocksPerLayer(int numOps, int numExecUnits)
{
    int clks = (float) numOps / (float) numExecUnits + 0.9;
// printf("numOps %i numExecUnits %i clks %i\n", numOps, numExecUnits, clks);
    return clks;
}


int clocksPerNetwork(int numExecUnits)
{
    int clks = 0;
    for (int i = 0; i < NUM_LAYERS; i++)
    {
        clks += clocksPerLayer(net[i].numMuls, numExecUnits);
    }

    return clks;
}

int selectNumExecUnits( int oldNumUnits )
{

    int newNumUnits = oldNumUnits / 2;

    for (int i = 0; i < NUM_LAYERS; i++) {

        if ((net[i].numMuls > newNumUnits) && (net[i].numMuls < oldNumUnits)){

            newNumUnits = net[i].numMuls;

        }
    }

    return newNumUnits;
}

void initPowerArea()
{
    powerArea[INT8].powerAdd = 0.03f;
    powerArea[INT8].powerMul = 0.2f;
    powerArea[INT8].areaAdd = 36;
    powerArea[INT8].areaMul = 282;
    powerArea[INT8].numElementsPerWord32 = 4;

    powerArea[INT16].powerAdd = 0.05f;
    powerArea[INT16].powerMul = 1.5f;   // XXX
    powerArea[INT16].areaAdd = 67;
    powerArea[INT16].areaMul = 1500;    // XXX
    powerArea[INT16].numElementsPerWord32 = 2;

    powerArea[INT32].powerAdd = 0.1;
    powerArea[INT32].powerMul = 3.1f;
    powerArea[INT32].areaAdd = 137;
    powerArea[INT32].areaMul = 3495;
    powerArea[INT32].numElementsPerWord32 = 1;

    powerArea[FP16].powerAdd = 0.4;
    powerArea[FP16].powerMul = 1.1f;
    powerArea[FP16].areaAdd = 1360;
    powerArea[FP16].areaMul = 1640;
    powerArea[FP16].numElementsPerWord32 = 2;

    powerArea[FP32].powerAdd = 0.9;
    powerArea[FP32].powerMul = 3.7f;
    powerArea[FP32].areaAdd = 4184;
    powerArea[FP32].areaMul = 7700;
    powerArea[FP32].numElementsPerWord32 = 1;

    powerReadSRAM32 = 5;
    powerReadDRAM32 = 640;
}

float computePower()
{
    float power = 0;

    for (int i = 0; i < NUM_LAYERS; i++){

        int numElements = net[i].width * net[i].height;
        int numMulAddPerElement = net[i].convSize * net[i].convSize;
        power += numElements * numMulAddPerElement * powerArea[net[i].precision].powerMul;
        power += numElements * numMulAddPerElement * powerArea[net[i].precision].powerAdd;
        power += (numElements / powerArea[net[i].precision].numElementsPerWord32) * powerReadSRAM32;
    }

    return power;
}

int computeArea(int numExecUnits, int unitPrecision)
{
    int area = numExecUnits * powerArea[unitPrecision].areaMul;
    area += numExecUnits * powerArea[unitPrecision].areaAdd;

    // XXX SRAM area

    return area;
}


static
int mainLoop(int unitPrecision,
             int targetNumClocks)
{

    initPowerArea();

    for (int i = 0; i < NUM_LAYERS; i++)
    {
        net[i].id = i;
        net[i].width = 64;
        net[i].height = 64;
        net[i].op = OP_CONV;
        net[i].convSize = 3 + 2*(i & 1);
        net[i].precision = unitPrecision;
    }



        // compute number of mul/add per layer
        // compute power consumption
        // find layer with most operations
    int maxLayerOps = 0;
    for (int i = 0; i < NUM_LAYERS; i++)
    {
        net[i].numMuls = net[i].width * net[i].height * net[i].convSize * net[i].convSize;
        net[i].numAdds = net[i].width * net[i].height * (net[i].convSize * net[i].convSize - 1);
        if ( net[i].numMuls > maxLayerOps)
            maxLayerOps = net[i].numMuls;
    }

    int numExecUnits = maxLayerOps;
    int totalClks = NUM_LAYERS;
    int numExecUnitsPrev = numExecUnits;
    int totalClksPrev = totalClks;

    while (totalClks < targetNumClocks) {

        numExecUnitsPrev = numExecUnits;
        totalClksPrev = totalClks;
        
        numExecUnits = selectNumExecUnits( numExecUnits );
        
        totalClks = clocksPerNetwork(numExecUnits);
        
//        printf("maxOps %i num units: %i num clocks %d\n", maxLayerOps, numExecUnits, totalClks);
    }
    
    if (totalClks > targetNumClocks){
        numExecUnits = numExecUnitsPrev;
        totalClks = totalClksPrev;
    }

    float power = computePower();

    int area = computeArea(numExecUnits,  unitPrecision);

    printf("target clks %i precision %s\n", targetNumClocks, precisionName[unitPrecision]);
    printf("real clks %i num units %i area %i power %f\n", totalClks, numExecUnits, area, power);
    
    return 0;
}

int flexNNAnaliticalModel(flexModelSpace model_space,
                          flexParam model_params)
{
    int ret = 0;
    
    model_space.InitPowerArrea();
    model_params.parseNetModelFile();
    flexModel model;
    
    model.calcMaxNumOpPerLayer(model_params);
    model.searchHWConfig(model_space, model_params);
    model.computePower(model_space, model_params);
    model.computeArea(model_space, model_params);
    
    mainLoop(model_params.unitPrecision, (int)model_params.targetNumClocks);
    return (ret);
}
