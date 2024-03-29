// EEC170_Project2_Cache.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <math.h>
#include <algorithm>

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
		* Exe -> Exe
		* Exe -> -> Exe
		* Mem -> -> Exe

	5. Effectiveness of static branch prediction

*/

const int cacheSize = 1024; // specified in bytes
const int blockSize = 16;    // specified in words 
const int associativity = 2; // blocks per set
const int blockCount = cacheSize / (blockSize * 4);
double associativity_penalty = max((associativity > 1 ? 0.1 : 0) + (log2(associativity) - 1) * 0.02, (double)0); // 10% + (2% * (associativity - 1))

typedef unsigned int Address;

enum class InstructionType
{
	Load = 0,
	Store = 1,
	Branch = 2,
	Arithmetic = 3,
	Other = 4
};

struct Instruction
{
	Address address;
	InstructionType type;

	unsigned int opcode;
	unsigned int rd;
	unsigned int rs1;
	unsigned int rs2;
	unsigned int signbit;

	Instruction() {}

	Instruction(unsigned int address, unsigned int data)
	{
		this->address = address;

		opcode = data & 0x0000007f;
		rd = (data >> 7) & 0x0000001f;
		rs1 = (data >> 15) & 0x0000001f;
		rs2 = (data >> 20) & 0x0000001f;
		signbit = (data >> 31) & 0x00000001;

		// Set instruction type
		if (opcode == 0b0110011       // Arithmetic
			|| opcode == 0b0010011)   // Immediate arithmetic
		{ 
			this->type = InstructionType::Arithmetic; 
		}
		else if (opcode == 0b0100011) // Store
		{ 
			this->type = InstructionType::Store; 
		}
		else if (opcode == 0b0000011) // Load
		{ 
			this->type = InstructionType::Load;
		}
		else if (opcode == 0b1100011)  // Branch
//			|| opcode == 0b1100111    // JALR
//			|| opcode == 0b1101111)   // JAL
		{ 
			this->type = InstructionType::Branch;
		}
		else { this->type = InstructionType::Other; }
	}
};

struct Block
{
	Address address;
	unsigned int valid = 0;

	bool checkHit(Block access)
	{
		if (this->valid == 0) { return false; }
		if (access.tag() != this->tag()) { return false; }

		return true;
	}

	void setValid(Block access)
	{
		this->address = access.address;
		this->valid = 1;
	}

	unsigned int tag()
	{
		unsigned int offset_bits = log2(blockSize * 4);
		unsigned int index_bits = log2(blockCount);

		return  (address >> (offset_bits + index_bits)) & 0xffffffff;
	}

	unsigned int index()
	{
		unsigned int offset_bits = log2(blockSize * 4);
		unsigned int index_bits = log2(blockCount);

		return (address >> offset_bits) & ((unsigned int)pow(2, index_bits) - 1);
	}

	Block() { this->address = 0; }
	Block(Address address) { this->address = address; }
};

void checkLoadUseHazard(Instruction currInstr, Instruction prevInstr);
void checkForwarding(Instruction currInstr, Instruction prevInstr, Instruction prevPrevInstr);
void checkStaticBranchPrediction(Instruction currInstr, Instruction prevInstr);

unsigned int totalInstructions = 0;
unsigned int instructionsOfType[5] = {};

unsigned int loadUseHazards = 0;
unsigned int successfulBranchPredictions = 0;
unsigned int dataForwards = 0;

unsigned int cache_hits = 0;
unsigned int cache_misses = 0;

// to access cache[index]
Block cache[blockCount];

