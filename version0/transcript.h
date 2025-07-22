#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>

struct TranscriptEntry {
    double startTime;
    std::string text;  // ✅ use std::string
};

class Transcript {
    std::vector<TranscriptEntry> entries;
    int index = 0;
public:
    void addEntry(double startTime, const std::string& text) {
        entries.push_back({startTime, text});
    }
    TranscriptEntry getCurrentIndex() const {
        if (index < entries.size()) {
            return entries[index];
        } else {
            return {0,""};
        }
    }
    void moveIndex() {
        if (moreIndex()) {
            index++;
        }
    }
    bool moreIndex() {
        return index < entries.size();
    }
};

// ✅ Updated to take std::string and parse it
double parseTimestamp(const std::string& timestamp) {
    int hours, minutes, seconds, milliseconds;
    char colon1, colon2, dot;
    std::istringstream iss(timestamp);
    iss >> hours >> colon1 >> minutes >> colon2 >> seconds >> dot >> milliseconds;
    return hours * 3600 + minutes * 60 + seconds + milliseconds / 1000.0;
}

Transcript parseTranscriptFile(const char* fileName) {
    Transcript transcript;
    std::ifstream file(fileName);
    std::string line;

    std::regex lineRegex(R"(\[(\d{2}:\d{2}:\d{2}\.\d{3}) --> \d{2}:\d{2}:\d{2}\.\d{3}\]\s*(.*))");
    // std::regex lineRegex(R"(\[\d{2}:\d{2}:\d{2}\.\d{3} --> (\d{2}:\d{2}:\d{2}\.\d{3})\]\s*(.*))");


    while (std::getline(file, line)) {
        std::smatch match;
        if (std::regex_match(line, match, lineRegex)) {
            std::string startTimeStr = match[1];
            std::string text = match[2];
            double startTime = parseTimestamp(startTimeStr);
            transcript.addEntry(startTime * 1000, text);
        }
    }

    return transcript;
}