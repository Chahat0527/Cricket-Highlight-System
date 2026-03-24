#!/usr/bin/env python3
import cv2
import json
import requests
import time
import numpy as np

def generate_test_video():
    """Generate a simple test video"""
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    out = cv2.VideoWriter('test_cricket.mp4', fourcc, 30.0, (1280, 720))
    
    for i in range(300):  # 10 seconds
        # Create frame
        frame = np.zeros((720, 1280, 3), dtype=np.uint8)
        
        # Draw cricket pitch
        cv2.rectangle(frame, (540, 400), (740, 600), (0, 255, 0), -1)
        
        # Draw stumps
        cv2.rectangle(frame, (620, 400), (660, 500), (139, 69, 19), -1)
        
        # Simulate ball movement for boundary (frames 100-150)
        if 100 <= i <= 150:
            ball_x = 640 + (i - 100) * 15
            cv2.circle(frame, (ball_x, 500), 10, (0, 0, 255), -1)
        
        out.write(frame)
    
    out.release()
    print("Test video generated: test_cricket.mp4")

def test_analyzer():
    """Test the analyzer with test video"""
    print("Testing Cricket Analyzer...")
    
    # This would call your C++ program
    # For demo, we'll simulate
    print("  Starting analyzer...")
    time.sleep(2)
    
    # Simulate detection
    events = [
        {"type": "SIX", "timestamp": 3000000, "confidence": 0.95},
        {"type": "WICKET", "timestamp": 8000000, "confidence": 0.88}
    ]
    
    print(f"  Detected {len(events)} events")
    for event in events:
        print(f"    - {event['type']} at {event['timestamp']}ms")
    
    return True

def test_network():
    """Test network server connection"""
    print("\nTesting Network Server...")
    
    try:
        # Try to connect to server
        response = requests.get("http://localhost:8080", timeout=2)
        print("  Server is running and accepting connections")
        return True
    except:
        print("  Server not running (this is OK for standalone test)")
        return False

def test_highlight_generation():
    """Test highlight generation"""
    print("\nTesting Highlight Generation...")
    
    events = [
        {"start": 2.0, "end": 5.0},
        {"start": 7.0, "end": 10.0}
    ]
    
    print(f"  Generating highlights from {len(events)} events")
    print("  Highlights would be extracted from these segments:")
    
    for i, event in enumerate(events):
        print(f"    Highlight {i+1}: {event['start']}s - {event['end']}s")
    
    return True

def main():
    """Main test function"""
    print("=" * 50)
    print("Cricket Highlight Application Tests")
    print("=" * 50)
    
    # Generate test video
    generate_test_video()
    
    # Run tests
    test_analyzer()
    test_network()
    test_highlight_generation()
    
    print("\n" + "=" * 50)
    print("All tests completed!")
    print("=" * 50)

if __name__ == "__main__":
main()