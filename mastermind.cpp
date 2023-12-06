#include <iostream>
#include <cmath>
#include <vector>
#include <random>
#include <conio.h>
#include <string>
#include <algorithm>
#include <unordered_map>

#include "combonations.cpp"
#include "profiler.cpp"

using namespace std;

Profiler::Profiler profiler;

struct comparisonOutput{
    int correctColors = 0;
    int correctSpot = 0;

    comparisonOutput(){
    }

    comparisonOutput(int a, int b){
        this->correctColors = a;
        this->correctSpot = b;
    }

    bool operator==(const comparisonOutput& lhs)
    {
        return(this->correctColors == lhs.correctColors && this->correctSpot==lhs.correctSpot);
    }
};

vector<string> colors;
int numColors;
int codeLength;

vector<int> code;

vector<vector<int>> allCombos;
vector<vector<int>> guesses;
vector<comparisonOutput> outputs;
vector<comparisonOutput> possibleOutputs;

std::random_device rand_dev;
std::mt19937 rng(rand_dev());

float randomFloat(double min, double max){
    //returns a continous random float from [min, max)
    std::uniform_real_distribution<float> dist(min,max);
    return(dist(rng));
}
int randomInt(int min, int max){
    //returns a discrete random number integer from [min, max]
    std::uniform_int_distribution<int> dist(min,max);
    return(dist(rng));
}

vector<int> randomCode(int numC, int size){
    vector<int> c;
    for(int i=0; i<size; i++){
        c.push_back(randomInt(0,numC-1));
    }
    return(c);
}

comparisonOutput compare(vector<int>& code1, vector<int>& code2){
    comparisonOutput output;
    /*if(code1.size() != code2.size()){
        return(output);
    }*/
    vector<int> colorCount(numColors,0);
    for(int i=0; i<code1.size(); i++){
        colorCount[code1[i]] += 1;
    }
    for(int i=0; i<code1.size(); i++){
        /*if(code1[i] >= numColors || code2[i] >= numColors){
            output.correctColors = 0;
            output.correctSpot = 0;
            return(output);
        }*/
        if(colorCount[code2[i]] > 0){
            colorCount[code2[i]] += -1;
            output.correctColors += 1;
        }
        output.correctSpot += 1*(code1[i] == code2[i]);
    }
    return(output);
}

vector<vector<int>> filterCombos(vector<vector<int>>& currentCombos, vector<int>& guess, comparisonOutput& output){
    vector<vector<int>> newCombos;
    for(vector<int>& combo: currentCombos){
        if(compare(combo, guess) == output){
            newCombos.push_back(combo);
        }
    }
    return(newCombos);
}

vector<comparisonOutput> getAllPossibleOutputs(int size){
    vector<comparisonOutput> o;
    o.reserve(size*(size+1)/2);
    for(int a=0; a<size+1; a++){
        for(int b=0; b<a+1; b++){
            o.push_back(comparisonOutput(a,b));
        }
    }
    return(o);
}

double bits(double probability){
    return(-log2(probability));
}

double rtnm(double num, double multiple){
    return(round(num/multiple)*multiple);
}

/*double calculateEntropy(vector<vector<int>>& currentCombos, vector<int> guess, int depth=1){
    int originalSize = currentCombos.size();
    double totalEntropy = 0;
    for(comparisonOutput& out: possibleOutputs){
        vector<vector<int>> guessList = {guess};
        vector<comparisonOutput> outList = {out};
        vector<vector<int>> filteredCombos = filterCombos(currentCombos,guessList,outList);
        double p = filteredCombos.size()/(double)currentCombos.size();
        double info = bits(p);
        
        if(p != 0){
            totalEntropy += p*info;
            if(depth>1){
                totalEntropy += p*calculateBestGuesses(allCombos,currentCombos,depth-1)[0].entropy;
            }
        }
    }
    return(totalEntropy);
}*/

struct guessObject{
    vector<int> guess;
    //double totalEntropy = 0;
    double probability = 0;
    vector<double> entropyList = {};
    
    guessObject(){}
};

vector<guessObject> sortGuesses(vector<guessObject>& list, int num, int key){
    int size = list.size();
    vector<guessObject> copy = list;
    for(int i = 0; i<key; i++){
        copy=sortGuesses(copy,num,key-1);
    }
    vector<guessObject> sortedList;
    sortedList.reserve(size);
    for(int i=0; i<num; i++){
        int bestIndex=0;
        for(int a=0; a<copy.size(); a++){
            if(copy[a].entropyList[key]>copy[bestIndex].entropyList[key]){
                bestIndex = a;
            }
            if(copy[a].entropyList[key]==copy[bestIndex].entropyList[key] && copy[a].probability>copy[bestIndex].probability){
                bestIndex = a;
            }
        }
        sortedList.push_back(copy[bestIndex]);
        copy.erase(copy.begin()+bestIndex);
    }
    return(sortedList);
}

