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
#include <functional>

using T = int;

class constants {
public:
    static T groosMass;
    static T civilNum;
    static T uniBorder;
    static T scanRPerCivilDg;
    static T massPerCivilDg;
    static T hideCD;
    static T cleanCD;

    constants(T gm=1,T cn=1,T ub=1,T src=1,T mpc=1,T hd=1,T cd=1){}
    static void readConstants(char **);
    static void printConstants(std::ofstream&);
    static void updateConstants();
    static void checkValidity();
    
private:
    static const uint constantsNum = 7;
    //bind all the consts to a single variable
    //static T* uniConstants[constantsNum];
    static std::reference_wrapper<T> uniC[constantsNum];
};

std::reference_wrapper<T> uniC[] = {constants::groosMass,constants::civilNum};

//T* constants::uniConstants[] = {
//      &(constants::groosMass)
//    , &(constants::civilNum)
//    , &(constants::uniBorder)
//    , &(constants::scanRPerCivilDg)
//    , &(constants::massPerCivilDg)
//    , &(constants::hideCD)
//    , &(constants::cleanCD)
//};

#endif /* constants_h */
