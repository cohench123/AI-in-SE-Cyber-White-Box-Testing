#include "config.h"
#include <iostream>
#include <fstream>
#include <string>

Config::Config() {
    timeLimit = 0;
    refreshRate = 0;
    regionName = "";
}

void Config::initiateConfig(string configName, string &regionName) {

    string currLine;
    ifstream file(configName); //opens config file

while (getline(file, currLine)) {
    size_t pos = currLine.find(':');
    if (pos != string::npos) {
        string key = currLine.substr(0, pos);
        string value = currLine.substr(pos + 1);

        while (!value.empty() && value.front() == ' ') value.erase(0, 1);

        //stores config values in according variables
        if (key == "Region Layout") {
            regionName = value; }
        else if (key == "Time Limit") {
            timeLimit = stoi(value);}
        else if (key == "Refresh Rate") {
            refreshRate = stoi(value);}

        
        
        }
    }

    file.close();
}
