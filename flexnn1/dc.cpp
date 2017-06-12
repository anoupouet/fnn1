#include "flexnn_internal.h"



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


/*funcion that show the help information*/
static
void showhelpinfo(char *s)
{
    cout<<"Usage:   "<<s<<" [-option] [argument]"<<endl;
    cout<<"option:  "<<"-h  show help information"<<endl;
    cout<<"         "<<"-p  presision:  int8, int16, int32, fp16, fp32; default: fp32"<<endl;
    cout<<"         "<<"-c  targret clock; default: 1000"<<endl;
    cout<<"         "<<"-m  model definition file; default: flexnn.ad"<<endl;
    cout<<"         "<<"-v  show version infomation"<<endl;
    cout<<"example: "<<s<<" -pfp16 -c10000 -mmymodel.ad"<<endl;
}




static
int argParser (int argc, char *argv[], flexParam & param, flexModelSpace & prec_table)
{
    char tmp;
    /*if the program is ran witout options ,it will show the usgage and exit*/
    if(argc == 1)
    {
        showhelpinfo(argv[0]);
        exit(1);
    }
    /*use function getopt to get the arguments with option."hp:c:m:v" indicate
     that option h,v are the options without arguments while u,p,s are the
     options with arguments*/
    while((tmp=getopt(argc,argv,"hp:c:m:v"))!=-1)
    {
        switch(tmp)
        {
                /*option h show the help infomation*/
            case 'h':
                showhelpinfo(argv[0]);
                break;
                /*option c */
            case 'c':
                cout<<"Target clock is "<<optarg<<endl;
                param.targetNumClocks = std::stoi(optarg);
                break;
                /*option p */
            case 'p':
            {
                std::string data = std::string(optarg);
                transform(data.begin(), data.end(), data.begin(),::toupper);
                cout<<"Target precision is "<< data <<endl;
                
                string_array::iterator it;
                string_array str_arr = prec_table.precisions;
                it = std::find(str_arr.begin(), str_arr.end(), data);
                
                if (it != prec_table.precisions.end()) {
                    //cerr << "Found at position " << std::distance(prec_table.begin(), it) << endl;
                    param.unitPrecision = (int)std::distance(str_arr.begin(), it);
                }
            }
                break;
                /*option m */
            case 'm':
                cout<<"Model file is "<<optarg<<endl;
                param.model_file = optarg;
                break;
                /*option v show the version infomation*/
            case 'v':
                cout<<"The current version is 0.1"<<endl;
                break;
                /*invail input will get the heil infomation*/
            default:
                showhelpinfo(argv[0]);
                break;
        }
    }
    
    return (0);
}

int flexModelSpace::Init(int argc, const char * argv[], flexParam & params)
{
    int ret = 0;
    ret = argParser (argc,(char**)argv, params, *this);
    if (!ret)
    {
        ret = InitPowerArrea();
    }
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
    string line;
    ifstream m_file (model_file);
    if (m_file.is_open())
    {
        while ( getline (m_file,line) )
        {
            cout << line << '\n';
        }
        m_file.close();
    }
    
    else
    {
        cout << "Unable to open model definition file : " << model_file << endl;
        ret = 1;
    }
    
   
    return(ret);
}

int flexParam::Init(const flexModelSpace & model_space )
{
    int ret = 0;
    ret = parseNetModelFile();
    return(ret);

}




int flexModel::Init(void)
{
    int ret = 0;
    return(ret);
}

int flexModel::calcMaxNumOpPerLayer(const flexParam & flex_params)
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

size_t flexModel::clocksPerLayer(size_t numOps, int numExecUnits)
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
    
    calcMaxNumOpPerLayer(flex_params);

    
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
    const power_area_array & powerArea = flex_space.powerArea;
    
    area = numExecUnits * powerArea[flex_params.unitPrecision].areaMul;
    area += numExecUnits * powerArea[flex_params.unitPrecision].areaAdd;
    
    // XXX SRAM area
    
    return ret;
}




int flexNNAnaliticalModel(int argc, const char * argv[])
{
    int ret = 0;
    flexModelSpace model_space;
    flexParam model_params;
    
    if (!(ret = model_space.Init(argc, argv, model_params) ))
    {
        
        model_params.Init(model_space);
        
        flexModel model;
    
        model.searchHWConfig(model_space, model_params);
        model.computePower(model_space, model_params);
        model.computeArea(model_space, model_params);
 
        printf("target clks %i precision %s\n", (int)model_params.targetNumClocks, model_space.precisions[model_params.unitPrecision].c_str());
        printf("real clks %i num units %i area %i power %f\n", (int)model.totalClks, model.numExecUnits, (int)model.area, model.power);
    }
 
    return (ret);
}
