// EEC170_Project2_Cache.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

using namespace std;

/*

* R-type

  31        25 24     20 19     15 14  12 11      7 6           0
 +------------+---------+---------+------+---------+-------------+
 | funct7     | rs2     | rs1     |funct3| rd      | opcode      |
 +------------+---------+---------+------+---------+-------------+

* I-type

  31                  20 19     15 14  12 11      7 6           0
 +----------------------+---------+------+---------+-------------+
 | imm                  | rs1     |funct3| rd      | opcode      |
 +----------------------+---------+------+---------+-------------+

* S-type

  31        25 24     20 19     15 14  12 11      7 6           0
 +------------+---------+---------+------+---------+-------------+
 | imm        | rs2     | rs1     |funct3| imm     | opcode      |
 +------------+---------+---------+------+---------+-------------+

* U-type

  31                                      11      7 6           0
 +---------------------------------------+---------+-------------+
 | imm                                   | rd      | opcode      |
 +---------------------------------------+---------+-------------+
 
*/

/*
Goals:
	1. Count the number of instructions executed
	2. Compare fraction of loads, stores, branches, arithmetic, etc. instructions
		* Operand and funct3/funct7
	3. Detect fraction of load-use hazard cases to all memory loads
		* Current and previous instruction
	4. Find fraction of register writes that must be forwarded
		* 
	5. Effectiveness of static branch prediction

*/

typedef unsigned int Address;

enum InstructionType
{
	Load = 0,
	Store = 1,
	Branch = 2,
	Arithmetic = 3,
	Invalid = 99
};

struct Instruction
{
	Address address;
	InstructionType type;

	unsigned int opcode;
	unsigned int rd;
	unsigned int rs1;
	unsigned int rs2;
	unsigned int funct3;
	unsigned int funct7;
	unsigned int imm;

	Instruction(unsigned int address, unsigned int data)
	{
		this->address = address;

		opcode = data & 0x0000001f;
		rd = (data >> 7) & 0x0000001f;
		funct3 = (data >> 12) & 0x00000007;
		rs1 = (data >> 15) & 0x0000001f;
		rs2 = (data >> 20) & 0x0000001f;
		funct7 = (data >> 25) & 0x0000007f;


		// Set immediate variable
		if (opcode == 0b0010011		// Logical I-type
			|| opcode == 0b0000011  // Loads
			|| opcode == 0b1100111) // JALR
		{
			imm = (data >> 20) & 0x00000fff;
		}
		else if (opcode == 0b0)
		{

		}

		// Set instruction type
		if (opcode == 0b0110011 || opcode == 0b0010011) { this->type = Arithmetic; }
		else if (opcode == 0b0100011) { this->type = Store; }
		else if (opcode == 0b0) { this->type = Load; }
		else if (opcode == 0b0) { this->type = Branch; }
		else { this->type = Invalid; }

	}
};

void checkLoadUseHazard(Instruction currInstr, Instruction prevInstr);

const int blockCount = 1;
const int blockSize = 1;

unsigned int totalInstructions = 0;
unsigned int instructionsOfType[4] = {};

unsigned int cache_hits = 0;
unsigned int cache_misses = 0;

//Instruction previous = 0x00000000;
//Address cache[blockCount][blockSize];

int main(int argc, char* argv[])
{
	//unsigned int a = 0b11111110000011111000111110000000;
	//Instruction i(a);

	// Check if a log file is specified
	if (argc < 2)
	{ 
		cerr << "Usage: " << argv[0] << "<logfile>" << endl; 
		return 1;
	}

	ifstream logfile(argv[1]);

	// Make sure the log file is loaded
	if (!logfile.is_open())
	{
		cerr << "Error: log file not found (" << argv[1] << ")" << endl;
	}

	string line;
	char op, colon;

	unsigned int data;
	unsigned int previousData;

	while (getline(logfile, line))
	{
		istringstream stream(line);

		if (!(stream >> op >> colon >> hex >> data))
		{
			cerr << "Parse error: " << line << endl;
			break;
		}

		switch (op)
		{
		case 'i':
			Instruction instr(previousData, data);
			instructionsOfType[instr.type]++;
			totalInstructions++;
			break;
		case 'I':
		default:
			break;
		}


		//checkLoadUseHazard(op, data);
		previousData = data;
	}


}

void checkLoadUseHazard(Instruction currInstr, Instruction prevInstr)
{

}
/*
int main(int argc, char* argv[])
{

	std::ifstream infile(argv[1]);

	std::string line;
	char op, colon;

	typedef unsigned int Instruction;
	typedef unsigned int Address;
	unsigned int data;

	Instruction previous = 0x00000000;

	// This is a stupid cache. It has one entry of one word.
	Address cache[1];
	cache[0] = 0x00000000;
	unsigned int cache;

	while(std::getline(infile, line))
	{
		std::istringstream iss(line);
		int a, b;
		if (!(iss >> op >> colon >> std::hex >> data)) {
			std::cerr << "Parse error: " << line << std::endl;
			break;
		}
		std::cout << std::hex << std::setfill('0') << std::setw(8) << std::showbase;

		switch (op) {
		case 'i':
			std::cout << "I see an instruction: "
				<< data << std::endl;
			std::cout << "The previous instruction I ran was: "
				<< previous << std::endl;
			previous = data;    // for next time
			break;
		case 'I':
			std::cout << "I accessed an instruction at address: "
				<< data << std::endl;
			break;
		case 'L':
			std::cout << "I issued a load from memory address: "
				<< data << std::endl;
			// let's see if it's a hit!
			// our cache stores a whole word, so any access within the word
			//   should be a hit
			data &= 0xfffffffc; // this sets bottom two bits to zero
			if (data == cache[0]) {
				cache_hits++;
				std::cout << "That data access was a hit in cache!" << std::endl;
			}
			else {
				cache_misses++;
				std::cout << "That data access was a miss in cache!" << std::endl
					<< "What was actually in that cache slot was " << cache[0] << std::endl;

				// now update the cache
				cache[0] = data;
			}
			break;
		case 'S':
			std::cout << "I issued a store to memory address: "
				<< data << std::endl;
			break;
		default:
			std::cerr << "Something went wrong: I see op " << op << std::endl;
			return -1;
			break;
		}

	}
	std::cout << std::dec << std::setw(0) << std::noshowbase;
	std::cout << std::dec << "I saw " << cache_hits << " cache hits and " << cache_misses << " cache misses" << std::endl;
	return 0;
}
*/