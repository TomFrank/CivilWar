//
//  main.cpp
//  civilWar
//
//  Created by ZZJ on 15/11/21.
//  Copyright © 2015年 ZZJ. All rights reserved.
//
// 
// My universe's description:
// At the begining,7 constants are defined,so is the number of civils.
// fundamental principles:
// 1.hideCD is always smaller than cleanCD.
// 2.At the begining of the world,There are 3 civilizations at least.
// 3.Only one can attack others when its civilDg is greater.
// 4.When attacking,both sides of civils will get injured.Including civilDg and mass lossing.
// 5.universal time
// 
// 
// civil-related principle:
// 1....
// 
// 
// program-related:
// 1.Fowlling two interval times are aimed to contral the frequency of monitoring.
// 2....
// 
// 
// Usage : civilWar [runTimes] [groosMass,civilNum,uniBorder,scanRPerCivilDg,massPerCivilDg,hideCD,cleanCD]
// 
// runTimes                 times that new universe will be created.
// 
// if gave no args,run with default args : runTimes=100

#include <fstream>
#include <iostream>
#include <random>
#include <vector>
#include <string>
#include <ctime>
#include <cmath>
#include <chrono>
#include <thread>
#include <mutex>

#include "constants.h"
#include "civilization.h"

using namespace std;

//main information
ofstream fout("DarkForest.txt");
//detialed information
ofstream data("Detials.csv");
//storing importent constants
ofstream data2("General.csv", ios::app);

//program-related constants
//const auto thread1SleepTime=chrono::milliseconds(100);
const auto moniterSleepTime = chrono::microseconds(1000);
//const auto thread3SleepTime = chrono::microseconds(0);
volatile bool endThread = false;
mutex mtx;

//random generator
mt19937 randEng;//(time(NULL));
normal_distribution<> rate(1.0, 0.01);

inline void constants::readConstants(char **argv) {
    for (int i = 0; i != constantsNum; ++i) {
        *constants::uniConstants[i] = stoi(argv[i + 2]);
    }
}

inline void constants::printConstants(ofstream& logFile) {
    for (const auto &c : uniConstants) {
        logFile << "," << *c;
    }
}

inline void constants::updateConstants() {
    //update consts
    for (auto &c : uniConstants) {
        (*c) *= rate(randEng);
    }
}

inline void constants::checkValidity() {
    //principles
    if (hideCD > cleanCD) {
        swap(hideCD,cleanCD);
    }
    if (civilNum <= 2) {
        civilNum = 3;
    }
    for (auto& c :uniConstants) {
        if (*c <= 0) {
            *c = 10;
        }
    }
}

//universal constants
int constants::groosMass = 1e8;
int constants::civilNum = 1e2;
int constants::uniBorder = 1e8;
int constants::scanRPerCivilDg = 1e3;
int constants::massPerCivilDg = 1e1;
int constants::hideCD = 1e3;
int constants::cleanCD = 1e5;

unsigned civilization::unusedMass = 0;
unsigned civilization::uniTime = 0;
unsigned civilization::circle = 0;
unsigned civilization::runTimes = 0;

normal_distribution<> normalCivilDgChgTime(1.5, 0.2);
normal_distribution<> hideCivilDgChgTime(1.2, 0.1);
uniform_int_distribution<> coords(0, constants::uniBorder);
uniform_int_distribution<> initialMass((constants::groosMass / constants::civilNum) / 4, constants::groosMass / constants::civilNum);

//file IO function
void start();
void readData(int, char **);
void writeData(bool, ofstream &detialedFile = data, ofstream &logFile = data2);
void writeTitle(ofstream&);

civilization::civilization(decltype(name) n): name(n) {
    civilDg = 2;
    scanR = 2;
    deadReason = deadReasons::unknown;
    whoKill = constants::civilNum;
    mass = initialMass(randEng);
    x = coords(randEng);
    y = coords(randEng);
    hideMode = false;
    cleanGen = false;
}

inline bool civilization::isAlive() const {
    if (civilDg != 0)return true;
    return false;
}

inline void civilization::setDie(deadReasons dR) {
    civilDg = 0;
    unusedMass += mass;
    mass = 0;
    deadReason = dR;
}

inline void civilization::setHide(bool b) {
    hideMode = b;
}

inline void civilization::setClean(bool b) {
    cleanGen = b;
}

inline void civilization::massTrans(decltype(mass) m) {
    mass -= m;
    unusedMass += m;
}

inline bool civilization::massCollect(decltype(mass) m) {
    if (m > unusedMass) {
        mass += m;
        unusedMass -= m;
        return true;
    } else return false;
    
}

vector<civilization> uniScty;
vector<vector<int>> civilDis;

