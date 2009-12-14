#pragma once
#include "stdafx.h"
#include "InstructionSet.h"

void CallbackInitialize();

void SetTypeConst(bool isConst);
void SetCurrentAlignment(unsigned int alignment);

// Function is called when block {} is opened, to save the number of defined variables and functions
void BeginBlock();
// Function is called when block {} is closed, to restore previous number of defined variables and functions to hide those that lost visibility
void EndBlock(bool hideFunctions = true);

// Functions for adding nodes with constant numbers of different types
void AddNumberNodeChar(const char* pos);
void AddNumberNodeInt(const char* pos);
void AddNumberNodeFloat(const char* pos);
void AddNumberNodeLong(const char* pos, const char* end);
void AddNumberNodeDouble(const char* pos);

void AddVoidNode();

void AddHexInteger(const char* pos, const char* end);
void AddOctInteger(const char* pos, const char* end);
void AddBinInteger(const char* pos, const char* end);

// Function that places string on stack, using list of NodeNumber in NodeExpressionList
void AddStringNode(const char* s, const char* e);

// Function that creates node that removes value on top of the stack
void AddPopNode(const char* pos);

// Function that creates unary operation node that changes sign of value
void AddNegateNode(const char* pos);

// Function that creates unary operation node for logical NOT
void AddLogNotNode(const char* pos);
// Function that creates unary operation node for binary NOT
void AddBitNotNode(const char* pos);

void AddBinaryCommandNode(const char* pos, CmdID id);

void AddReturnNode(const char* pos);
void AddBreakNode(const char* pos);
void AddContinueNode(const char* pos);

void SelectTypeByPointer(void* type);
void SelectTypeByIndex(unsigned int index);
void* GetSelectedType();

void AddVariable(const char* pos, InplaceStr varName);

void AddVariableReserveNode(const char* pos);

void ConvertTypeToReference(const char* pos);

void ConvertTypeToArray(const char* pos);

void GetTypeSize(const char* pos, bool sizeOfExpr);

void SetTypeOfLastNode();

// Function that retrieves variable address
void AddGetAddressNode(const char* pos, InplaceStr varName, bool preferLastFunction = false);

// Function for array indexing
void AddArrayIndexNode(const char* pos);

// Function for pointer dereferencing
void AddDereferenceNode(const char* pos);

// Function for variable assignment in place of definition
void AddDefineVariableNode(const char* pos, InplaceStr varName);

void AddSetVariableNode(const char* pos);

void AddGetVariableNode(const char* pos);
void AddMemberAccessNode(const char* pos, InplaceStr varName);

void AddPreOrPostOpNode(const char* pos, bool isInc, bool prefixOp);

void AddModifyVariableNode(const char* pos, CmdID cmd);

void AddOneExpressionNode(void *retType = NULL);
void AddTwoExpressionNode(void *retType = NULL);

void AddArrayConstructor(const char* pos, unsigned int arrElementCount);

void AddTypeAllocation(const char* pos);

void FunctionAdd(const char* pos, const char* funcName);
void FunctionParameter(const char* pos, InplaceStr paramName);
void FunctionPrototype(const char* pos);
void FunctionStart(const char* pos);
void FunctionEnd(const char* pos, const char* funcName);
void FunctionToOperator(const char* pos);

bool AddFunctionCallNode(const char* pos, const char* funcName, unsigned int callArgCount, bool silent = false);
void AddMemberFunctionCall(const char* pos, const char* funcName, unsigned int callArgCount);

void AddIfNode(const char* pos);
void AddIfElseNode(const char* pos);
void AddIfElseTermNode(const char* pos);

void IncreaseCycleDepth();

void AddForNode(const char* pos);
void AddWhileNode(const char* pos);
void AddDoWhileNode(const char* pos);

void BeginSwitch(const char* pos);
void AddCaseNode(const char* pos);
void AddDefaultNode();
void EndSwitch();

void TypeBegin(const char* pos, const char* end);
void TypeAddMember(const char* pos, const char* varName);
void TypeFinish();
void TypeContinue(const char* pos);
void TypeStop();

void AddAliasType(InplaceStr aliasName);

void AddUnfixedArraySize();

unsigned int GetGlobalSize();
void CallbackReset();
