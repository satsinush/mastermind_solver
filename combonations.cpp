#include <iostream>
#include <cmath>
#include <vector>
#include <random>
#include <conio.h>

using namespace std;

namespace combonations{
    int rtnm(float num, int multiple){
        return(ceil(num/multiple)*multiple);
    }

    vector<vector<int>> getCombonations(int numChars, int length){
        int numCombos = pow(numChars,length);

        vector<vector<int>> repeatingLists;

        for(int r=0; r<length; r++){
            vector<int> row;
            row.reserve(numCombos);

            int last;
            int currentIndex = -1;

            for(int i=0; i<numCombos;i++){
                int multiple = numCombos/(pow(numChars,r+1));

                int roundedNum = rtnm(r*numCombos + i+1, multiple);
                int index = floor(roundedNum-r*numCombos);

                if(index != last){
                    currentIndex += 1;
                    if(currentIndex >= numChars){
                        currentIndex = 0;
                    }
                    last = index;
                }
                row.push_back(currentIndex);
            }
            repeatingLists.push_back(row);
        }

        vector<vector<int>> combos;
        combos.reserve(numCombos);

        for(int i=0; i<numCombos; i++){
            vector<int> c;
            c.reserve(numChars);

            for(int index=0; index<length; index++){
                c.push_back(repeatingLists[index][i]);
            }
            combos.push_back(c);
        }

        return(combos);
    }
}