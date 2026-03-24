#ifndef CRICKET_ANALYZER_H
#define CRICKET_ANALYZER_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

// Event Types
enum EventType {
    NO_EVENT = 0,
    FOUR = 1,
    SIX = 2,
    WICKET = 3,
    CROWD_CHEER = 4
};

// Structure to store detected events
struct CricketEvent {
    EventType type;
    long long timestamp;
    float confidence;
    string description;
    
    CricketEvent() {
        type = NO_EVENT;
        timestamp = 0;
        confidence = 0;
        description = "";
    }
};

// Main Analyzer Class
class CricketAnalyzer {
private:
    // Video properties
    int frameWidth;
    int frameHeight;
    double fps;
    
    // Previous frames for motion detection
    Mat previousFrame;
    Mat previousGray;
    
    // Detection thresholds
    int motionThreshold;
    int audioThreshold;
    
    // Store detected events
    vector<CricketEvent> events;
    
    // Helper methods
    double calculateMotion(const Mat& currentFrame);
    double calculateAudioEnergy(const vector<short>& audioData);
    bool isBoundaryRegion(const Mat& frame);
    bool isWicketRegion(const Mat& frame);
    void drawRegions(Mat& frame);
    
public:
    // Constructor and Destructor
    CricketAnalyzer();
    ~CricketAnalyzer();
    
    // Initialize with video properties
    bool initialize(int width, int height, double videoFps);
    
    // Process video frame and audio
    CricketEvent processFrame(const Mat& frame, const vector<short>& audioData);
    
    // Set detection sensitivity
    void setThresholds(int motion, int audio);
    
    // Save events to JSON file
    void saveEvents(const string& filename);
    
    // Generate highlight video
    void generateHighlights(const string& inputVideo, const string& outputVideo);
    
    // Get all detected events
    vector<CricketEvent> getEvents() { return events; }
};
#endif