#include <iostream>
#include "residential.h"
#include "region.h"
#include <vector>
using namespace std;

bool residential(int i, int j, vector<vector<pair<char, int>>> &region) {

    pair<char, int> p = region[i][j];

    if (p.second == 0) {  //if population of current cell is 0

        int x[] = {-1, -1, -1, 0, 0, 1, 1, 1}; //Establishing an array that will check adjacent cells
        int y[] = {-1, 0, 1, -1, 1, -1, 0, 1};

        int rows = region.size(); //Gathering bounds info to check later
        int cols = region[0].size();

        for (int k = 0; k < 8; k++) { //Looping for 8 checks
            
            int ai = i + x[k]; //sets value for adjacent's position
            int aj = j + y[k];

            if (ai >= 0 && ai < rows && aj >= 0 && aj < cols) { //Ensures current adjacent cell is within bounds of region
                if (region[ai][aj].first == 'T' || region[ai][aj].first == '#' || region[ai][aj].second >= 1) { //Check adjacent cell to be a powerline or powerline over road or have population >= 1
                    return true; //Returns true for growth
                }
            }
        }
        return false; //No adjacent powerline, so no growth
    }



    else if (p.second == 1) {  //if population of current cell is 1

        int x[] = {-1, -1, -1, 0, 0, 1, 1, 1}; //Establishing an array that will check adjacent cells
        int y[] = {-1, 0, 1, -1, 1, -1, 0, 1};
        int count = 0;

        int rows = region.size(); //Gathering bounds info to check later
        int cols = region[0].size();

        for (int k = 0; k < 8; k++) { //Looping for 8 checks
            
            int ai = i + x[k]; //sets value for adjacent's position
            int aj = j + y[k];

            if (ai >= 0 && ai < rows && aj >= 0 && aj < cols) { //Ensures current adjacent cell is within bounds of region
                if (region[ai][aj].second >= 1) { //Check adjacent cell population to be 1 or more
                    count++;
                }
            }
        }
        if (count >= 2) {
            return true;
        }
        else {
            return false;
        }

    }

    else if (p.second == 2) {  //if population of current cell is 2

        int x[] = {-1, -1, -1, 0, 0, 1, 1, 1}; //Establishing an array that will check adjacent cells
        int y[] = {-1, 0, 1, -1, 1, -1, 0, 1};
        int count = 0;

        int rows = region.size(); //Gathering bounds info to check later
        int cols = region[0].size();

        for (int k = 0; k < 8; k++) { //Looping for 8 checks
            
            int ai = i + x[k]; //sets value for adjacent's position
            int aj = j + y[k];

            if (ai >= 0 && ai < rows && aj >= 0 && aj < cols) { //Ensures current adjacent cell is within bounds of region
                if (region[ai][aj].second >= 2) { //Check adjacent cell to have population >= 2
                    count++; //Iterate counter for adjacent cells with population >= 2
                }
            }
        }
        if (count >= 4) { //Check number of adjacent cells with population >= 2
            return true;
        }
        else {
            return false;
        }

    }

    else if (p.second == 3) {  //if population of current cell is 3

        int x[] = {-1, -1, -1, 0, 0, 1, 1, 1}; //Establishing an array that will check adjacent cells
        int y[] = {-1, 0, 1, -1, 1, -1, 0, 1};
        int count = 0;

        int rows = region.size(); //Gathering bounds info to check later
        int cols = region[0].size();

        for (int k = 0; k < 8; k++) { //Looping for 8 checks
            
            int ai = i + x[k]; //sets value for adjacent's position
            int aj = j + y[k];

            if (ai >= 0 && ai < rows && aj >= 0 && aj < cols) { //Ensures current adjacent cell is within bounds of region
                if (region[ai][aj].second >= 3) { //Check adjacent cell to have population >= 2
                    count++; //Iterate counter for adjacent cells with population >= 3
                }
            }
        }
        if (count >= 6) { //Check number of adjacent cells with population >= 3
            return true;
        }
        else {
            return false;
        }

    }

    else if (p.second >= 4) {  //if population of current cell is 4

        int x[] = {-1, -1, -1, 0, 0, 1, 1, 1}; //Establishing an array that will check adjacent cells
        int y[] = {-1, 0, 1, -1, 1, -1, 0, 1};
        int count = 0;

        int rows = region.size(); //Gathering bounds info to check later
        int cols = region[0].size();

        for (int k = 0; k < 8; k++) { //Looping for 8 checks
            
            int ai = i + x[k]; //sets value for adjacent's position
            int aj = j + y[k];

            if (ai >= 0 && ai < rows && aj >= 0 && aj < cols) { //Ensures current adjacent cell is within bounds of region
                if (region[ai][aj].second >= 4) { //Check adjacent cell to have population >= 2
                    count++; //Iterate counter for adjacent cells with population >= 4
                }
            }
        }
        if (count >= 8) { //Check number of adjacent cells with population >= 4
            return true;
        }
        else {
            return false;
        }

    }
    else{
        return false;
    }

}