int main(int argc, char* argv[])
{
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

	stringstream logFileBuf;
	logFileBuf << logfile.rdbuf();
	logfile.close();

	string line;
	char op, colon;

	unsigned int data = 0;
	unsigned int previousData = 0;

	Instruction prevInstr;
	Instruction prevPrevInstr;

	while(getline(logFileBuf, line))
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
		{
			Instruction instr(previousData, data);

			// Check instruction statistics
			instructionsOfType[(int)instr.type]++;
			totalInstructions++;

			checkLoadUseHazard(instr, prevInstr);
			checkForwarding(instr, prevInstr, prevPrevInstr);
			checkStaticBranchPrediction(instr, prevInstr);

			prevPrevInstr = prevInstr;
			prevInstr = instr;
			break;
		}
		case 'L':
		{
			Block currBlock(data);
			int block_index = currBlock.index();
			int block_set = block_index % (blockCount / associativity); // Calculate which set this memory address should be in
			int beginning_set_index = block_set * associativity; // Calculate beginning set index

			bool hit = false;

			for (int i = 0; i < associativity; i++)
			{
				if (cache[beginning_set_index + i].checkHit(currBlock))
				{
					hit = true;
				}
			}

			if (hit)
			{
				cache_hits++;
			}
			else
			{
				cache_misses++;

				int rand_i = rand() % associativity;
				cache[beginning_set_index + rand_i].setValid(currBlock); // Update this block to the new data
			}

			break;
		}
		case 'S':
		{
			// Write-through no-allocate

			Block currBlock(data);
			int block_index = currBlock.index();
			int block_set = block_index % (blockCount / associativity); // Calculate which set this memory address should be in
			int beginning_set_index = block_set * associativity; // Calculate beginning set index

			int hit = -1;

			for (int i = 0; i < associativity; i++)
			{
				if (cache[beginning_set_index + i].checkHit(currBlock))
				{
					hit = i;
				}
			}

			if (hit > -1)
			{
				cache[beginning_set_index + hit].setValid(currBlock); // Update the cache block hit (and memory) with the new value
			}

			break;
		}
		default:
			break;
		}

		previousData = data;
	}

	cout << "Instruction Type" << endl;
	cout << "Load:       " << instructionsOfType[(int)InstructionType::Load] << endl;
	cout << "Store:      " << instructionsOfType[(int)InstructionType::Store] << endl;
	cout << "Branch:     " << instructionsOfType[(int)InstructionType::Branch] << endl;
	cout << "Arithmetic: " << instructionsOfType[(int)InstructionType::Arithmetic] << endl;
	cout << "Other:      " << instructionsOfType[(int)InstructionType::Other] << endl;
	cout << "Total:      " << totalInstructions << endl << endl;

	cout << "Load-use Hazards: " << loadUseHazards << endl;
	cout << "Load-use Hazards/Total Loads: " << setprecision(3) << ((double)loadUseHazards / (double)instructionsOfType[(int)InstructionType::Load]) << endl << endl;

	cout << "Required data forwards: " << dataForwards << endl;
	cout << "Data forwards/Total instructions: " << setprecision(3) << ((double)dataForwards / (double)totalInstructions) << endl << endl;

	cout << "Successful static branch predictions: " << successfulBranchPredictions << endl;
	cout << "Successful branch predictions/Total branches: " << setprecision(3) << ((double)successfulBranchPredictions / (double)instructionsOfType[(int)InstructionType::Branch]) << endl << endl;

	cout << "Cache Structure" << endl;
	cout << "Block Count: " << blockCount << "    Block Size (bytes): " << blockSize * 4 << endl;
	cout << "Associativity: " << associativity << endl << endl;

	cout << "Cache Hits: " << cache_hits << endl;
	cout << "Cache Misses: " << cache_misses << endl << endl;

	cout << "Performance" << endl;
	cout << "Total clock cycles = " << setw(9) << totalInstructions << " (total instructions)" << endl;
	cout << "                     + " << setw(7) << loadUseHazards << " (bubbles from load-use hazards)" << endl;
	cout << "                     + " << setw(7) << 50 * cache_misses << " (50 x cache misses)" << endl;
	cout << "                   = " << setw(9) << totalInstructions + loadUseHazards + (50 * cache_misses) << " * " << 1 + associativity_penalty << " (associativity penalty)" << endl;
	cout << "                   = " << setw(9) << (int)((totalInstructions + loadUseHazards + (50 * cache_misses)) * (1 + associativity_penalty)) << endl;

	return 0;

}

void checkLoadUseHazard(Instruction currInstr, Instruction prevInstr)
{
	if (prevInstr.type == InstructionType::Load)
	{
		if (prevInstr.rd == currInstr.rs1 || prevInstr.rd == currInstr.rs2)
		{
			loadUseHazards++;
		}
	}
}

void checkForwarding(Instruction currInstr, Instruction prevInstr, Instruction prevPrevInstr)
{
	// Exe -> Exe, Mem -> Exe
	if ((prevInstr.rd == currInstr.rs1 && currInstr.rs1 != 0) || (prevInstr.rd == currInstr.rs2 && currInstr.rs2 != 0))
	{
		dataForwards++;
	}

	if ((prevPrevInstr.rd == currInstr.rs1 && currInstr.rs1 != 0) || (prevPrevInstr.rd == currInstr.rs2 && currInstr.rs2 != 0))
	{
		dataForwards++;
	}

}

void checkStaticBranchPrediction(Instruction currInstr, Instruction prevInstr)
{
	if (prevInstr.type == InstructionType::Branch)
	{
		// Jumping backwards
		if (prevInstr.signbit == 1)
		{
			if (currInstr.address < prevInstr.address) { successfulBranchPredictions++; }
		}
		// Jumping forwards
		else
		{
			if (currInstr.address > prevInstr.address) { successfulBranchPredictions++; }
		}
	}
}