void init() {
    
    endThread = false;
    civilization::uniTime = 0;
    civilization::unusedMass = 0;
    constants::updateConstants();
    constants::checkValidity();
    
    //clear to write
    uniScty.clear();
    //init the vector
    for (auto i = 0; i < constants::civilNum; ++i) {
        uniScty.push_back(i);
    }
    //-----------------------------------importent!!!-----Once there was a bug! :(-------------------
    //resize the vector
    civilDis.resize(constants::civilNum);
    for (int i = 0; i != constants::civilNum; ++i) {
        civilDis[i].resize(constants::civilNum);
    }
    
    //auto square = [&](decltype(civilization::x) a) {return a * a;};
    
    //must open compile option -fopenmp -lpthread
    /***********************/
//#pragma omp parallel for
    /***********************/
    //----------------------------------------------------------------------------------------------
    for (auto i = 0; i < constants::civilNum; ++i) {
        for (auto j = 0; j < i; ++j) {
            //---------------------------------------maybe OVERFLOW!!!!------------------------------
            //civilDis[i][j] = sqrt(square(uniScty[i].x - uniScty[j].x) + square(uniScty[i].y - uniScty[j].y));
            civilDis[i][j] = hypot(uniScty[i].x-uniScty[j].x,uniScty[i].y-uniScty[j].y);
            //---------------------------------------------------------------------------------------
        }
    }
    //init unused mass
    unsigned usedMass = 0;
    for (const auto &elem : uniScty) {
        usedMass += elem.mass;
    }
    civilization::unusedMass = constants::groosMass - usedMass;
}

void civilDgBom() {
    //while(true) {
    for (auto &c : uniScty) {
        if (c.isAlive()) {
            //tech explosion
            //god-civils need not dev
            if (c.civilDg * constants::massPerCivilDg <= constants::groosMass / 3) {
                if (c.hideMode) {
                    c.civilDg = static_cast<double>(c.civilDg) * hideCivilDgChgTime(randEng);
                } else {
                    c.civilDg = static_cast<double>(c.civilDg) * normalCivilDgChgTime(randEng);
                }
            }
            //update scanR
            if (c.civilDg * constants::scanRPerCivilDg < constants::uniBorder) {
                c.scanR = c.civilDg * constants::scanRPerCivilDg;
            } else {
                c.scanR = constants::uniBorder;
            }
            //open hide mode in low civil degree
            if (c.civilDg > constants::hideCD && !c.cleanGen) {
                c.setHide(true);
            }
            //high-civils begin the clean mode,shut down hide mode
            if (c.civilDg > constants::cleanCD) {
                c.setClean(true);
                c.setHide(false);
            }
            //if a high-civil had been attacked,it would be protected
            if (
                c.cleanGen               //make sure it is a high-civil
                && (c.civilDg < constants::cleanCD) //male sure if it got injured
                ) {
                c.setHide(true);
            }
        }
    }
    //this_thread::sleep_for(thread1SleepTime);
    ++civilization::uniTime;
    //}
}

void civilExp() {
    writeTitle(data);
    while (!endThread) {
        
        civilDgBom();
        
        for (auto &c : uniScty) {
            if (c.isAlive()) {
                //collect unused mass
                c.massCollect((static_cast<double>(c.scanR) / constants::uniBorder)*civilization::unusedMass);
                //if mass were running out,this civilization would DIE!
                if (c.hideMode) {
                    //hide mode civilizations dev more slowly
                    if (c.mass >= c.civilDg * constants::massPerCivilDg / 100) {
                        //need to control
                        c.massTrans(c.civilDg * constants::massPerCivilDg / 100);
                    } else {
                        c.setDie(deadReasons::die_of_hunger);
                    }
                } else {
                    //normal civilization
                    if (c.mass >= c.civilDg * constants::massPerCivilDg) {
                        c.massTrans(c.civilDg * constants::massPerCivilDg);
                    } else {
                        c.setDie(deadReasons::die_of_hunger);
                    }
                }
            }
            
            //begin to clean!
            for (auto &d : uniScty) {
                if (
                    c.isAlive() && d.isAlive()
                    && !c.hideMode && !d.hideMode
                    && c.cleanGen
                    && c.civilDg > d.civilDg
                    && c.scanR > civilDis[c.name][d.name]
                    && c.scanR > civilDis[d.name][c.name]
                    && c.civilDg > (1 / (c.civilDg - d.civilDg)) * 100
                    && c.mass > (1 / (c.civilDg - d.civilDg) / constants::massPerCivilDg) * 100
                    ) {
                    //I would like to use 1% resourse to clean
                    //-------------------------------need more ...-----------------------------
                    //begin to KILL the other!
                    d.setDie(deadReasons::being_killed);
                    d.whoKill = c.name;
                    //get injured as well
                    c.civilDg -= 1 / (c.civilDg - d.civilDg);
                    //-------------------------------need to adjust--------------------------------
                    c.massTrans(1 / (c.civilDg - d.civilDg)*constants::massPerCivilDg);
                }
            }
        }
        writeData(false);
        //this_thread::sleep_for(thread2SleepTime);
    }
}

void analyse(double hungerRate,double killRate){
    if (hungerRate>=50) {
        fout << "Too many civilizations are died of hunger!\n";
        constants::massPerCivilDg *= 0.9;
        constants::scanRPerCivilDg *= 1.1;
    }else{
        fout << "Too many civilizations are killed!\n";
        constants::hideCD *= 0.9;
        constants::cleanCD *= 1.1;
    }
    fout << "\n";
}

