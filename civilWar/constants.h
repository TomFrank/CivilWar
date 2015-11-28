//
//  constants.h
//  civilWar
//
//  Created by ZZJ on 15/11/21.
//  Copyright © 2015年 ZZJ. All rights reserved.
//

#ifndef constants_h
#define constants_h

#include <fstream>

class constants {
public:
    static int groosMass;
    static int civilNum ;
    static int uniBorder ;
    static int scanRPerCivilDg ;
    static int massPerCivilDg ;
    static int hideCD ;
    static int cleanCD ;
public:
    static void readConstants(char **);
    static void printConstants(std::ofstream&);
    static void updateConstants();
    static void checkValidity();
    
private:
    static const int constantsNum = 7;
    //bind all the consts to a single variable
    static int* uniConstants[constantsNum];

};

int* constants::uniConstants[] = {
    &(constants::groosMass)
    , &(constants::civilNum)
    , &(constants::uniBorder)
    , &(constants::scanRPerCivilDg)
    , &(constants::massPerCivilDg)
    , &(constants::hideCD)
    , &(constants::cleanCD)
};

#endif /* constants_h */
