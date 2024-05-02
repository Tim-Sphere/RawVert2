#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include "exiv2/exiv2.hpp"

using namespace std;

struct Segment {
    uint16_t marker;
    uint16_t length;
    vector<uint8_t> data;
};

vector<Segment> parseJPEGSegments(const string& inputFile) {
    vector<Segment> segments;

    ifstream inFile(inputFile, ios::binary);
    if (!inFile) {
        cerr << "Error: Unable to open input file " << inputFile << endl;
        return segments;
    }

    constexpr uint8_t SOImarker[] = { 0xFF, 0xD8, 0xFF, 0xE1 }; // Start of JPEG marker
    constexpr uint8_t endMarker[] = { 0xFF, 0xD9, 0x00, 0x80 };
    
    uint16_t marker;
    uint16_t length;

    while (inFile.read(reinterpret_cast<char*>(&marker), sizeof(marker))){
        if (marker == 0xFFD8){
            cout << "Found SOI Marker" << endl;
            break;
        }
    }
    
    while (inFile.read(reinterpret_cast<char*>(&marker), sizeof(marker))) {
        
        /*
        if (marker != 0xFF) {
            cerr << "Error: Invalid marker: " << marker << endl;
            return segments;
        }
        */
        if (marker == 0xFFD9){
            cout << "Found EOI Marker";
            return segments;
        }

        //inFile.read(reinterpret_cast<char*>(&marker), sizeof(marker)); // Read marker again
        inFile.read(reinterpret_cast<char*>(&length), sizeof(length));

        // Check for invalid segment length
        if (length < 2) {
            cerr << "Error: Invalid segment length" << endl;
            return segments;
        }

        Segment seg;
        seg.marker = marker;
        seg.length = length;

        // Read segment data
        seg.data.resize(length - 2); // Subtract 2 for length bytes
        inFile.read(reinterpret_cast<char*>(seg.data.data()), seg.length - 2);
        cout << "Marker: " << marker << " ; Length: " << length << endl;
        segments.push_back(seg);
    }

    return segments;
}

void writeJPEGSegments(const vector<Segment>& segments, const string& outputFile) {
    ofstream outFile(outputFile, ios::binary);
    if (!outFile) {
        cerr << "Error: Unable to open output file " << outputFile << endl;
        return;
    }
    
    uint16_t eoiMarker = 0xFFD8;
    outFile.write(reinterpret_cast<const char*>(&eoiMarker), sizeof(eoiMarker));
    
    // Write JPEG segment headers and data
    for (const auto& seg : segments) {
        // Write segment header
        outFile.write(reinterpret_cast<const char*>(&seg.marker), sizeof(seg.marker));
        outFile.write(reinterpret_cast<const char*>(&seg.length), sizeof(seg.length));

        // Write segment data
        outFile.write(reinterpret_cast<const char*>(seg.data.data()), seg.data.size());
    }
    eoiMarker = 0xFFD9;
    outFile.write(reinterpret_cast<const char*>(&eoiMarker), sizeof(eoiMarker));
    cout << "JPEG segments copied to " << outputFile << endl;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <input.jpg> <output.jpg>" << endl;
        return 1;
    }

    const string inputFile = argv[1];
    const string outputFile = argv[2];

    vector<Segment> segments = parseJPEGSegments(inputFile);
    if (segments.empty()) {
        cerr << "Failed to parse JPEG segments." << endl;
        return 1;
    }

    writeJPEGSegments(segments, outputFile);

    return 0;
}
