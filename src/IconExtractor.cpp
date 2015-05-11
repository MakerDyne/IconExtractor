//============================================================================
// Name			: Icon Extractor (IconExtractor.cpp)
// Description 	: Reads a one-bit-per-pixel bitmap file containing multiple
//				: elements arranged in distinct rows and columns and copies
//				: each element to its own individual bitmap file.
//
// Notes		: Compatible with C++11 or later
//				: Tested with gcc 4.8.3 on Linux
//
// Author		: Richard Leszczynski
// Contact		: richard@makerdyne.com
//
// License		: Copyright (C) 2015 Richard Leszczynski
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//============================================================================

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include <utility>
#include <list>

#include <sys/types.h>
#include <sys/stat.h>

#include "ConsoleOutput.h"

using std::cout;
using std::cin;
using std::cerr;
using std::endl;

int main(int argc, char * argv[]) {
	// Variables to be set by command line args
	// Verbose output?
	bool verbose = false;
	// Make all icons the same size? (White padding added around the edges of the smaller ones to make their files dimensionally the same size as the largest icon)
	bool sameSizeIcons = false;
	// Add white margins to each icon or not, and what size margins? (The value provided for the margin will be added to each edge)
	bool addMargins = false;
	unsigned int horizontalMargin = 0;
	unsigned int verticalMargin = 0;
	// Input file (Must be a one-bit-per-pixel bitmap file)
	std::string inputFile;
	bool inputFileSpecified = false;
	// Output folder (Directory into which to place the icon files created by this program)
	std::string outputDir = "";
	bool outputDirSpecified = false;
	// Create object for formatted console error and information output
	ConsoleOutput bitmapInfo(78, '-');

	//--------------------------------------------------
	// Process and store command line parameters
	//--------------------------------------------------

	if(argc < 2) {
		bitmapInfo.printMessage(ConsoleOutput::ERR, "No command line arguments given. Nothing to do", "");
		bitmapInfo.printMessage(ConsoleOutput::ERR, "At a minimum, an input file is required: -i /path/to/iconarray.bmp", "");
		// TODO : add help text function here
		return false;
	}
	else {
		for(int i=1; i<argc; i++) {
			// Argument for specifying input filename
			if(std::string(argv[i]) == "-i") {
				if(i+1 == argc) {	// "-i" need an additional argument to hold a filename
					bitmapInfo.printMessage(ConsoleOutput::ERR, "Command line argument error: No input filename specified", "");
					return false;
				}
				else {
					inputFile = argv[++i];
					inputFileSpecified = true;
					struct stat pathInfo;
					int result = stat(inputFile.c_str(), &pathInfo);
					if(result != 0) {
						bitmapInfo.printMessage(ConsoleOutput::ERR, "Input file does not exist. File provided is", inputFile);
						return false;
					}
					else if( (pathInfo.st_mode & S_IFREG) != S_IFREG) {
						bitmapInfo.printMessage(ConsoleOutput::ERR, "Path provided for input file is not to a file. Path provided is", inputFile);
						return false;
					}
				}
			}
			// Argument for specifying output directory
			else if(std::string(argv[i]) == "-o") {
				if(i+1 == argc) {	// "-o" need an additional argument to hold a filename
					bitmapInfo.printMessage(ConsoleOutput::ERR, "Command line argument error: No output directory specified", "");
					bitmapInfo.printMessage(ConsoleOutput::ERR, "The output directory argument '-o' is optional but, if present, it must be followed by a valid local directory", "");
					return false;
				}
				else {
					outputDir = argv[++i];
					outputDirSpecified = true;
					struct stat pathInfo;
					if(stat(outputDir.c_str(), &pathInfo) != 0) {
						bitmapInfo.printMessage(ConsoleOutput::ERR, "Path for output directory does not exist. Path provided is", outputDir);
						return false;
					}
					else if( (pathInfo.st_mode & S_IFDIR) != S_IFDIR ) {
						bitmapInfo.printMessage(ConsoleOutput::ERR, "Path provided for output directory is not a directory. Path provided is", outputDir);
						return false;
					}
				}
			}
			// Argument for printing verbose output to console
			else if(std::string(argv[i]) == "-v") {
				verbose = true;
			}
			// Argument for constructing same size icons
			else if(std::string(argv[i]) == "--samesize") {
				sameSizeIcons = true;
			}
			// Argument for adding horizontal margin (extra pixels above and below each icon)
			else if(std::string(argv[i]) == "--hmargin") {
				std::istringstream argChecker(argv[++i]);
				if (!(argChecker >> horizontalMargin) || horizontalMargin > 1000) {
				    bitmapInfo.printMessage(ConsoleOutput::ERR, "Expected positive integer value for horizontal margin. Received", argChecker.str(), "instead");
				    return false;
				}
				addMargins = true;
			}
			// Argument for adding vertical margin (extra pixels left and right of each icon)
			else if(std::string(argv[i]) == "--vmargin") {
				std::istringstream argChecker(argv[++i]);
				if (!(argChecker >> verticalMargin) || verticalMargin > 1000) {
					bitmapInfo.printMessage(ConsoleOutput::ERR, "Expected positive integer value of less than 1000 pixels for vertical margin. Received", argChecker.str(), "instead");
					return false;
				}
				addMargins = true;
			}
			// Argument for printing help text
			else if(std::string(argv[i]) == "-h") {
				// TODO: Write help text, or execute function to print help text
				return true;
			}
			// Argument not recognised
			else {
				bitmapInfo.printMessage(ConsoleOutput::ERR, "Command line argument error: Invalid argument ", argv[i]);
				// TODO: Write help text, or execute function to print help text
				return false;
			}
		}
	}

	// Exit if no input file has been specified
	if(!inputFileSpecified) {
		bitmapInfo.printMessage(ConsoleOutput::ERR, "No input file specified.", "");
		// TODO call help text function here
		return false;
	}

	if(verbose) {
		bitmapInfo.printHeading("Icon Extractor");
	}

	if(verbose) {
		cout << endl;
		bitmapInfo.printHeading("Summary of command line arguments");
		bitmapInfo.printMessage(ConsoleOutput::INFO, "Input file is", inputFile);
		if(outputDirSpecified) {
			bitmapInfo.printMessage(ConsoleOutput::INFO, "Output directory is", outputDir);
		}
		else {
			bitmapInfo.printMessage(ConsoleOutput::INFO, "No output directory has been specified", "");
		}
		bitmapInfo.printMessage(ConsoleOutput::INFO, "Verbose output option is set to", ((verbose) ? "true" : "false") );
		bitmapInfo.printMessage(ConsoleOutput::INFO, "Add margins option is set to", ((addMargins) ? "true" : "false") );
		if(addMargins) {
			bitmapInfo.printMessage(ConsoleOutput::INFO, "Horizontal margin is set to", horizontalMargin, "pixels");
			bitmapInfo.printMessage(ConsoleOutput::INFO, "Vertical margin is set to", verticalMargin, "pixels");
		}
		bitmapInfo.printMessage(ConsoleOutput::INFO, "Option to pad out all icon files to the same dimensions is set to", ((sameSizeIcons) ? "true" : "false") );
	}

	if(verbose) {
		cout << endl;
		bitmapInfo.printHeading("Opening bitmap file");
	}


	// Open file specified in command line args
	std::ifstream bitmapFile;
	bitmapFile.open(inputFile, (std::ifstream::in | std::ifstream::binary));
	if(bitmapFile.fail()) {
		bitmapInfo.printMessage(ConsoleOutput::ERR, "Failed to open input file", inputFile);
		cout << endl;
		return false;
	}
	else {
		if(verbose) {
			bitmapInfo.printMessage(ConsoleOutput::INFO, "File", inputFile, "opened");
		}
	}

	// get bitmapFile size
	bitmapFile.seekg(0, bitmapFile.end);
	const unsigned int bitmapFileSize = bitmapFile.tellg();
	bitmapFile.seekg(0, bitmapFile.beg);

	if(verbose) {
		cout << endl;
		bitmapInfo.printHeading("Bitmap File Header Information:");
	}

	if(bitmapFileSize < 54) {
		bitmapInfo.printMessage(ConsoleOutput::ERR,	"Bitmap file is too small to contain minimum required file headers. File is", bitmapFileSize, "bytes");
		bitmapFile.close();
		return false;
	}

	//--------------------------------------------------
	// Processing bitmap file header
	//--------------------------------------------------
	// Extract useful information from the 14 byte Bitmap file Header
	// Position:00-01, Length:2, Info: File identifier bytes "BM" / 0x424D
	uint16_t bmpFileId = 0;
	bitmapFile.seekg(0);
	bitmapFile.read((char *)&bmpFileId, sizeof(uint16_t));
	// Position:02-05, Length:4, Info: File size in bytes, little endian format
	uint32_t bmpFileSize = 0;
	bitmapFile.seekg(2);
	bitmapFile.read((char *)&bmpFileSize, sizeof(uint32_t));
	// Position:10-13, Length:4, Info: Offset in file where bit map data begins, little endian format
	uint32_t bmpDataOffset = 0;
	bitmapFile.seekg(10);
	bitmapFile.read((char *)&bmpDataOffset, sizeof(uint32_t));

	// Sanity check parameters from Bitmap file header
	// Check file is in windows bitmap format. The first 2 bytes should be the ascii characters B and M (Bytes 0-1 = 0x42, 0x4D)
	// Here it is reversed for 0x4D42 as the value has been stored in little endian format
	if(bmpFileId == 0x4D42) {
		if(verbose) {
			bitmapInfo.printMessage(ConsoleOutput::INFO, "File", inputFile, "identified as Windows Bitmap format");
		}
	}
	else {
		bitmapInfo.printMessage(ConsoleOutput::ERR, "File", inputFile, "is not a Windows Bitmap file");
		std::stringstream ss;
		uint16_t bigEndianBmpId = 0x0000;
		bigEndianBmpId |= (bmpFileId >> 8) | (bmpFileId << 8);
		ss << '\'' << ((char)(bmpFileId & 0x00FF)) << ((char)(bmpFileId >> 8)) << "\' (0x" << std::hex << std::uppercase << bigEndianBmpId << ") instead";
		bitmapInfo.printMessage(ConsoleOutput::ERR,	"Expected identifier 'BM' (0x424D) as first two bytes of file, but got", ss.str());
		bitmapFile.close();
		return false;
	}

	// Print the filesize in bytes. Check for agreement between size recorded within the file and file.size() method
	if(bitmapFileSize == bmpFileSize) {
		if(verbose) {
			bitmapInfo.printMessage(ConsoleOutput::INFO, "Size returned by File.size() agrees with size declared within file", inputFile);
			bitmapInfo.printMessage(ConsoleOutput::INFO, "Size of the file is",	bmpFileSize, "bytes");
		}
	}
	else {
		bitmapInfo.printMessage(ConsoleOutput::ERR,	"Size returned by file.size() is different to size written within file.", "");
		bitmapInfo.printMessage(ConsoleOutput::ERR,	"Size returned by file.size() is", bitmapFileSize, "bytes");
		bitmapInfo.printMessage(ConsoleOutput::ERR,	"Size declared within file is   ", bmpFileSize, "bytes");
		bitmapFile.close();
		return false;
	}

	// Print the offset within the bitmap file at which the bit map data begins
	if(bitmapFileSize > bmpDataOffset) {
		if(verbose) {
			bitmapInfo.printMessage(ConsoleOutput::INFO, "The offset within the file at which the bit map data begins is", bmpDataOffset);
		}
	}
	else {
		bitmapInfo.printMessage(ConsoleOutput::ERR,	"The offset within the file at which the bitmap data begins is larger than the file size", "");
		bitmapInfo.printMessage(ConsoleOutput::ERR, "Bit map data offset is", bmpDataOffset, "bytes");
		bitmapInfo.printMessage(ConsoleOutput::ERR, "File size is",	bitmapFileSize, "bytes");
		return false;
	}

	//--------------------------------------------------
	// Processing DIB (bitmap information) header
	//--------------------------------------------------
	// Extract useful information from the 124 byte DIB bitmap information header
	// Position:14-17, Length:4, Info: DIB header length in bytes. 124 bytes for the BITMAPV5HEADER variant
	bitmapFile.seekg(14);
	uint32_t dibDibLength = 0;
	bitmapFile.read((char *)&dibDibLength, sizeof(uint32_t));
	// Position:18-21, Length:4, Info: Image width in pixels
	bitmapFile.seekg(18);
	uint32_t dibImageWidth = 0;
	bitmapFile.read((char *)&dibImageWidth, sizeof(uint32_t));
	// Position 22-25, Length:4, Info: Image height in pixels
	bitmapFile.seekg(22);
	uint32_t dibImageHeight = 0;
	bitmapFile.read((char *)&dibImageHeight, sizeof(uint32_t));
	// Position 26-27, Length:2, Info: Number of colour planes in image. MUST BE 1
	bitmapFile.seekg(26);
	uint16_t dibColourPlanes = 0;
	bitmapFile.read((char *)&dibColourPlanes, sizeof(uint16_t));
	// Position 28-29, Length:2, Info: Number of bits per pixel/home/richard/Documents/
	bitmapFile.seekg(28);
	uint16_t dibBitsPerPixel = 0;
	bitmapFile.read((char *)&dibBitsPerPixel, sizeof(uint16_t));
	// Position 30-33, Length:4, Info: Compression method NEED 1 (no compression) here
	bitmapFile.seekg(30);
	uint32_t dibCompression = 0;
	bitmapFile.read((char *)&dibCompression, sizeof(uint32_t));
	// Position 34-37, Length:4, Info: length of bit map data within the bitmap file
	bitmapFile.seekg(34);
	uint32_t dibLengthOfBitMapData = 0;
	bitmapFile.read((char *)&dibLengthOfBitMapData, sizeof(uint32_t));
	// Position 38-41, Length:4, Info: horizontal resolution, pixels per metre, irrelevant here
	bitmapFile.seekg(38);
	uint32_t dibHorizontalResolution = 0;
	bitmapFile.read((char *)&dibHorizontalResolution, sizeof(uint32_t));
	// Position 42-45, Length:4, Info: vertical resolution, pixels per metre, irrelevant here
	bitmapFile.seekg(42);
	uint32_t dibVerticalResolution = 0;
	bitmapFile.read((char *)&dibVerticalResolution, sizeof(uint32_t));
	// Position 46-49, Length:4, Info: number of colours in colour palette MUST BE 2
	bitmapFile.seekg(46);
	uint32_t dibColoursInPalette = 0;
	bitmapFile.read((char *)&dibColoursInPalette, sizeof(uint32_t));
	// Position 50-53, Length:4, Info: number of important colours. Not terribly important for 1 bit-per-pixel bitmaps!
	bitmapFile.seekg(50);
	uint32_t dibImportantColours = 0;
	bitmapFile.read((char *)&dibImportantColours, sizeof(uint32_t));

	// Print and sanity check the DIB header information
	if(verbose) {
		cout << endl;
		bitmapInfo.printHeading("DIB Header Information:");
		bitmapInfo.printMessage(ConsoleOutput::INFO, "Length of DIB header is",	dibDibLength, "bytes");
		bitmapInfo.printMessage(ConsoleOutput::INFO, "Image width is ",	dibImageWidth, "pixels");
		bitmapInfo.printMessage(ConsoleOutput::INFO, "Image height is",	dibImageHeight, "pixels");
	}
	if(dibColourPlanes == 1) {
		if(verbose) {
			bitmapInfo.printMessage(ConsoleOutput::INFO, "Number of colour planes is", dibColourPlanes);
		}
	}
	else {
		bitmapInfo.printMessage(ConsoleOutput::ERR,	"Number of colour planes must be 1. Instead there are",	dibColourPlanes, "colour planes");
		bitmapFile.close();
		return false;
	}
	if(dibBitsPerPixel == 1) {
		if(verbose) {
			bitmapInfo.printMessage(ConsoleOutput::INFO, "Number of bits per pixel is", dibBitsPerPixel);
		}
	}
	else {
		bitmapInfo.printMessage(ConsoleOutput::ERR,	"Number of bits per pixel must be 1. Instead there are", dibBitsPerPixel, "bits per pixel");
		bitmapFile.close();
		return false;
	}
	if(dibCompression == 0) {
		if(verbose) {
			bitmapInfo.printMessage(ConsoleOutput::INFO, "Image data is not compressed", "");
		}
	}
	else {
		bitmapInfo.printMessage(ConsoleOutput::ERR,	"Image data is compressed. It must not be", "");
		switch (dibCompression) {
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 11:
		case 12:
		case 13:
			bitmapInfo.printMessage(ConsoleOutput::ERR, "Compression method is", dibCompression);
			bitmapFile.close();
			return false;
		default:
			bitmapInfo.printMessage(ConsoleOutput::ERR,	"Compression method is not even recognised. Value is", dibCompression);
			bitmapFile.close();
			return false;
		}
	}
	if(dibLengthOfBitMapData + bmpDataOffset > bitmapFileSize) {
		bitmapInfo.printMessage(ConsoleOutput::ERR,	"Length of bit map data is calculated to overshoot the end of the file", "");
		bitmapInfo.printMessage(ConsoleOutput::ERR, "File size is",	bitmapFileSize, "bytes");
		bitmapInfo.printMessage(ConsoleOutput::ERR, "Calculated end of bit map data is", dibLengthOfBitMapData + bmpDataOffset);
		bitmapFile.close();
		return false;
	}
	else {
		if(verbose) {
			bitmapInfo.printMessage(ConsoleOutput::INFO,
					"Length of bit map data is", dibLengthOfBitMapData);
			bitmapInfo.printMessage(ConsoleOutput::INFO, "Number of bytes left in file after bit map data is", bitmapFileSize - (dibLengthOfBitMapData + bmpDataOffset));
		}
	}
	if(dibColoursInPalette == 2) {
		if(verbose) {
			bitmapInfo.printMessage(ConsoleOutput::INFO, "Number of colours in palette is", dibColoursInPalette);
		}
	}
	else {
		bitmapInfo.printMessage(ConsoleOutput::ERR, "Number of colours in the palette must be 2, instead it is", dibColoursInPalette);
		bitmapFile.close();
		return false;
	}

	//--------------------------------------------------
	// Processing Colour Table Information
	//--------------------------------------------------
	if(verbose) {
		cout << endl;
		bitmapInfo.printHeading("Colour table information");
	}
	// The colour table starts after the DIB information header and the (optional) RGB bitmasks
	// The (optional) rgb bitmasks are not useful here, so the offset within the file of the
	// colour table needs to be calculated
	const unsigned int colourTableOffset = 14 + dibDibLength;
	const unsigned int colourTableLength = bmpDataOffset - colourTableOffset;
	const unsigned int numColoursInColourTable = colourTableLength / 4;
	if(colourTableLength == 8) {
		if(verbose) {
			bitmapInfo.printMessage(ConsoleOutput::INFO, "The colour table length is", colourTableLength);
			bitmapInfo.printMessage(ConsoleOutput::INFO, "The offset within the file at which the colour table begins is",	colourTableOffset);
		}
	}
	else {
		bitmapInfo.printMessage(ConsoleOutput::ERR,	"Colour table length should be 8 bytes, instead it is",	colourTableLength, "bytes");
		bitmapFile.close();
		return false;
	}
	// Each bit of the bit map data in the bitmap file does not represent a colour, isntead it represents
	// an index in the colour table from which the actual colour of that pixel can be looked up.
	// Each colour table entry contains 4 bytes in standard ARGB (alpa, red, green, blue) format.
	// But being lttle endian, they appear in the file as BGRA.
	// Being a monochrome bitmap there should only be 2 colour table entries, hence the earlier
	// checks for (dibColoursInPalette == 2) and (colourTableLength == 8)
	// Expecting colour table data as follows:
	//   Index 0  |  Index 1
	// BB GG RR AA BB GG RR AA
	// Need to establish which index to use as black and which to use as white
	// The Sharp Memory LCDs interpret pixel data as 0=black, 1=white
	// RGB colour codes 0xFFFFFF=white, 0x000000=black
	// Ideally an index value of 0 will correspond to black, avoiding the need to invert the whole bit map before displaying it on the lcd
	// Padding bits and bytes at the endo of each line appear to be stored as zeroes
	bool invertBitMap = false;
	uint32_t * colourTable = new uint32_t[numColoursInColourTable];
	bitmapFile.seekg(colourTableOffset);
	for (unsigned int i = 0; i < numColoursInColourTable; i++) {
		bitmapFile.read((char *)(colourTable + i), sizeof(uint32_t));
		if(!bitmapFile) {
			bitmapInfo.printMessage(ConsoleOutput::ERR,	"Unable to read sufficient bytes from file to populate colour table", "");
			bitmapFile.close();
			return false;
		}
		else {
			// remove alpha chanel byte which is not needed (set to zero in this case)
			// colourTable[i] &= ~(0xFF << 24);
		}
	}

	// Monochrome colours might not be black and white, so just take lowest value hex colour value as black
	if(colourTable[0] < colourTable[1]) {
		invertBitMap = false;
		if(verbose) {
			bitmapInfo.printMessage(ConsoleOutput::INFO, "Bitmap file bit map maps 0 to black and 1 to white", "");
			bitmapInfo.printMessage(ConsoleOutput::INFO, "No need to invert bit map data for display on Memory LCD", "");
		}
	}
	else {
		invertBitMap = true;
		if(verbose) {
			bitmapInfo.printMessage(ConsoleOutput::INFO, "Bitmap file bit map maps 0 to white and 1 to black", "");
			bitmapInfo.printMessage(ConsoleOutput::INFO, "Need to invert bit map data for display on Memory LCD", "");
		}
	}

	//--------------------------------------------------
	// Extract the bit map data
	//--------------------------------------------------

	// Need the data presented from top row to bottom row without any padding bytes
	// but bitmap files store the data from bottom row to top row.
	// Bitmap files also pad each row to the nearest multiple of 4 bytes
	const unsigned int bytesInImageRow = ceil((double)(float)dibImageWidth/8);  // minimum number of bytes required to store image pixel width
	unsigned int bytesInBitMapRow; // this value includes any multiple-of-4 padding bytes
	if(bytesInImageRow % 4 == 0) {
		bytesInBitMapRow = bytesInImageRow;
	}
	else {
		bytesInBitMapRow =  4 * ceil((double)(float)bytesInImageRow/4);
	}
	if(verbose) {
		cout << endl;
		bitmapInfo.printHeading("Bit map information");
		bitmapInfo.printMessage(ConsoleOutput::INFO, "Minimum number of bytes required to store one row of pixels according to image width is", bytesInImageRow, "bytes");
		bitmapInfo.printMessage(ConsoleOutput::INFO, "Number of bytes required to store one row of bit map data with 4-byte-multiple padding is", bytesInBitMapRow, "bytes");
	}

	const uint32_t numBytesInBitmap = ceil((double)dibImageHeight * bytesInImageRow);
	uint8_t * bitmapData = new uint8_t[numBytesInBitmap];

	for(unsigned int currentLine = 0; currentLine < dibImageHeight; currentLine++) {
		bitmapFile.seekg(bmpDataOffset + (bytesInBitMapRow*dibImageHeight) - ((currentLine+1)*bytesInBitMapRow));
		bitmapFile.read((char *)bitmapData+(currentLine*bytesInImageRow), bytesInImageRow);
		if(bitmapFile.gcount() != bytesInImageRow) {
			bitmapInfo.printMessage(ConsoleOutput::ERR, "Unable to read sufficent bytes from bit map to fill a row in the framebuffer", "");
			bitmapInfo.printMessage(ConsoleOutput::ERR, "Failed on image line", currentLine);
			bitmapFile.close();
			return false;
		}
	}
	if(invertBitMap) {
		for(unsigned int i = 0; i<numBytesInBitmap; i++) {
			bitmapData[i] = ~(bitmapData[i]);
		}
	}
	// Ensure that all padding bits in bytes at the RHS of the bitmap are ones
	if(dibImageWidth%8 != 0) {
		const uint8_t bitMask = ( pow(2,(8- (dibImageWidth%8) ) ) - 1);
		for(unsigned int row=1; row<dibImageHeight+1; row++) {
			bitmapData[(row*bytesInImageRow)-1] |= bitMask;
		}
	}

	//--------------------------------------------------
	// Establish the limits of each icon within the bitmap
	//--------------------------------------------------

	// in bitmap data, 1 is white, 0 is black
	// First establish overall bounds of the rows and columns
	// First element of pair is start of a row/col, second element of pair is end of a row/col
	std::list <std::pair <unsigned int, unsigned int>> rows;
	std::list <std::pair <unsigned int, unsigned int>> cols;
	// Find tops and bottoms of icon rows
	bool iconRowDetected = false;
	for(unsigned int row=0; row<dibImageHeight; row++) {
		bool pixelDetectedInRow = false;
		for(unsigned int col=0; col<bytesInImageRow; col++) {
			// detect black pixels
			if(bitmapData[(row*bytesInImageRow)+col] != 0xFF) {
				pixelDetectedInRow = true;
				break;
			}
		}
		// Start of an icon row detected
		if(!iconRowDetected && pixelDetectedInRow) {
			iconRowDetected = true;
			// Set start of row in list of pairs
			rows.emplace_back();
			rows.back().first = row;
		}
		// end of an icon row detected
		else if(iconRowDetected && !pixelDetectedInRow) {
			iconRowDetected = false;
			rows.back().second = row - 1;
		}
	}
	if(rows.size() == 0) {
		bitmapInfo.printMessage(ConsoleOutput::ERR, "No icon rows found in bitmap image", "");
		bitmapFile.close();
		return false;
	}

	// Find lefts and rights of icon columns
	bool iconColDetected = false;
	for(unsigned int col=0; col<dibImageWidth; col++) {
		bool pixelDetectedInCol = false;
		for(unsigned int row=0; row<dibImageHeight; row++) {
			// isolate and invert byte from bitmapData array
			const unsigned int byteInCurrentRow = floor((double)col/8);
			const uint8_t bitmask = (1 << (7-(col%8)));
			const uint8_t currentByte = ~(bitmapData[(row*bytesInImageRow)+byteInCurrentRow]);
			if(bitmask & currentByte) {
				pixelDetectedInCol = true;
				break;
			}
		}
		// start of icon column detected
		if(!iconColDetected && pixelDetectedInCol) {
			iconColDetected = true;
			cols.emplace_back();
			cols.back().first = col;
		}
		// end of icon column detected
		else if(iconColDetected && !pixelDetectedInCol) {
			iconColDetected = false;
			cols.back().second = col - 1;
		}
	}

	if(verbose) {
		bitmapInfo.printMessage(ConsoleOutput::INFO, "There are", rows.size(), "rows of icons detected in the bitmap");
		bitmapInfo.printMessage(ConsoleOutput::INFO, "There are", cols.size(), "columns of icons detected in the bitmap");
	}

	//--------------------------------------------------
	// Determine precise extents for each individual icon
	//--------------------------------------------------
	// Only the rough boundaries of the icons have been found within the overall rows and columns so far
	// Individual icons may not necessarily be centred in each row or column
	// Now there is the opportunity to discover the extents of each individual icon
	// This will allow the maximum icon size to be determined and, if neccessary, to add white borders
	// to smaller icons as they are extracted to their indivudual bitmap files.

	// Discover the extents of each individual icon
	struct iconExtents {
		unsigned int top;
		unsigned int bottom;
		unsigned int left;
		unsigned int right;
	};
	std::list<iconExtents> iconList;
	for(std::list<std::pair<unsigned int, unsigned int>>::iterator itRow = rows.begin(); itRow!=rows.end(); itRow++) {
		for(std::list<std::pair<unsigned int, unsigned int>>::iterator itCol = cols.begin(); itCol!=cols.end(); itCol++) {
		iconList.emplace_back();
			// find extents of current icon
			bool foundPixel = false;
			// find top extent of icon
			for(unsigned int row = itRow->first; row < (itRow->second + 1); row++) {
				for(unsigned int col = itCol->first; col < (itCol->second + 1); col++) {
					// check relevant pixel with a bitmask and bitwise AND operation
					const unsigned int byteInCurrentRow = floor((double)col/8);
					const uint8_t bitmask = (1 << (7-(col%8)));
					const uint8_t currentByte = ~(bitmapData[(row*bytesInImageRow)+byteInCurrentRow]);
					if(bitmask & currentByte) {
						iconList.back().top = row;
						foundPixel = true;
						break;
					}
				}
				if(foundPixel) {
					break;
				}
			}
			// Quick sanity check before proceeding further (only need to do this check once)
			// Check if any pixels found at this particular row/col grid. If not then it is an incomplete
			// row/col with no icon present at this particular grid.
			// Delete the list element created earlier to hold the now non-existent icon and break to the next iteration of row
			if(!foundPixel) {
				bitmapInfo.printMessage(ConsoleOutput::WARN, "Unable to find any pixels within the following row/column bounds", "");
				bitmapInfo.printMessage(ConsoleOutput::WARN, "Top bound is", itRow->first);
				bitmapInfo.printMessage(ConsoleOutput::WARN, "Bottom bound is", itRow->second);
				bitmapInfo.printMessage(ConsoleOutput::WARN, "Left bound is", itCol->first);
				bitmapInfo.printMessage(ConsoleOutput::WARN, "Right bound is", itCol->second);
				iconList.pop_back();
			}
			else {
				// find bottom extent of icon
				foundPixel = false;
				for(unsigned int row = itRow->second; row > (itRow->first - 1); row--) {
					for(unsigned int col = itCol->first; col < (itCol->second + 1); col++) {
						// check relevant pixel with a bitmask and bitwise AND operation
						const unsigned int byteInCurrentRow = floor((double)col/8);
						const uint8_t bitmask = (1 << (7-(col%8)));
						const uint8_t currentByte = ~(bitmapData[(row*bytesInImageRow)+byteInCurrentRow]);
						if(bitmask & currentByte) {
							iconList.back().bottom = row;
							foundPixel = true;
							break;
						}
					}
					if(foundPixel) {
						break;
					}
				}
				// find left extent of icon
				foundPixel = false;
				for(unsigned int col = itCol->first; col < (itCol->second + 1); col++) {
					const unsigned int byteInCurrentCol = floor((double)col/8);
					const uint8_t bitmask = (1 << (7-(col%8)));
					for(unsigned int row = itRow->first; row < (itRow->second + 1); row++) {
						const uint8_t currentByte = ~(bitmapData[(row*bytesInImageRow)+byteInCurrentCol]);
						if(bitmask & currentByte) {
							iconList.back().left = col;
							foundPixel = true;
							break;
						}
					}
					if(foundPixel) {
						break;
					}
				}
				// find right extent of icon
				foundPixel = false;
				for(unsigned int col = itCol->second; col > (itCol->first - 1); col--) {
					const unsigned int byteInCurrentCol = floor((double)col/8);
					const uint8_t bitmask = (1 << (7-(col%8)));
					for(unsigned int row = itRow->first; row < (itRow->second + 1); row++) {
						const uint8_t currentByte = ~(bitmapData[(row*bytesInImageRow)+byteInCurrentCol]);
						if(bitmask & currentByte) {
							iconList.back().right = col;
							foundPixel = true;
							break;
						}
					}
					if(foundPixel) {
						break;
					}
				}
			}
		}
	}

	// TODO: Delete as not really necessary? Plus it clogs up the verbose output for individual icon information with info about the overall bitmap
	// sanity check - have we stored the extents of all icons discovered in the earlier, cruder search for rows and columns?
//	if(iconList.size() != (rows.size()*cols.size())) {
//		bitmapInfo.printMessage(ConsoleOutput::WARN, "Fewer icons found in search for individual extents than in search for rows and columns", "");
//		bitmapInfo.printMessage(ConsoleOutput::WARN, "This suggests an incomplete row or column of icons within the bitmap file", "");
//		bitmapInfo.printMessage(ConsoleOutput::WARN, "Number of rows found is", rows.size());
//		bitmapInfo.printMessage(ConsoleOutput::WARN, "Numer of columns found is", cols.size());
//		bitmapInfo.printMessage(ConsoleOutput::WARN, "Product of rows and columns is", rows.size()*cols.size());
//		bitmapInfo.printMessage(ConsoleOutput::WARN, "Number of icons found is", iconList.size());
//	}

	// Find largest horizontal and vertical dimensions of the icons
	uint32_t minIconWidth = UINT32_MAX;
	uint32_t maxIconWidth = 0;
	uint32_t minIconHeight = UINT32_MAX;
	uint32_t maxIconHeight = 0;
	// +1 for actual pixel width e.g. an icon from px2 to px6 is 5 pixels wide
	// 0 1 2 3 4 5 6 7 8 9
	// - - X X X X X - - -
	for(std::list<iconExtents>::iterator icon = iconList.begin(); icon != iconList.end(); icon++) {
		const unsigned int currentHeight = (icon->bottom - icon->top) + 1;
	    const unsigned int currentWidth = (icon->right - icon->left) + 1;
		maxIconHeight = (currentHeight > maxIconHeight) ? currentHeight : maxIconHeight;
		minIconHeight = (currentHeight < minIconHeight) ? currentHeight : minIconHeight;
		maxIconWidth = (currentWidth > maxIconWidth) ? currentWidth : maxIconWidth;
		minIconWidth = (currentWidth < minIconWidth) ? currentWidth : minIconWidth;
	}
	// TODO: Delete as not really necessary? Plus it clogs up the verbose output for individual icon information with info about the overall bitmap
//	if(verbose) {
//		bitmapInfo.printMessage(ConsoleOutput::INFO, "Minimum icon pixel height is", minIconHeight);
//		bitmapInfo.printMessage(ConsoleOutput::INFO, "Maximum icon pixel height is", maxIconHeight);
//		bitmapInfo.printMessage(ConsoleOutput::INFO, "Minimum icon pixel width is", minIconWidth);
//		bitmapInfo.printMessage(ConsoleOutput::INFO, "Maximum icon pixel width is", maxIconWidth);
//	}

	//--------------------------------------------------
	// Create new bitmap files for each individual icon
	//--------------------------------------------------
	unsigned int iconNumber = 0;
	for(std::list<iconExtents>::iterator it = iconList.begin(); it != iconList.end(); it++, iconNumber++) {
		if(verbose) {
			cout << endl;
			bitmapInfo.printHeading("Icon information");
		}
		// Create numbered flenames with enough leading zeroes so that the lowest numbers are the same length as the highest
		// TODO prepend file path onto filename
		std::string fileNumber = std::to_string(iconNumber);
		fileNumber.insert(0,(std::to_string(iconList.size()).size() - fileNumber.size()),'0');
		fileNumber.append(".bmp"); // TODO: prefix a proper full path onto the filename
		std::ofstream iconFile;
		iconFile.open(fileNumber.insert(0,outputDir), (std::ofstream::out | std::ofstream::binary | std::ios::trunc));
		if(iconFile.fail()) {
			cerr << "ERROR\t\tFailed to create icon file " << fileNumber << endl;
			cout << endl;
			return false;
		}
		else {
			if(verbose) {
				bitmapInfo.printMessage(ConsoleOutput::INFO, "Icon bitmap file", fileNumber, "created for writing");
			}
		}

		uint32_t iconWidth = 0;
		uint32_t iconHeight = 0;
		if(sameSizeIcons) {
			//  maxIconWIdth has already been +1'ed above
			iconWidth = maxIconWidth + (2*horizontalMargin);
			iconHeight = maxIconHeight + (2*verticalMargin);
		}
		else {
			// +1 for actual pixel width e.g. an icon from px2 to px6 is 5 pixels wide
			// 0 1 2 3 4 5 6 7 8 9
			// - - X X X X X - - -
			iconWidth = (it->right - it->left) + 1 + (2*horizontalMargin);
			iconHeight = (it->bottom - it->top) + 1 + (2*verticalMargin);
		}
		const unsigned int iconArraySize = ceil((double)iconWidth/8) * iconHeight;
		uint8_t * iconData = new uint8_t[iconArraySize];
		// as the iconData array will be largely filled by bitwise OR operations it is important to initialise it to zeroes
		for(unsigned int i=0; i<iconArraySize; i++) {
			iconData[i] = 0x00;
		}

		if(verbose) {
			bitmapInfo.printMessage(ConsoleOutput::INFO, "Horizontal margin of", horizontalMargin, "pixels added to this icon");
			bitmapInfo.printMessage(ConsoleOutput::INFO, "Vertical margin of", verticalMargin, "pixels added to this icon");
			bitmapInfo.printMessage(ConsoleOutput::INFO, "Icon pixel width including margin is", iconWidth);
			bitmapInfo.printMessage(ConsoleOutput::INFO, "Icon pixel height including margin is", iconHeight);
			bitmapInfo.printMessage(ConsoleOutput::INFO, "Size of array required to hold this icon is", iconArraySize);
		}

		// Add margins to all sides and any additional white padding required if current icon dimensions != max icon dimensions
		unsigned int whitePixelsAtTop    = verticalMargin   + ceil((double)  ( iconHeight - (2*verticalMargin)  - ((it->bottom - it->top) + 1) ) /2 );
		unsigned int whitePixelsAtBottom = verticalMargin   + floor((double) ( iconHeight - (2*verticalMargin)  - ((it->bottom - it->top) + 1) ) /2 );
		unsigned int whitePixelsAtLeft   = horizontalMargin + ceil((double)  ( iconWidth - (2*horizontalMargin) - ((it->right - it->left) + 1) ) /2 );
		unsigned int whitePixelsAtRight  = horizontalMargin + floor((double) ( iconWidth - (2*horizontalMargin) - ((it->right - it->left) + 1) ) /2 ); // TODO: make sure the padding bits at the end of each line are also 1'ed
		// add top margin
		for(unsigned int i=0; i<whitePixelsAtTop*ceil((double)iconWidth/8); i++) {
			iconData[i] |= 0xFF;
		}

		// add bottom margin
		for(unsigned int i = (whitePixelsAtTop + ((it->bottom - it->top) + 1)) * ceil((double)iconWidth/8); i<iconArraySize; i++) {
			iconData[i] |= 0xFF;
		}

		// add left margin
		for(unsigned int row=whitePixelsAtTop; row<iconHeight-whitePixelsAtBottom; row++) {
			unsigned int col = 0;
			unsigned int currentByte = row * ceil((double)iconWidth/8);
			while(col<whitePixelsAtLeft) {
				// Assumes always starting at MSB of icon byte
				if(whitePixelsAtLeft-col > 7) {
					iconData[currentByte++] |= 0xFF;
					col += 8;
				}
				else {
					uint8_t bitmask = ((uint8_t)pow(2,whitePixelsAtLeft-col)-1) << (8-(whitePixelsAtLeft-col));
					iconData[currentByte++] |= bitmask;
					col += whitePixelsAtLeft-col;
				}
			}
		}

		// add right margin
		// this is trickier as it may neither start nor end at the start or end of a byte
		for(unsigned int row=whitePixelsAtTop; row<iconHeight-whitePixelsAtBottom; row++) {
			unsigned int col = iconWidth - whitePixelsAtRight;
			unsigned int currentByte = (row * ceil((double)iconWidth/8)) + floor((double)col/8);
			unsigned int endOfLineBit = ceil((double)iconWidth/8)*8; // ensures that padding bits at the end of each line are set to 1
			while(col<endOfLineBit) {
				uint8_t bitmask = pow(2,8-(col%8))-1;
				iconData[currentByte++] |= bitmask;
				col += 8-(col%8);
			}
		}

		// The magic happens here:
		// Copy and bitshift all pixels from the defined "icon" regions in the original bitmap to the new individual icon files
		for(unsigned int iconRow = whitePixelsAtTop; iconRow < iconHeight-whitePixelsAtBottom; iconRow++) {
			unsigned int iconCol = whitePixelsAtLeft;
			unsigned int bitmapRow = it->top + (iconRow - whitePixelsAtTop);
			unsigned int bitmapCol = it->left;
			unsigned int currentIconByte = (iconRow * ceil((double)iconWidth/8)) + floor((double)iconCol/8);
			unsigned int currentBitmapByte = (bitmapRow * ceil((double)dibImageWidth/8)) + floor((double)bitmapCol/8);
			while(iconCol < iconWidth-whitePixelsAtRight) {
				uint8_t bitInBitmapByte = (bitmapCol%8);	// 0->7, msb->lsb
				uint8_t bitInIconByte = (iconCol%8); 		// 0->7, msb->lsb
				// number of bits to copy must be lt or eq to bits remaining in current icon byte
				// number of bits to copy must be lt or eq to bits remaining in current bitmap byte
				// number of bits to copy must be lt or eq to bits remaining on current line
				uint8_t numBitsToCopy = 0;
				unsigned int numBitsLeftOnRow = ((iconWidth-whitePixelsAtRight)-iconCol);
				uint8_t bitsLeftInIconByte = 8-bitInIconByte;	// Max number of bits possible to copy from bitmapData to iconData in next operation
				uint8_t bitsLeftInBitmapByte = 8-bitInBitmapByte; // Number of bits left to copy from current bitmap byte
				numBitsToCopy = (bitsLeftInIconByte > bitsLeftInBitmapByte) ? bitsLeftInBitmapByte : bitsLeftInIconByte;
				numBitsToCopy = (numBitsToCopy < numBitsLeftOnRow ) ? numBitsToCopy : numBitsLeftOnRow;
				// Construct a bitmask containing the bits to copy from bitmapData to iconData
				uint8_t bitmask = 0x00;
				uint8_t bitsToCopy = bitmapData[currentBitmapByte];
				// e.g. need to write bits     --XXX--- to icon data (making this example work, with 4 remaining but 3 to copy, should ensure end of line compatibility)
				// from a bitmapByte with bits ----XXXX remaining
				// Zero any already-copied bits in the bitsToCopy byte
				bitmask = pow(2,bitsLeftInBitmapByte)-1; // WAS: bitmask = pow(2,bitInBitmapByte)-1; // WAS: bitmask = pow(2,bitInBitmapByte+1)-1;
				bitsToCopy &= bitmask;
				if(numBitsToCopy < bitsLeftInBitmapByte) {
						bitmask = pow(2,bitsLeftInBitmapByte-numBitsToCopy)-1;
						bitsToCopy ^= bitmask;
				}
				// Shift the bitsToCopy left OR right depending on where the iconCol is relative to the bitmapCol
				// NB: left shift by a negative number is undefined behaviour in C++
				if(bitInIconByte < bitInBitmapByte) {
					bitsToCopy = bitsToCopy << (bitInBitmapByte - bitInIconByte);
				}
				else {
					bitsToCopy = bitsToCopy >> (bitInIconByte - bitInBitmapByte);
				}
				// Update iconData - at last...
				iconData[currentIconByte] |= bitsToCopy;

				// Update all counter variables ready for next iteration
				// Neither the currentIconByte nor the currentBitmapByte necessarily increment after a copy operation
				if(bitsLeftInIconByte - numBitsToCopy == 0) {
					currentIconByte++;
				}
				if(bitsLeftInBitmapByte - numBitsToCopy == 0) {
					currentBitmapByte++;
				}
				iconCol += numBitsToCopy;
				bitmapCol += numBitsToCopy;
			}
		}

		// Use the headers from the original bitmap file to form the foundation of the headers for the individual icons' bitmap files
		// Copy original bitmap file as far as the start of the bit map data

		// The original header must now be modified for:
		// 		- the new icon bitmap file size (bmp header)
		//		- the new icon bitmap width (dib header)
		//		- the new icon bitmap height (dib header)
		//		- length of bit map data (dib header)
		// 		- colours in colour table may need to be swapped around

		// Copy start of original bitmap file right up to the start of the bit map data
		bitmapFile.seekg(0);
		char * buffer = new char[bmpDataOffset];
		bitmapFile.read(buffer,bmpDataOffset);
		if(!bitmapFile) {
			bitmapInfo.printMessage(ConsoleOutput::ERR, "Unable to read all headers from bitmap file. Failed after", bitmapFile.gcount(), "bytes");
			bitmapInfo.printMessage(ConsoleOutput::ERR, "Expected to read", bmpDataOffset, "bytes");
			bitmapFile.close();
			iconFile.close();
			return false;
		}
		iconFile.write(buffer, bmpDataOffset);
		if(!iconFile) {
			bitmapInfo.printMessage(ConsoleOutput::ERR, "Unable to write all headers to icon file. Failed after", bitmapFile.gcount(), "bytes");
			bitmapInfo.printMessage(ConsoleOutput::ERR, "Expected to write", bmpDataOffset, "bytes");
			bitmapFile.close();
			iconFile.close();
			return false;
		}

		// Calculate new file size and write it to iconFile
		const uint32_t iconFileDataSize = (4* ceil( ceil((double)iconWidth/8) /4)) * iconHeight;
		const uint32_t iconCalculatedFileSize = bmpDataOffset + iconFileDataSize;
		iconFile.seekp(2);
		iconFile.write((char *)&iconCalculatedFileSize, sizeof(uint32_t));
		if(verbose) {
			bitmapInfo.printMessage(ConsoleOutput::INFO, "Size of icon file calculated to be", iconCalculatedFileSize, "bytes");
		}

		// Write icon dimensions to iconFile
		// Position:18-21, Length:4, Info: Image width in pixels
		iconFile.seekp(18);
		iconFile.write((char *)&iconWidth, sizeof(uint32_t));
		// Position 22-25, Length:4, Info: Image height in pixels
		iconFile.seekp(22);
		iconFile.write((char *)&iconHeight, sizeof(uint32_t));


		// Write length of bit map data to icon file
		// Position 34-37, Length:4, Info: length of bit map data within the bitmap file
		iconFile.seekp(34);
		iconFile.write((char *)&iconFileDataSize, sizeof(uint32_t));

		if(invertBitMap) {
			iconFile.seekp(colourTableOffset);
			// A bit lazy but now cofirmed that the colour table is only for 2 colours
			iconFile.write((char *)(&colourTable[1]), sizeof(uint32_t));
			iconFile.write((char *)(&colourTable[0]), sizeof(uint32_t));
		}

		// Write iconData to iconFile
		// extra padding bytes must be written to the end of each line
		unsigned int bytesInIconRow = ceil((double)iconWidth/8);
		unsigned int numPaddingBytes = 0;
		if(bytesInIconRow%4 != 0) {
			numPaddingBytes = 4-(bytesInIconRow%4);
		}
		char * paddingBytes = new char[numPaddingBytes];
		for(uint8_t i=0; i<numPaddingBytes; i++) {
			paddingBytes[i] = 0xFF;
		}
		iconFile.seekp(bmpDataOffset);
		if(!iconFile) {
			bitmapInfo.printMessage(ConsoleOutput::ERR, "Unable to seek to start of iconData in iconFile for writing", "");
			bitmapFile.close();
			iconFile.close();
			return false;
		}
		for(unsigned int row=0; row<iconHeight; row++) {
//			cout << "iconFile to be written at position " << iconFile.tellp() << endl;
			unsigned int iconDataOffset = (iconArraySize-((row+1)*bytesInIconRow));
//			cout << "iconDataOffset is " << iconDataOffset << endl;
			iconFile.write((char*)&iconData[iconDataOffset] ,bytesInIconRow);
			if(numPaddingBytes != 0) {
				iconFile.write(paddingBytes, numPaddingBytes);
			}
		}

		// check measured file size of iconFile against its calculated file size.
		// get iconFile size
		iconFile.seekp(0, iconFile.end);
		const unsigned int iconActualFileSize = iconFile.tellp();
		iconFile.seekp(0, iconFile.beg);
		if(iconActualFileSize != iconCalculatedFileSize) {
			bitmapInfo.printMessage(ConsoleOutput::ERR,	"Size calculated for iconFIle is different to actual size of iconFile", fileNumber);
			bitmapInfo.printMessage(ConsoleOutput::ERR,	"Calculated size for iconFile is", iconCalculatedFileSize, "bytes");
			bitmapInfo.printMessage(ConsoleOutput::ERR,	"Actual size for iconFile is    ", iconActualFileSize, "bytes");
			bitmapFile.close();
			iconFile.close();
			return false;
		}

		if(verbose) {
			bitmapInfo.printMessage(ConsoleOutput::INFO, "Successfully created icon file", fileNumber);
		}

		//--------------------------------------------------
		// Delete dynamically allocated memory for icon file
		//--------------------------------------------------
		delete[] paddingBytes;
		delete[] buffer;
		delete[] iconData;
		iconFile.close();

	}

	//--------------------------------------------------
	// Delete dynamically allocated memory for bitmap file
	//--------------------------------------------------
	delete[] colourTable;
	delete[] bitmapData;
	bitmapFile.close();
	return 0;
}
