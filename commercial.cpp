#include <iostream>
#include <vector>
#include "commercial.h"
#include "region.h"
using namespace std;

bool commercial(vector<vector<pair<char, int>>> &region, int i, int j, int workers, int goods) {

    pair<char, int> p = region[i][j];

    if (p.second == 0) {

        int x[] = {-1, -1, -1, 0, 0, 1, 1, 1}; //Establishing an array that will check adjacent cells
        int y[] = {-1, 0, 1, -1, 1, -1, 0, 1};

        int rows = region.size(); //Gathering bounds info to check later
        int cols = region[0].size();

        for (int k = 0; k < 8; k++) { //Looping for 8 adjacent checks
            
            int ai = i + x[k]; //sets value for adjacent's position
            int aj = j + y[k];

            if (ai >= 0 && ai < rows && aj >= 0 && aj < cols) { //Ensures current adjacent cell is within bounds of region
                if (region[ai][aj].first == 'T' || region[ai][aj].first == '#' || region[ai][aj].second >= 1) { //Check adjacent cell to be a powerline or powerline over road or have adjacent population >= 1
                    if (workers >= 1) { //Checks for available workers
                        if(goods >= 1) { 
                            return true; //Returns true for growth
                        }
                    }
                }
            }
        }
        return false; //No adjacent powerline or available workers/goods, so no growth

    }

    if (p.second >= 1) {

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
            if (workers >= 1) { //Check for available workers
                if (goods >= 1) {
                    return true;
                }
            }
        }
        else {
            return false;
        }

    }
    else{
        return false;
    }

}