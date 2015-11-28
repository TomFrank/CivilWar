//
//  civilization.h
//  civilWar
//
//  Created by ZZJ on 15/11/21.
//  Copyright © 2015年 ZZJ. All rights reserved.
//

#ifndef civilization_h
#define civilization_h

//death reason
enum class deadReasons {unknown, die_of_hunger, being_killed};

class civilization {
public:
    //basic info
    int name;
    int mass;
    int civilDg;
    int x, y;
    int scanR;
    //advanced info
    bool hideMode;
    bool cleanGen;
    //extra info for analyse
    deadReasons deadReason;
    int whoKill;
    //universal variables
    static unsigned unusedMass;
    static unsigned uniTime;
    static unsigned circle;
    static unsigned runTimes;
public:
    civilization(decltype(name));
    void setDie(deadReasons);
    void setHide(bool);
    bool isAlive()const;
    void setClean(bool);
    void massTrans(decltype(mass));
    bool massCollect(decltype(mass)) ;
};

#endif /* civilization_h */
