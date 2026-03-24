#include "CricketAnalyzer.h"
CricketAnalyzer::CricketAnalyzer() {
    // Default values
    frameWidth = 0;
    frameHeight = 0;
    fps = 30.0;
    motionThreshold = 50;
    audioThreshold = 2000;
}

CricketAnalyzer::~CricketAnalyzer() {
    // Cleanup
    previousFrame.release();
    previousGray.release();
}

bool CricketAnalyzer::initialize(int width, int height, double videoFps) {
    frameWidth = width;
    frameHeight = height;
    fps = videoFps;
    
    cout << "Analyzer initialized: " << frameWidth << "x" << frameHeight 
         << " at " << fps << " fps" << endl;
    
    return true;
}

void CricketAnalyzer::setThresholds(int motion, int audio) {
    motionThreshold = motion;
    audioThreshold = audio;
    cout << "Thresholds set - Motion: " << motionThreshold 
         << ", Audio: " << audioThreshold << endl;
}

double CricketAnalyzer::calculateMotion(const Mat& currentFrame) {
    // Convert to grayscale
    Mat currentGray;
    cvtColor(currentFrame, currentGray, COLOR_BGR2GRAY);
    
    if (previousGray.empty()) {
        previousGray = currentGray.clone();
        return 0;
    }
    
    // Calculate absolute difference between frames
    Mat diff;
    absdiff(previousGray, currentGray, diff);
    
    // Calculate average motion
    Scalar meanDiff = mean(diff);
    double motion = meanDiff[0];
    
    // Store for next frame
    previousGray = currentGray.clone();
    
    return motion;
}

double CricketAnalyzer::calculateAudioEnergy(const vector<short>& audioData) {
    double energy = 0;
    for (short sample : audioData) {
        energy += abs(sample);
    }
    return energy / audioData.size();
}

bool CricketAnalyzer::isBoundaryRegion(const Mat& frame) {
    // Define boundary regions (these are approximate positions)
    // Adjust based on camera angle
    
    // Region 1: Long on area (bottom left)
    Rect boundary1(100, frame.rows - 200, 300, 150);
    
    // Region 2: Long off area (bottom right)
    Rect boundary2(frame.cols - 400, frame.rows - 200, 300, 150);
    
    // Region 3: Deep mid-wicket (top center)
    Rect boundary3(frame.cols/2 - 200, 100, 400, 150);
    
    // Check if there's motion in boundary regions
    double motion1 = 0, motion2 = 0, motion3 = 0;
    
    if (boundary1.x + boundary1.width <= frame.cols &&
        boundary1.y + boundary1.height <= frame.rows) {
        Mat roi1 = frame(boundary1);
        motion1 = calculateMotion(roi1);
    }
    
    if (boundary2.x + boundary2.width <= frame.cols &&
        boundary2.y + boundary2.height <= frame.rows) {
        Mat roi2 = frame(boundary2);
        motion2 = calculateMotion(roi2);
    }
    
    if (boundary3.x + boundary3.width <= frame.cols &&
        boundary3.y + boundary3.height <= frame.rows) {
        Mat roi3 = frame(boundary3);
        motion3 = calculateMotion(roi3);
    }
    
    // Return true if significant motion in any boundary region
    return (motion1 > motionThreshold || motion2 > motionThreshold || 
            motion3 > motionThreshold);
}

bool CricketAnalyzer::isWicketRegion(const Mat& frame) {
    // Define wicket region (stumps area)
    int wicketX = frame.cols/2 - 100;
    int wicketY = frame.rows - 300;
    Rect wicketRegion(wicketX, wicketY, 200, 150);
    
    if (wicketRegion.x + wicketRegion.width <= frame.cols &&
        wicketRegion.y + wicketRegion.height <= frame.rows) {
        Mat roi = frame(wicketRegion);
        double motion = calculateMotion(roi);
        
        // Sudden motion in wicket area indicates wicket
        return (motion > motionThreshold * 1.5);
    }
    
    return false;
}

void CricketAnalyzer::drawRegions(Mat& frame) {
    // Draw boundary regions for debugging
    Rect boundary1(100, frame.rows - 200, 300, 150);
    Rect boundary2(frame.cols - 400, frame.rows - 200, 300, 150);
    Rect boundary3(frame.cols/2 - 200, 100, 400, 150);
    Rect wicketRegion(frame.cols/2 - 100, frame.rows - 300, 200, 150);
    
    rectangle(frame, boundary1, Scalar(0, 255, 0), 2);
    rectangle(frame, boundary2, Scalar(0, 255, 0), 2);
    rectangle(frame, boundary3, Scalar(0, 255, 0), 2);
    rectangle(frame, wicketRegion, Scalar(0, 0, 255), 2);
}