void monitor() {
    static decltype(civilization::unusedMass) lastData = 0;
    unsigned countc, counth, countk, counthd;
    double deathRate, cleanRate, hideRate, hungerRate, killRate;
    
    while (!endThread) {
        countc = counthd = counth = countk = 0;
        for (const auto &c : uniScty) {
            if (c.cleanGen)++countc;
            if (c.hideMode)++counthd;
            if (c.deadReason == deadReasons::die_of_hunger)++counth;
            if (c.deadReason == deadReasons::being_killed)++countk;
        }
        
        auto calcRate = [&](decltype(countc) a) {return static_cast<double>(a) / constants::civilNum * 100;};
        
        cleanRate = calcRate(countc);
        hideRate = calcRate(counthd);
        hungerRate = calcRate(counth);
        killRate = calcRate(countk);
        deathRate = hungerRate + killRate;
        
        //cout << "death rate:" << deathRate << "\n";
        //mtx.lock();
        fout << "Time: " << civilization::uniTime << "\n";
        fout
        << "There are " << hideRate << "% civilizations opening the hide mode.\n"
        << "There are " << cleanRate << "% civilizations having the cleaning ability.\n"
        << "There are " << hungerRate << "% civilizations dying of hunger.\n"
        << "There are " << killRate << "% civilizations being killed.\n"
        << "There are " << deathRate << "% civilizations being conquered or dead.\n"
        << "Total unused mass:" << civilization::unusedMass << "\n\n";
        //mtx.unlock();
        //no civilizations,universe ends
        if (deathRate >= 99.999999) {
            fout
            << "--------------------------------------------\n"
            << "All civilizations are dead! The universe " << civilization::circle << " ends!\n"
            << "This universe has run " << civilization::uniTime << " universe time.\n"
            << "This program has run " << clock() / static_cast<double>(CLOCKS_PER_SEC) << "s.\n"
            << "--------------------------------------------\n";
            cout << civilization::circle << ":" << civilization::uniTime << " uTime\n";
            
            analyse(hungerRate, killRate);
            
            //write log file
            writeData(true);
            
            ++civilization::circle;
            endThread = true;
            return;
        }
        if (lastData == civilization::unusedMass) {
            fout << "***********************************************\n"
                 << "This is a stable universe.Nothing is changing.\n"
                 << "***********************************************\n\n";
            civilization::uniTime = 0;
            writeData(true);
            ++civilization::circle;
            endThread = true;
            return;
        }
        lastData = civilization::unusedMass;
        //monitor update rate
        this_thread::sleep_for(moniterSleepTime);
    }
}

void civilUpd() {
    //thread thread1(civilDgBom);
    thread thread2(civilExp);
    thread thread3(monitor);
    //thread1.join();
    thread2.join();
    thread3.join();
}

void start() {
    do {
        init();
        civilUpd();
    }
    while (civilization::circle < civilization::runTimes);
    fout.close();
    data.close();
    data2.close();
}

void readData(int argc, char* argv[]) {
    switch (argc) {
        case 8+1:
            civilization::runTimes = stoi(argv[1]);
            constants::readConstants(argv);
            break;
            // use the default args
        case 1+1:
            civilization::runTimes = stoi(argv[1]);
            break;
        case 0+1:
            civilization::runTimes = 100;
            break;
        default:
            cerr << "wrong args!";
            exit(EXIT_FAILURE);
    }
}

void writeTitle(ofstream &file) {
    if (civilization::circle > 0)return;
    file << "time" << ","
         << "name" << ","
         << "civDg" << ","
         << "hide" << ","
         << "clean" << ","
         << "DeadReason" << ","
         << "whokill" << ","
         << "mass" << ","
         << "\n";
    data2 << "time" << ","
         << "groosMass" << ","
         << "civilNum" << ","
         << "uniBorder" << ","
         << "scanRPerCivilDg" << ","
         << "massPerCivilDg" << ","
         << "hideCD" << ","
         << "cleanCD" << "\n";
}

void writeData(bool ifLog, ofstream &detialedFile, ofstream &logFile) {
    if (ifLog) {
        logFile << civilization::uniTime;
        constants::printConstants(logFile);
        logFile << "\n";
    }
    // detialedFile
    //         << uniTime
    //         << "," << unusedMass
    //         << "\n";
    // mtx.lock();
    // for (const auto &c : uniScty) {
    // 	detialedFile
    // 	        << uniTime << ","
    // 	        << c.name << ","
    // 	        << c.civilDg << ","
    // 	        << c.hideMode << ","
    // 	        << c.cleanGen << ","
    // 	        << c.deadReason << ","
    // 	        << c.whoKill << ","
    // 	        << c.mass
    // 	        << "\n";
    // }
    // mtx.unlock();
}

int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);
    readData(argc, argv);
    start();
    return 0;
}

