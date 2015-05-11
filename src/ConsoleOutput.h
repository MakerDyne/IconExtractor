//============================================================================
// Name			: Console Output (ConsoleOutput.h)
// Description 	: Formats program output for consistent display on the command
//				: line (a.k.a console or terminal)
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

#ifndef _CONSOLE_OUTPUT_LIB_H
#define _CONSOLE_OUTPUT_LIB_H

#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

// Class for consistent formatting of serial output to console
class ConsoleOutput {

private:
	const unsigned char hWidth;
	const char hChar;

public:
	// Enumerator for message category
	enum category_t {ERR, WARN, INFO, STATUS};


	// Constructor
	ConsoleOutput(unsigned char headingWidth, char headingChar) : hWidth(headingWidth), hChar(headingChar) {
		//
	}


	// Prints a message of a particular category
	template <typename T> void printMessage(const category_t cat, const char * message, const T value, const char * units = "") const {
		bool(isError) = false;
		// TODO optional timestamp (of different formats, millis, HH:MM:SS etc)
		switch(cat) {
		case 0:
			isError = true;
			cerr << "ERROR";
			break;
		case 1:
			cout << "WARNING";
			break;
		case 2:
			cout << "INFO";
			break;
		case 3:
			cout << "STATUS";
			break;
		default:
			break;
		}
		// TODO check first char of message is lowercase alphabet and capitalise it.
		if(isError) {
			cerr << ":\t\t" << message << " " << value << " " << units << endl;
		}
		else {
			cout << ":\t\t" << message << " " << value << " " << units << endl;
		}
	}


	// Prints a title with a line of spacer characters both above and below it
	void printHeading(const char * title) const {
		if(hWidth != 0) {
			for(unsigned char i=0; i<hWidth; i++) {
				cout << hChar;
			}
			cout << endl;
		}
		cout << title << endl;  // TODO centre title and surround with chars?
		if(hWidth != 0) {
			for(unsigned char i=0; i<hWidth; i++) {
				cout << hChar;
			}
			cout << endl;
		}
	}


	// Prints a line of spacer characters
	void printDivider() const {
		for(unsigned char i=0; i<hWidth; i++) {
			cout << hChar;
		}
		cout << endl;
	}

};
#endif