CricketEvent CricketAnalyzer::processFrame(const Mat& frame, const vector<short>& audioData) {
    CricketEvent event;
    
    if (frame.empty()) {
        return event;
    }
    
    // Calculate overall motion
    double motion = calculateMotion(frame);
    
    // Calculate audio energy
    double audioEnergy = calculateAudioEnergy(audioData);
    
    // Check for boundary (Four or Six)
    if (isBoundaryRegion(frame)) {
        event.type = (motion > motionThreshold * 2) ? SIX : FOUR;
        event.timestamp = (long long)(events.size() * 1000000 / fps);
        event.confidence = min(1.0f, (float)(motion / 100.0f));
        
        if (event.type == SIX) {
            event.description = "SIX! Ball cleared the boundary!";
        } else {
            event.description = "FOUR! Ball reaches the boundary!";
        }
        
        events.push_back(event);
        cout << "[EVENT] " << event.description << " (Confidence: " 
             << event.confidence << ")" << endl;
    }
    
    // Check for wicket
    else if (isWicketRegion(frame) && audioEnergy > audioThreshold) {
        event.type = WICKET;
        event.timestamp = (long long)(events.size() * 1000000 / fps);
        event.confidence = min(1.0f, (float)(audioEnergy / 5000.0f));
        event.description = "WICKET! The batsman is out!";
        
        events.push_back(event);
        cout << "[EVENT] " << event.description << " (Confidence: " 
             << event.confidence << ")" << endl;
    }
    
    // Check for crowd cheer
    else if (audioEnergy > audioThreshold * 1.5) {
        event.type = CROWD_CHEER;
        event.timestamp = (long long)(events.size() * 1000000 / fps);
        event.confidence = min(1.0f, (float)(audioEnergy / 3000.0f));
        event.description = "CROWD CHEER! Exciting moment!";
        
        events.push_back(event);
        cout << "[EVENT] " << event.description << " (Confidence: " 
             << event.confidence << ")" << endl;
    }
    return event;
}

void CricketAnalyzer::saveEvents(const string& filename) {
    ofstream file(filename);
    
    file << "{" << endl;
    file << "  \"events\": [" << endl;
    for (size_t i = 0; i < events.size(); i++) {
        string typeName;
        switch(events[i].type) {
            case FOUR: typeName = "FOUR"; break;
            case SIX: typeName = "SIX"; break;
            case WICKET: typeName = "WICKET"; break;
            case CROWD_CHEER: typeName = "CROWD_CHEER"; break;
            default: typeName = "UNKNOWN";
        }
        
        file << "    {" << endl;
        file << "      \"type\": \"" << typeName << "\"," << endl;
        file << "      \"timestamp\": " << events[i].timestamp << "," << endl;
        file << "      \"confidence\": " << events[i].confidence << "," << endl;
        file << "      \"description\": \"" << events[i].description << "\"" << endl;
        
        if (i < events.size() - 1) {
            file << "    }," << endl;
        } else {
            file << "    }" << endl;
        }
    }
    file << "  ]" << endl;
    file << "}" << endl;
    
    file.close();
    cout << "Saved " << events.size() << " events to " << filename << endl;
}

void CricketAnalyzer::generateHighlights(const string& inputVideo, const string& outputVideo) {
    if (events.empty()) {
        cout << "No events found. Cannot generate highlights." << endl;
        return;
    }
    
    VideoCapture cap(inputVideo);
    if (!cap.isOpened()) {
        cout << "Error: Cannot open video file" << endl;
        return;
    }
    
    // Get video properties
    int width = cap.get(CAP_PROP_FRAME_WIDTH);
    int height = cap.get(CAP_PROP_FRAME_HEIGHT);
    double videoFps = cap.get(CAP_PROP_FPS);
    int totalFrames = cap.get(CAP_PROP_FRAME_COUNT);
    
    // Create video writer
    VideoWriter writer(outputVideo, 
                       VideoWriter::fourcc('m', 'p', '4', 'v'),
                       videoFps, 
                       Size(width, height));
    
    if (!writer.isOpened()) {
        cout << "Error: Cannot create output video" << endl;
        return;
    }
    
cout << "Generating highlights..." << endl;

    // For each event, extract 8 seconds of video
    int highlightDuration = 8 * videoFps; // 8 seconds in frames
    for (size_t i = 0; i < events.size(); i++) {
        // Calculate frame number (assuming 30 fps and timestamp in microseconds)
        int eventFrame = (events[i].timestamp * 30) / 1000000;
        int startFrame = max(0, eventFrame - 30); // Start 1 second before
        int endFrame = min(totalFrames - 1, startFrame + highlightDuration);
        
        cout << "  Extracting highlight " << (i+1) << " of " << events.size() 
             << " (frames " << startFrame << "-" << endFrame << ")" << endl;
        
        // Seek to start frame
        cap.set(CAP_PROP_POS_FRAMES, startFrame);
        
        // Write frames
        for (int f = startFrame; f <= endFrame; f++) {
            Mat frame;
            cap >> frame;
            if (frame.empty()) break;
            writer.write(frame);
        }
    }
    cap.release();
    writer.release();
    
    cout << "Highlights saved to: " << outputVideo << endl;
}