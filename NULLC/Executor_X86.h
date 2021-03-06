#pragma once

#include "stdafx.h"

#include "Executor_Common.h"
#include "InstructionSet.h"
#include "Instruction_X86.h"
#include "Output.h"

typedef struct DCCallVM_ DCCallVM;

class ExecutorX86
{
public:
	ExecutorX86(Linker *linker);
	~ExecutorX86();

	bool	Initialize();

	void	ClearNative();
	bool	TranslateToNative(bool enableLogFiles, OutputContext &output);
	void	SaveListing(OutputContext &output);

	void	Run(unsigned int functionID, const char *arguments);
	void	Stop(const char* error);

	const char*	GetResult();
	int			GetResultInt();
	double		GetResultDouble();
	long long	GetResultLong();

	const char*	GetExecError();

	char*	GetVariableData(unsigned int *count);

	void			BeginCallStack();
	unsigned int	GetNextAddress();

	void*			GetStackStart();
	void*			GetStackEnd();

	void	SetBreakFunction(void *context, unsigned (*callback)(void*, unsigned));
	void	ClearBreakpoints();
	bool	AddBreakpoint(unsigned int instruction, bool oneHit);
	bool	RemoveBreakpoint(unsigned int instruction);

	bool	SetStackPlacement(void* start, void* end, unsigned int flagMemoryAllocated);
private:
	void	InitExecution();

	bool	codeRunning;

	char	execError[512];
	char	execResult[64];

	Linker		*exLinker;

	FastVector<ExternTypeInfo>	&exTypes;
	FastVector<ExternFuncInfo>	&exFunctions;
	FastVector<VMCmd>			&exCode;
	FastVector<bool>			codeJumpTargets;

	FastVector<x86Instruction, true, true>	instList;

	unsigned int		globalStartInBytecode;

	char			*paramBase;
	void			*genStackTop, *genStackPtr;

	unsigned char	*binCode;
	unsigned int	binCodeStart;
	unsigned int	binCodeSize, binCodeReserved;

	unsigned int	lastInstructionCount;

	int				callContinue;

	unsigned int	*callstackTop;

	unsigned int	oldJumpTargetCount;
	unsigned int	oldFunctionSize;

	unsigned int	oldCodeHeadProtect;
	unsigned int	oldCodeBodyProtect;

	DCCallVM		*dcCallVM;

public:
	FastVector<unsigned char*>	instAddress;

	void *breakFunctionContext;
	unsigned (*breakFunction)(void*, unsigned);

	struct Breakpoint
	{
		Breakpoint(): instIndex(0), oldOpcode(0){}
		Breakpoint(unsigned int instIndex, unsigned char oldOpcode, bool oneHit): instIndex(instIndex), oldOpcode(oldOpcode), oneHit(oneHit){}
		unsigned int	instIndex;
		unsigned char	oldOpcode;
		bool			oneHit;
	};
	FastVector<Breakpoint>		breakInstructions;

	FastVector<unsigned int>	functionAddress;
	struct FunctionListInfo
	{
		FunctionListInfo(): list(NULL), count(0){}
		FunctionListInfo(unsigned *list, unsigned count): list(list), count(count){}
		unsigned	*list;
		unsigned	count;
	};
	FastVector<FunctionListInfo>	oldFunctionLists;

private:
	void operator=(ExecutorX86& r){ (void)r; assert(false); }
};
