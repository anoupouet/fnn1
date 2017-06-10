//
//  main.cpp
//  flexnn1
//
//  Created by Alexander Lyashevsky on 6/9/17.
//  Copyright © 2017 Alexander Lyashevsky. All rights reserved.
//

/*In the C++ header file "unistd.h", function getopt can get the arguments with different options.*/
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <locale>         // std::locale, std::toupper
#include <iterator>
#include <algorithm>
#include <array>
#include <vector>


#define NUM_TYPES   5
enum {INT8, INT16, INT32, FP16, FP32};
extern "C" const char *precisionName[NUM_TYPES] = {"INT8", "INT16", "INT32", "FP16", "FP32"};
typedef std::vector<std::string> precTbl;

extern "C" int mainLoop(int unitPrecision,
             int targetNumClocks);

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


static
void showhelpinfo(char *s);
using namespace std;
int myParser (int argc, char *argv[], flexParam & param, precTbl & prec_table)
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
                std::locale loc;
                std::string data = std::string(optarg);
                transform(data.begin(), data.end(), data.begin(),::toupper);
                cout<<"Target precision is "<< data <<endl;
                
                precTbl::iterator it;
                it = std::find(prec_table.begin(), prec_table.end(), data);

                if (it != prec_table.end()) {
                    //cerr << "Found at position " << std::distance(prec_table.begin(), it) << endl;
                    param.unitPrecision = (int)std::distance(prec_table.begin(), it);
                }
            }
                break;
                /*option m */
            case 'm':
                cout<<"Model file is "<<optarg<<endl;
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

/*funcion that show the help information*/
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


int main(int argc, const char * argv[]) {
    // insert code here...
// initialization
    flexParam flex_param;
    precTbl precNames(precisionName, precisionName + NUM_TYPES);
    
    myParser (argc,(char**)argv, flex_param, precNames);
    
    
    mainLoop(flex_param.unitPrecision,flex_param.targetNumClocks);
    
    return 0;
}
