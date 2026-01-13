#pragma once
#include <string>
#include <vector>

struct Segment {
    int number;              // segment number (1051, 1052, ...)
    double duration;         // seconds
    std::string url;         // full URL
};

struct MPD {
    std::vector<Segment> segments;
};

class MPDParser {
public:
    static MPD parse(const std::string& mpdPath,
                     const std::string& serverBaseURL);
};
