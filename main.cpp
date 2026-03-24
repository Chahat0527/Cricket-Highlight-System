#include "CricketAnalyzer.h"
#include "NetworkServer.h"
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, char* argv[]) {
    cout << "========================================" << endl;
    cout << "   Cricket Highlight Generator" << endl;
    cout << "========================================" << endl;
    
    // Default file paths
    string videoFile = "cricket_match.mp4";
    string outputFile = "highlights.mp4";
    string eventsFile = "events.json";
    
    // Parse command line arguments
    if (argc > 1) videoFile = argv[1];
    if (argc > 2) outputFile = argv[2];
    
    // Open video file
    VideoCapture cap(videoFile);
    if (!cap.isOpened()) {
        cout << "Error: Cannot open video file: " << videoFile << endl;
        cout << "Usage: " << argv[0] << " [input_video] [output_video]" << endl;
        return 1;
    }
    
    // Get video properties
    int width = cap.get(CAP_PROP_FRAME_WIDTH);
    int height = cap.get(CAP_PROP_FRAME_HEIGHT);
    double fps = cap.get(CAP_PROP_FPS);
    int totalFrames = cap.get(CAP_PROP_FRAME_COUNT);
    
    cout << "\nVideo Information:" << endl;
    cout << "  Resolution: " << width << " x " << height << endl;
    cout << "  FPS: " << fps << endl;
    cout << "  Duration: " << (totalFrames / fps) << " seconds" << endl;
    
    // Initialize analyzer
    CricketAnalyzer analyzer;
    analyzer.initialize(width, height, fps);
    
    // Set detection thresholds
    analyzer.setThresholds(30, 1500);
    
    // Start network server
    NetworkServer server(&analyzer);
    server.start(8080);
    
    cout << "\nProcessing video..." << endl;
    cout << "Server running on port 8080" << endl;
    
    // Process video frames
    Mat frame;
    int frameCount = 0;
    vector<short> dummyAudio; // In real app, this would come from audio file
    
    while (true) {
        cap >> frame;
        if (frame.empty()) break;
        
        // Create dummy audio data (replace with actual audio in real implementation)
        dummyAudio.clear();
        for (int i = 0; i < 1000; i++) {
            dummyAudio.push_back(rand() % 1000);
        }
        
        // Process frame
        CricketEvent event = analyzer.processFrame(frame, dummyAudio);
        
        // Broadcast event if detected
        if (event.type != NO_EVENT) {
            server.broadcastEvent(event);
        }
        
        frameCount++;
        // Show progress
        if (frameCount % 100 == 0) {
            cout << "  Processed: " << frameCount << "/" << totalFrames 
                 << " frames (" << (frameCount * 100 / totalFrames) << "%)" << endl;
        }
    }
    
    cout << "\nProcessing complete!" << endl;
    cout << "Total frames processed: " << frameCount << endl;
    cout << "Events detected: " << analyzer.getEvents().size() << endl;
    
    // Save events to file
    analyzer.saveEvents(eventsFile);
    
    // Generate highlights
    if (analyzer.getEvents().size() > 0) {
        analyzer.generateHighlights(videoFile, outputFile);
    } else {
        cout << "No events detected. Try adjusting thresholds." << endl;
    }
    
    // Cleanup
    cap.release();
    server.stop();
    
    cout << "\nDone!" << endl;
    cout << "Events saved to: " << eventsFile << endl;
    if (analyzer.getEvents().size() > 0) {
        cout << "Highlights saved to: " << outputFile << endl;
    }
    return 0;
}