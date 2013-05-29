/*
 *  Interpreter.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 25.04.10
 *  Copyright 2010 RWTH Aachen University All rights reserved.
 *
 */

#include <vector>
#include <stdint.h>

class RXEFile;
class System;
class VMMemory;

/*!
 * @abstract Runs the bytecode.
 * @coclass RXEFile Contains the memory start information and the code that is
 * being executed.
 * @coclass System Calls into system for IO and communicating with the robot that
 * this program is running on.
 * @coclass VMMemory Manages the data the program is operating on.
 * @discussion The Interpreter ultimately executes all bytecode. It has only one
 * interesting public method, step, which executes a single operation. However,
 * it has a large number of private methods, one for each operation that exists.
 * Not all of these are implemented, many simply print out their name and that
 * they are not yet supported.
 */
class Interpreter
{
	const RXEFile *file;
	VMMemory *memory;
	System *system;
	
	std::vector<unsigned> stack;
	unsigned instruction;
	unsigned currentClump;
	
	// Required for waiting
	unsigned waitUntil;
	
	// Helpers used internally by other ops
	bool compare(unsigned mode, int a, int b);
	void configureOutputForPort(unsigned port, unsigned numParams, const uint16_t *params);
	
	// Math Instructions
	void op_add(unsigned flags, const uint16_t *params);
	void op_sub(unsigned flags, const uint16_t *params);
	void op_neg(unsigned flags, const uint16_t *params);
	void op_mul(unsigned flags, const uint16_t *params);
	void op_div(unsigned flags, const uint16_t *params);
	void op_mod(unsigned flags, const uint16_t *params);
	void op_sqrt(unsigned flags, const uint16_t *params);
	void op_abs(unsigned flags, const uint16_t *params);
	
	// Logical Instructions
	void op_and(unsigned flags, const uint16_t *params);
	void op_or(unsigned flags, const uint16_t *params);
	void op_xor(unsigned flags, const uint16_t *params);
	void op_not(unsigned flags, const uint16_t *params);
	
	// Compare Instructions
	void op_cmp(unsigned flags, const uint16_t *params);
	void op_tst(unsigned flags, const uint16_t *params);
	
	// Data manipulation Instructions
	void op_index(unsigned flags, const uint16_t *params);
	void op_replace(unsigned flags, const uint16_t *params);
	void op_arrsize(unsigned flags, const uint16_t *params);
	void op_arrbuild(unsigned flags, const uint16_t *params);
	void op_arrsubset(unsigned flags, const uint16_t *params);
	void op_arrinit(unsigned flags, const uint16_t *params);
	void op_mov(unsigned flags, const uint16_t *params);
	void op_set(unsigned flags, const uint16_t *params);
	void op_flatten(unsigned flags, const uint16_t *params);
	void op_unflatten(unsigned flags, const uint16_t *params);
	void op_numtostring(unsigned flags, const uint16_t *params);
	void op_stringtonum(unsigned flags, const uint16_t *params);
	void op_strcat(unsigned flags, const uint16_t *params);
	void op_strsubset(unsigned flags, const uint16_t *params);
	void op_strtobytearr(unsigned flags, const uint16_t *params);
	void op_bytearrtostr(unsigned flags, const uint16_t *params);
	
	// Control flow Instructions
	void op_jmp(unsigned flags, const uint16_t *params);
	void op_brcmp(unsigned flags, const uint16_t *params);
	void op_brtst(unsigned flags, const uint16_t *params);
	void op_stop(unsigned flags, const uint16_t *params);
	void op_finclump(unsigned flags, const uint16_t *params);
	void op_finclumpimmed(unsigned flags, const uint16_t *params);
	void op_acquire(unsigned flags, const uint16_t *params);
	void op_release(unsigned flags, const uint16_t *params);
	void op_subcall(unsigned flags, const uint16_t *params);
	void op_subret(unsigned flags, const uint16_t *params);
	
	// System I/O instructions
	void op_syscall(unsigned flags, const uint16_t *params);
	void op_setin(unsigned flags, const uint16_t *params);
	void op_setout(unsigned flags, const uint16_t *params);
	void op_getin(unsigned flags, const uint16_t *params);
	void op_getout(unsigned flags, const uint16_t *params);
	void op_wait(unsigned flags, const uint16_t *params);
	void op_gettick(unsigned flags, const uint16_t *params);
	
public:
	/*!
	 * @abstract Constructs an Interpreter.
	 * @discussion Execution always starts at the first clump, based on the
	 * order in the clump data array in the RXE file. The interpreter does not
	 * take ownership of any of the objects passed into it, they all have to be
	 * deleted manually after the interpreter is deleted.
	 * @param file The RXE file to execute.
	 * @param memory A memory object, which has to have been created with the
	 * same RXE file.
	 * @param system The System interface used for IO and syscalls.
	 */
	Interpreter(const RXEFile *file, VMMemory *memory, System *system);
	
	/*!
	 * @abstract Executes a single operation.
	 */
	void step();
	
	/*!
	 * @abstract Debug method: Returns the name for an opcode.
	 * @discussion The returned name is taken directly from the official
	 * documentation. Returns "OP_UNKNOWN" for opcode values that do not exist.
	 * This method is mainly meant for debugging purposes.
	 * @param opcode The opcode of an instruction.
	 * @result The name of the instruction per the programming manual, or
	 * OP_UNKNOWN if no such instruction exists.
	 */
	static const char *nameForOpcode(unsigned opcode);
	
	/*!
	 * @abstract Tick at which the robot will next do something.
	 * @discussion If the robot is in wait mode, it will do nothing and no
	 * operation will be executed. With this, users can find out how long it
	 * will be until it operates again.
	 */
	unsigned waitingUntilTick() const { return waitUntil; }
};
