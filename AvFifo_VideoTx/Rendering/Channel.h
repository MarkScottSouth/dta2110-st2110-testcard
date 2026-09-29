#pragma once

#include <string>

struct Channel
{
    std::string label = "Unnamed";
    TestPattern pattern = TestPattern::SMPTEBars;
    
    std::string dstIp = "239.8.20.100";
    int dstPort = 50020;
}; 
