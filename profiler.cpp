#include <cmath>
#include <vector>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <map>
#include <string>

namespace Profiler{

    const double NANO_TO_SEC = 1.0/1000000000;
    double getTime(){
        //returns time in seconds
        return(std::chrono::duration_cast<std::chrono::nanoseconds>((std::chrono::system_clock::now()).time_since_epoch()).count()*NANO_TO_SEC);
    }

    class functionProfile{
        public:
        std::string functionName = "";
        double startTime = 0;
        int count = 0;
        double totalTime = 0;
        int depth = 0;

        functionProfile(){}

        functionProfile(const std::string& name){
            this->functionName = name;
        }

        int update(int globalDepth, double time){
            if(this->startTime == 0){
                this->depth = globalDepth;
                this->startTime = getTime();
                return(1); //started
            }else{
                this->totalTime += time-this->startTime;
                this->count = this->count + 1;
                this->startTime = 0;
                return(-1); //ended
            }
        }
    };

    class Profiler{
        public:
        std::map<std::string, functionProfile> profileMap;
        std::vector<std::string> functionList;
        int profilerDepth = 0;
        int currentIndex;
        functionProfile profilerUpdater;
        std::string logDirectory = "";
        double startTime;
        double endTime;

        Profiler(){
            this->start();
        }

        void log(const std::string& message){
            std::ofstream logFile;
            if(this->logDirectory==""){
                logFile.open("log.txt",std::ios::app);
            }else{
                logFile.open(this->logDirectory + "\\log.txt",std::ios::app);
            }
            logFile << std::fixed << std::setprecision(9);
            logFile << message << "\n";
            logFile.close();
        }


        void updateProfiler(const std::string& functionName){
            double t = getTime();
            if(profilerDepth == 0){
                this->currentIndex = this->functionList.size();
            }else{
                this->currentIndex++;
            }

            try{
                this->profilerDepth += this->profileMap.at(functionName).update(this->profilerDepth,t);
                this->currentIndex = find(this->functionList.begin(),this->functionList.end(),functionName)-this->functionList.begin();
            }catch(...){
                this->functionList.insert(this->functionList.begin()+this->currentIndex,functionName);
                this->profileMap[functionName] = functionProfile(functionName);
                this->profilerDepth += this->profileMap.at(functionName).update(this->profilerDepth,t);
            }
            this->profilerUpdater.count++;
            this->profilerUpdater.totalTime += getTime()-t;
        }

        void start(){
            this->startTime = getTime();
            this->profilerUpdater = functionProfile("Profiler");
            this->profileMap = std::map<std::string, functionProfile>();
        }
        void end(){
            this->endTime = getTime();
        }

        void logProfilerData(){
            double totalRunTime = (this->endTime)-(this->startTime);
            double timeSum = this->profilerUpdater.totalTime;
            log("Total Run time: " + std::to_string(totalRunTime) + "s");
            if(profileMap.size() > 0){
                log("Profiler Data: Depth, Average time, Count, Total time, Percent");
                log(
                    this->profilerUpdater.functionName + ": " + 
                    std::to_string((this->profilerUpdater.totalTime/this->profilerUpdater.count)*1000) + "ms, " + 
                    std::to_string(this->profilerUpdater.count) + ", " + 
                    std::to_string(this->profilerUpdater.totalTime) + "s, " +
                    std::to_string(int(round((this->profilerUpdater.totalTime/totalRunTime)*100))) + "%"
                );
                for(std::string& s: this->functionList){
                    functionProfile& f = this->profileMap.at(s);
                    std::string indent;
                    for(int i = 0; i < f.depth; i++){
                        indent.append("     ");
                    }
                    log(
                        indent +
                        f.functionName + ": " + 
                        std::to_string((f.totalTime/f.count)*1000) + "ms, " + 
                        std::to_string(f.count) + ", " + 
                        std::to_string(f.totalTime) + "s, " +
                        std::to_string(int(round((f.totalTime/totalRunTime)*100))) + "%"
                    );
                }
            }
            log("");
        }
    };
}