vector<guessObject> calculateBestGuesses(vector<vector<int>>& allCombonations, vector<vector<int>>& currentCombos, vector<comparisonOutput>& allOutputs, int depth=1, int recusion=0){
    vector<guessObject> guessList;
    guessList.reserve(allCombonations.size());
    for(vector<int>& c: allCombonations){
        //double e = calculateEntropy(currentCombos, c, depth);

        double entropy = 0;
        double totalEntropy = 0;
        vector<double> entropyList(depth-1, 0.0);
        int originalSize = currentCombos.size();

        for(comparisonOutput& out: allOutputs){
                profiler.updateProfiler("Filter combos");
            vector<vector<int>> filteredCombos = filterCombos(currentCombos,c,out);
                profiler.updateProfiler("Filter combos");
            double p = filteredCombos.size()/(double)currentCombos.size();
            double info = bits(p);
            
            if(p != 0){
                totalEntropy += p*info;
                entropy += p*info;
                if(depth>1 && entropy != 0){
                    guessObject bestNextGuess = calculateBestGuesses(allCombos,filteredCombos,allOutputs,depth-1,recusion+1)[0];
                    //totalEntropy += p*bestNextGuess.totalEntropy;
                    for(int i=0; i<depth-1; i++){
                        entropyList[i] += p*(-bits(currentCombos.size())+(bits(filteredCombos.size())+bestNextGuess.entropyList[i]));
                    }
                }
            }
        }

        guessObject g;
        g.guess = c;
        g.entropyList.push_back(entropy);
        //g.totalEntropy = totalEntropy;

        g.entropyList.insert(g.entropyList.end(),entropyList.begin(),entropyList.end());
        g.probability = 1.0/(currentCombos.size())*count(currentCombos.begin(),currentCombos.end(),c);
        guessList.push_back(g);
    }

        profiler.updateProfiler("Sort guesses");
    if(recusion == 0){
        guessList = sortGuesses(guessList, guessList.size(),guessList[0].entropyList.size()-1);
    }else{
        guessList = sortGuesses(guessList, 1,guessList[0].entropyList.size()-1);
    }
        profiler.updateProfiler("Sort guesses");
    return(guessList);
}

void printCombos(vector<vector<int>>& combos){
    for(vector<int>& c: combos){
        for(int i: c){
            cout << i<< " ";
        }
        cout << "\n";
    }
    cout << "\n";
}

vector<int> getGuess(){
    vector<int> g;
    string input;
    cin >> input;
    for(char c: input){
        if(!isspace(c)){
            g.push_back(c-'0');
        }
    }
    return(g);
}

int main(){
    colors = {"red","yellow","blue"};
    numColors = colors.size();
    numColors = 3;
    codeLength = 3;
    int depth = 3;

    possibleOutputs = getAllPossibleOutputs(codeLength);

    code = randomCode(numColors, codeLength);

    allCombos = combonations::getCombonations(numColors, codeLength);
    vector<vector<int>> possibleCombos = allCombos;

    vector<int> guess;

    for(int i: code){
        cout << i;
    }
    cout << "\n\n";

    while(true){
        //printCombos(possibleCombos);
        cout << "Uncertainty: " << (-bits(possibleCombos.size())) << "\n";
        getch();
        cout << "Calculating...\n";
        
        profiler.start();
        vector<guessObject> bestGuesses = calculateBestGuesses(allCombos, possibleCombos, possibleOutputs, depth);
        profiler.end();
        profiler.logProfilerData();
        
        cout << "Guess\tProbability\t";
        for(int i=0; i<depth; i++){
            cout << "E " << i+1 << "      \t";
        }cout << "\n";
        for(guessObject g: bestGuesses){
            for(int i: g.guess){
                cout << i;
            }cout << " \t";
            cout << (g.probability);
            for(double e: g.entropyList){
                cout << " \t" << (e);
            }cout << "\n";
        }
        cout <<"\n";
        
        guess = getGuess();
        //double e = bestGuesses[0].entropyList[0];
        guesses.push_back(guess);
        comparisonOutput output = compare(code, guess);
        outputs.push_back(output);
        cout << output.correctColors << " " << output.correctSpot << "\n";

        int previousLength = possibleCombos.size();
        possibleCombos = filterCombos(possibleCombos,guess,output);

        cout << "Expected info: " << bestGuesses[0].entropyList[0] << " Actual info: " << bits(possibleCombos.size()/(double)previousLength) << "\n";
    }

    return(0);
}