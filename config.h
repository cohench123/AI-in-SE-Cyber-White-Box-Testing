#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <string>
using namespace std;

class Config {

    public:
    int timeLimit;
    int refreshRate;
    string regionName;

    Config();

    void initiateConfig(string configName, string &regionName);
};

#endif