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

static
void split(const string& str, const string& delim, vector<string>& parts) {
    size_t start, end = 0;
    parts.clear();
    while (end < str.size()) {
        start = end;
        while (start < str.size() && (delim.find(str[start]) != string::npos)) {
            start++;  // skip initial whitespace
        }
        end = start;
        while (end < str.size() && (delim.find(str[end]) == string::npos)) {
            end++; // skip to end of word
        }
        if (end-start != 0) {  // just ignore zero-length strings.
            parts.push_back(string(str, start, end-start));
        }
    }
}


int flexParam::parseNetModelFile(void)
{
    int ret = 0;
    
    layer_array & net = net_model.net;
    
    
    std::string line;
    std::ifstream m_file (model_file);
    if (m_file.is_open())
    {
        int line_id = 0;
        while ( getline (m_file,line) )
        {
            cout << line << '\n';
            std::vector<std::string> parts;
            std::string delim = " ";
            split(line, delim, parts);
            if (line.c_str()[0] == '#')
            {
                continue;
            }
            int i = line_id;
            std::string dlm = ":";
            flexLayer lyr;
            
            lyr.id = i;
            lyr.op = OP_CONV;

            // batch
            std::vector<std::string> pair;
            split(parts[3], dlm, pair);
            lyr.batch = stoi(pair[1]);
            // inputs
            split(parts[4], dlm, pair);
            lyr.n_inputs = stoi(pair[1]);
  
            // in height
            split(parts[5], dlm, pair);
            lyr.in_height = stoi(pair[1]);
            
            // in width
            split(parts[6], dlm, pair);
            lyr.in_width = stoi(pair[1]);
            
            // outputs
            split(parts[7], dlm, pair);
            lyr.n_outputs = stoi(pair[1]);
            
            // out height
            split(parts[8], dlm, pair);
            lyr.out_height = stoi(pair[1]);
            
            // out width
            split(parts[9], dlm, pair);
            lyr.out_width = stoi(pair[1]);
            
            // conv height
            split(parts[10], dlm, pair);
            lyr.conv_height = stoi(pair[1]);
            
            // conv width
            split(parts[11], dlm, pair);
            lyr.conv_width = stoi(pair[1]);

            lyr.precision = unitPrecision;
            
            size_t n_convs = (size_t)lyr.batch * (size_t)lyr.n_inputs * (size_t)lyr.n_outputs * (size_t)lyr.out_width * (size_t)lyr.out_height;
            lyr.numMuls = n_convs * (size_t)lyr.conv_height * (size_t)lyr.conv_width;
            lyr.numAdds = n_convs * ((size_t)lyr.conv_height * (size_t)lyr.conv_width - 1);
            
            net.push_back(lyr);
            line_id++;
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

size_t flexModel::clocksPerLayer(size_t numOps, size_t numExecUnits_t)
{
    
    size_t clks_t = (size_t)((double) numOps / (double) numExecUnits_t + 0.5);
//    printf("numOps %ld numExecUnits %ld clks %ld\n", numOps, numExecUnits_t, clks_t);
    return clks_t;
    
}

size_t flexModel::  clocksPerNetwork(const flexParam & flex_params, size_t numExecUnits_t)
{
    size_t clks_t = 0;
    const layer_array & net = flex_params.net_model.net;
    
    for (int i = 0; i < net.size(); i++)
    {
        clks_t += clocksPerLayer(net[i].numMuls, numExecUnits_t);
    }
    return(clks_t);
}

size_t flexModel::  selectNumExecUnits(const flexParam & flex_params, size_t oldNumUnits )
{
    
    size_t newNumUnits_t = oldNumUnits / 2;
    const layer_array & net = flex_params.net_model.net;

    for (int i = 0; i < net.size(); i++) {
        
        if ((net[i].numMuls > newNumUnits_t) && (net[i].numMuls < oldNumUnits)){
            
            newNumUnits_t = net[i].numMuls;
            
        }
    }
    
    return (newNumUnits_t);
    
}

int flexModel ::  searchHWConfig(const flexModelSpace & flex_space, const flexParam & flex_params)
{
    int ret = 0;
    
    calcMaxNumOpPerLayer(flex_params);

    
    const layer_array & net = flex_params.net_model.net;
    size_t numExecUnits_t = maxLayerOps;
    size_t totalClks_t = net.size();
    size_t numExecUnitsPrev = numExecUnits_t;
    size_t totalClksPrev = totalClks_t;

    while (totalClks_t < flex_params.targetNumClocks) {
    
        numExecUnitsPrev = numExecUnits_t;
        totalClksPrev = totalClks_t;
    
        numExecUnits_t = selectNumExecUnits(flex_params, numExecUnits_t );
    
        totalClks_t = clocksPerNetwork(flex_params, numExecUnits_t);
    
 //       printf("maxOps %ld num units: %ld num clocks %ld\n", maxLayerOps, numExecUnits_t, totalClks);
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
        
        size_t numElements = net[i].batch * net[i].n_inputs * net[i].n_outputs * net[i].out_width * net[i].out_height;
        size_t numMulAddPerElement = net[i].conv_height * net[i].conv_width;
        
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
 
        printf("target clks %ld precision %s\n", model_params.targetNumClocks, model_space.precisions[model_params.unitPrecision].c_str());
        printf("real clks %ld num units %ld area %ld power %f\n", model.totalClks, model.numExecUnits, model.area, model.power);
    }
 
    return (ret);
}
