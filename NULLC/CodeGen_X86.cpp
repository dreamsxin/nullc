#include "CodeGen_X86.h"

//#undef NULLC_OPTIMIZE_X86

#ifdef NULLC_BUILD_X86_JIT

#include "Executor_Common.h"
#include "StdLib.h"

#ifdef NULLC_OPTIMIZE_X86
namespace NULLC
{
	unsigned lastInvalidate = 0;

	x86Argument genReg[rRegCount];		// Holds current register value
	unsigned genRegUpdate[rRegCount];	// Marks the last instruction that wrote to the register
	bool genRegRead[rRegCount];			// Marks if there was a read from the register after last write

	x86Argument xmmReg[rXmmRegCount];		// Holds current register value
	unsigned xmmRegUpdate[rXmmRegCount];	// Marks the last instruction that wrote to the register
	bool xmmRegRead[rXmmRegCount];			// Marks if there was a read from the register after last write

	struct MemCache
	{
		x86Argument	address;
		x86Argument value;
	};

	const unsigned memoryStateSize = 16;
	MemCache memCache[memoryStateSize];
	unsigned memCacheEntries = 0;

	unsigned MemFind(const x86Argument &address)
	{
		for(unsigned i = 0; i < memoryStateSize; i++)
		{
			MemCache &entry = memCache[i];

			if(entry.value.type != x86Argument::argNone && entry.address.type != x86Argument::argNone &&
				entry.address.ptrSize == address.ptrSize && entry.address.ptrNum == address.ptrNum &&
				entry.address.ptrBase == address.ptrBase && entry.address.ptrIndex == address.ptrIndex)
			{
				return i + 1;
			}
		}
		return 0;
	}

	void MemWrite(const x86Argument &address, const x86Argument &value)
	{
		if(unsigned index = MemFind(address))
		{
			index--;

			if(index != 0)
			{
				MemCache tmp = memCache[index - 1];

				memCache[index - 1] = memCache[index];
				memCache[index] = tmp;
				memCache[index - 1].value = value;
			}
			else
			{
				memCache[0].value = value;
			}
		}
		else
		{
			unsigned newIndex = memCacheEntries < memoryStateSize ? memCacheEntries : memoryStateSize - 1;

			if(memCacheEntries < memoryStateSize)
				memCacheEntries++;
			else
				memCacheEntries = memoryStateSize >> 1;	// Wrap to the middle

			memCache[newIndex].address = address;
			memCache[newIndex].value = value;
		}
	}

	void MemUpdate(unsigned index)
	{
		if(index != 0)
		{
			MemCache tmp = memCache[index - 1];
			memCache[index - 1] = memCache[index];
			memCache[index] = tmp;
		}
	}

	void InvalidateState()
	{
		for(unsigned i = 0; i < rRegCount; i++)
			genReg[i].type = x86Argument::argNone;

		for(unsigned i = 0; i < rXmmRegCount; i++)
			xmmReg[i].type = x86Argument::argNone;

		for(unsigned i = 0; i < memoryStateSize; i++)
			memCache[i].address.type = x86Argument::argNone;

		memCacheEntries = 0;
	}

	void InvalidateDependand(x86Reg dreg)
	{
		for(unsigned i = 0; i < rRegCount; i++)
		{
			if(genReg[i].type == x86Argument::argReg && genReg[i].reg == dreg)
				genReg[i].type = x86Argument::argPtrLabel;

			if(genReg[i].type == x86Argument::argPtr && (genReg[i].ptrBase == dreg || genReg[i].ptrIndex == dreg))
				genReg[i].type = x86Argument::argPtrLabel;
		}

		for(unsigned i = 0; i < memoryStateSize; i++)
		{
			MemCache &entry = memCache[i];

			if(entry.address.type == x86Argument::argReg && entry.address.reg == dreg)
				entry.address.type = x86Argument::argNone;

			if(entry.address.type == x86Argument::argPtr && (entry.address.ptrBase == dreg || entry.address.ptrIndex == dreg))
				entry.address.type = x86Argument::argNone;

			if(entry.value.type == x86Argument::argReg && entry.value.reg == dreg)
				entry.value.type = x86Argument::argNone;

			if(entry.value.type == x86Argument::argPtr && (entry.value.ptrBase == dreg || entry.value.ptrIndex == dreg))
				entry.value.type = x86Argument::argNone;
		}
	}

	void InvalidateDependand(x86XmmReg dreg)
	{
		for(unsigned i = 0; i < rXmmRegCount; i++)
		{
			if(xmmReg[i].type == x86Argument::argReg && xmmReg[i].xmmArg == dreg)
				xmmReg[i].type = x86Argument::argPtrLabel;
		}

		for(unsigned i = 0; i < memoryStateSize; i++)
		{
			MemCache &entry = memCache[i];

			if(entry.value.type == x86Argument::argXmmReg && entry.value.xmmArg == dreg)
				entry.value.type = x86Argument::argNone;
		}
	}

	void InvalidateAddressValue(CodeGenGenericContext &ctx, x86Argument arg)
	{
		// TODO: it's hard to say which registers might alias, so we invalidate all
		(void)arg;

		for(unsigned i = 0; i < rRegCount; i++)
		{
			if(genReg[i].type == x86Argument::argPtr)
				genReg[i].type = x86Argument::argNone;
		}

		for(unsigned i = 0; i < rXmmRegCount; i++)
		{
			if(xmmReg[i].type == x86Argument::argPtr)
				xmmReg[i].type = x86Argument::argNone;
		}
	}

	void KillUnreadRegisters(CodeGenGenericContext &ctx)
	{
		for(unsigned i = 0; i < rRegCount; i++)
			EMIT_REG_KILL(ctx, x86Reg(i));

		for(unsigned i = 0; i < rXmmRegCount; i++)
			EMIT_REG_KILL(ctx, x86XmmReg(i));
	}

	void ReadRegister(CodeGenGenericContext &ctx, x86Reg reg)
	{
		genRegRead[reg] = true;
	}

	void ReadRegister(CodeGenGenericContext &ctx, x86XmmReg reg)
	{
		xmmRegRead[reg] = true;
	}

	void OverwriteRegisterWithValue(CodeGenGenericContext &ctx, x86Reg reg, x86Argument arg)
	{
		// Destination is updated
		EMIT_REG_KILL(ctx, reg);
		InvalidateDependand(reg);

		genReg[reg] = arg;
		genRegUpdate[reg] = unsigned(ctx.x86Op - ctx.x86Base);
		genRegRead[reg] = false;
	}

	void OverwriteRegisterWithUnknown(CodeGenGenericContext &ctx, x86Reg reg)
	{
		// Destination is updated
		EMIT_REG_KILL(ctx, reg);
		InvalidateDependand(reg);

		genReg[reg].type = x86Argument::argNone;
		genRegUpdate[reg] = unsigned(ctx.x86Op - ctx.x86Base);
		genRegRead[reg] = false;
	}

	void OverwriteRegisterWithValue(CodeGenGenericContext &ctx, x86XmmReg reg, x86Argument arg)
	{
		// Destination is updated
		EMIT_REG_KILL(ctx, reg);
		InvalidateDependand(reg);

		xmmReg[reg] = arg;
		xmmRegUpdate[reg] = unsigned(ctx.x86Op - ctx.x86Base);
		xmmRegRead[reg] = false;
	}

	void OverwriteRegisterWithUnknown(CodeGenGenericContext &ctx, x86XmmReg reg)
	{
		// Destination is updated
		EMIT_REG_KILL(ctx, reg);
		InvalidateDependand(reg);

		xmmReg[reg].type = x86Argument::argNone;
		xmmRegUpdate[reg] = unsigned(ctx.x86Op - ctx.x86Base);
		xmmRegRead[reg] = false;
	}

	void ReadAndModifyRegister(CodeGenGenericContext &ctx, x86Reg reg)
	{
		NULLC::InvalidateDependand(reg);

		genReg[reg].type = x86Argument::argNone;
		genRegUpdate[reg] = unsigned(ctx.x86Op - ctx.x86Base);
		genRegRead[reg] = false;
	}

	void ReadAndModifyRegister(CodeGenGenericContext &ctx, x86XmmReg reg)
	{
		NULLC::InvalidateDependand(reg);

		xmmReg[reg].type = x86Argument::argNone;
		xmmRegUpdate[reg] = unsigned(ctx.x86Op - ctx.x86Base);
		xmmRegRead[reg] = false;
	}

	void RedirectAddressComputation(x86Reg &index, int &multiplier, x86Reg &base, unsigned &shift)
	{
		// If selected base register contains the value of another register, use it instead
		if(genReg[base].type == x86Argument::argReg)
		{
			base = NULLC::genReg[base].reg;
		}

		// If the base register contains a known value, add it to the offset and don't use the base register
		if(genReg[base].type == x86Argument::argNumber)
		{
			shift += NULLC::genReg[base].num;
			base = rNONE;
		}

		// If the index register contains a known value, add it to the offset and don't use the index register
		if(genReg[index].type == x86Argument::argNumber)
		{
			shift += NULLC::genReg[index].num * multiplier;
			multiplier = 1;
			index = rNONE;
		}
	}
}
#endif

void EMIT_COMMENT(CodeGenGenericContext &ctx, const char* text)
{
#if !defined(NDEBUG)
	ctx.x86Op->name = o_other;
	ctx.x86Op->comment = text;
	ctx.x86Op++;
#else
	(void)text;
#endif
}

void EMIT_LABEL(CodeGenGenericContext &ctx, unsigned labelID, int invalidate)
{
#ifdef NULLC_OPTIMIZE_X86
	if(invalidate)
		NULLC::InvalidateState();
#else
	(void)invalidate;
#endif

	ctx.x86Op->name = o_label;
	ctx.x86Op->labelID = labelID;
	ctx.x86Op->argA.type = x86Argument::argNone;
	ctx.x86Op->argA.num = invalidate;
	ctx.x86Op->argB.type = x86Argument::argNone;
	ctx.x86Op++;
}

void EMIT_OP(CodeGenGenericContext &ctx, x86Command op)
{
#ifdef NULLC_OPTIMIZE_X86
	switch(op)
	{
	case o_ret:
		NULLC::InvalidateState();

		// Mark return registers as implicitly used
		NULLC::ReadRegister(ctx, rEAX);
		NULLC::ReadRegister(ctx, rEDX);
		break;
	case o_rep_movsd:
		assert(NULLC::genReg[rECX].type == x86Argument::argNumber);

		NULLC::InvalidateState();

		// Implicit register reads
		NULLC::ReadRegister(ctx, rECX);
		NULLC::ReadRegister(ctx, rESI);
		NULLC::ReadRegister(ctx, rEDI);
		break;
	case o_use32:
		break;
	default:
		assert(!"unknown instruction");
	}

	/*if(op == o_cdq)
	{
		if(NULLC::reg[rEAX].type == x86Argument::argNumber)
		{
			EMIT_OP_REG_NUM(ctx, o_mov, rEDX, NULLC::reg[rEAX].num >= 0 ? 0 : ~0u);
			return;
		}
		NULLC::regRead[rEAX] = true;
		EMIT_REG_KILL(ctx, rEDX);
		NULLC::InvalidateDependand(rEDX);
		NULLC::reg[rEDX].type = x86Argument::argNone;
		NULLC::regRead[rEDX] = false;
		NULLC::regUpdate[rEDX] = unsigned(ctx.x86Op - ctx.x86Base);
	}*/
#endif

	ctx.x86Op->name = op;
	ctx.x86Op->argA.type = x86Argument::argNone;
	ctx.x86Op->argB.type = x86Argument::argNone;
	ctx.x86Op++;
}

void EMIT_OP_LABEL(CodeGenGenericContext &ctx, x86Command op, unsigned labelID, int invalidate, int longJump)
{
#ifdef NULLC_OPTIMIZE_X86
	switch(op)
	{
	case o_jmp:
	case o_ja:
	case o_jae:
	case o_jb:
	case o_jbe:
	case o_je:
	case o_jg:
	case o_jl:
	case o_jne:
	case o_jnp:
	case o_jp:
	case o_jge:
	case o_jle:
		if(invalidate)
		{
			if(longJump)
				NULLC::KillUnreadRegisters(ctx);

			NULLC::InvalidateState();
		}
		break;
	case o_call:
		NULLC::KillUnreadRegisters(ctx);

		NULLC::InvalidateState();
		break;
	default:
		assert(!"unknown instruction");
	}

	// Optimize conditional jumps
	/*if((ctx.x86Op - ctx.x86Base - 2) > (int)NULLC::lastInvalidate && ctx.x86Op[-2].name >= o_setl && ctx.x86Op[-2].name <= o_setnz && ctx.x86Op[-1].name == o_test &&
		ctx.x86Op[-2].argA.reg == ctx.x86Op[-1].argA.reg && ctx.x86Op[-1].argA.reg == ctx.x86Op[-1].argB.reg)
	{
		if(op == o_jz)
		{
			static const x86Command jump[] = { o_jge, o_jle, o_jg, o_jl, o_jne, o_je, o_jnz, o_jz };
			x86Command curr = jump[ctx.x86Op[-2].name - o_setl];
			if(ctx.x86Op[-4].name == o_xor && ctx.x86Op[-4].argA.reg == ctx.x86Op[-4].argB.reg)
				ctx.x86Op[-4].name = o_none;
			ctx.x86Op[-2].name = o_none;
			ctx.x86Op[-1].name = o_none;
			ctx.optimizationCount += 2;
			EMIT_OP_LABEL(ctx, curr, labelID, invalidate, longJump);
			return;
		}
		else if(op == o_jnz)
		{
			static const x86Command jump[] = { o_jl, o_jg, o_jle, o_jge, o_je, o_jne, o_jz, o_jnz };
			x86Command curr = jump[ctx.x86Op[-2].name - o_setl];
			if(ctx.x86Op[-4].name == o_xor && ctx.x86Op[-4].argA.reg == ctx.x86Op[-4].argB.reg)
				ctx.x86Op[-4].name = o_none;
			ctx.x86Op[-2].name = o_none;
			ctx.x86Op[-1].name = o_none;
			ctx.optimizationCount += 2;
			EMIT_OP_LABEL(ctx, curr, labelID, invalidate, longJump);
			return;
		}
	}*/
#else
	(void)invalidate;
	(void)longJump;
#endif

	ctx.x86Op->name = op;
	ctx.x86Op->argA.type = x86Argument::argLabel;
	ctx.x86Op->argA.labelID = labelID;
	ctx.x86Op->argB.type = x86Argument::argNone;
	ctx.x86Op->argB.num = invalidate;
	ctx.x86Op->argB.ptrNum = longJump;
	ctx.x86Op++;
}

void EMIT_OP_REG(CodeGenGenericContext &ctx, x86Command op, x86Reg reg1)
{
#ifdef NULLC_OPTIMIZE_X86
	switch(op)
	{
	case o_call:
		NULLC::ReadRegister(ctx, reg1);

		NULLC::KillUnreadRegisters(ctx);

		NULLC::InvalidateState();
		break;
	case o_setl:
	case o_setg:
	case o_setle:
	case o_setge:
	case o_sete:
	case o_setne:
	case o_setz:
	case o_setnz:
		NULLC::ReadAndModifyRegister(ctx, reg1); // Since instruction sets only lower bits, it 'reads' the rest
		break;
	default:
		assert(!"unknown instruction");
	}

	// Constant propagation
	/*if(op == o_neg && NULLC::reg[reg1].type == x86Argument::argNumber)
	{
		EMIT_OP_REG_NUM(ctx, o_mov, reg1, -NULLC::reg[reg1].num);
		return;
	}

	// Constant propagation
	if(op == o_not && NULLC::reg[reg1].type == x86Argument::argNumber)
	{
		EMIT_OP_REG_NUM(ctx, o_mov, reg1, ~NULLC::reg[reg1].num);
		return;
	}

	// Constant propagation
	if(op == o_idiv && NULLC::reg[rEAX].type == x86Argument::argNumber && NULLC::reg[reg1].type == x86Argument::argNumber && NULLC::reg[reg1].num)
	{
		EMIT_OP_REG_NUM(ctx, o_mov, rEDX, NULLC::reg[rEAX].num % NULLC::reg[reg1].num);
		EMIT_OP_REG_NUM(ctx, o_mov, rEAX, NULLC::reg[rEAX].num / NULLC::reg[reg1].num);
		return;
	}*/

	/*
	{
		NULLC::InvalidateDependand(reg1);
	}*/

	/*
	// Check if an operation with memory directly is possible
	if(op == o_imul || op == o_idiv)
	{
		NULLC::InvalidateDependand(rEAX);
		NULLC::reg[rEAX].type = x86Argument::argNone;
		NULLC::InvalidateDependand(rEDX);
		NULLC::reg[rEDX].type = x86Argument::argNone;
		if(op == o_idiv && NULLC::reg[reg1].type == x86Argument::argPtr)
		{
			EMIT_OP_RPTR(ctx, o_idiv, NULLC::reg[reg1].ptrSize, NULLC::reg[reg1].ptrBase, NULLC::reg[reg1].ptrNum);
			return;
		}
	}
	else
	{
		NULLC::regUpdate[reg1] = unsigned(ctx.x86Op - ctx.x86Base);
	}*/

	/*if(op == o_neg || op == o_not)
		NULLC::reg[reg1].type = x86Argument::argNone;
	NULLC::regRead[reg1] = (op == o_push || op == o_imul || op == o_idiv || op == o_neg || op == o_not || op == o_nop);

#ifdef _DEBUG
	if(op != o_push && op != o_pop && op != o_call && op != o_imul && op < o_setl && op > o_setnz)
		assert(!"invalid instruction");
#endif*/

#endif

	ctx.x86Op->name = op;
	ctx.x86Op->argA.type = x86Argument::argReg;
	ctx.x86Op->argA.reg = reg1;
	ctx.x86Op->argB.type = x86Argument::argNone;
	ctx.x86Op++;
}

void EMIT_OP_NUM(CodeGenGenericContext &ctx, x86Command op, unsigned num)
{
#ifdef NULLC_OPTIMIZE_X86
	switch(op)
	{
	default:
		assert(!"unknown instruction");
	}

	/*
	if(op == o_int)
		NULLC::regRead[rECX] = true;

#ifdef _DEBUG
	if(op != o_push && op != o_int)
		assert(!"invalid instruction");
#endif*/

#endif

	ctx.x86Op->name = op;
	ctx.x86Op->argA.type = x86Argument::argNumber;
	ctx.x86Op->argA.num = num;
	ctx.x86Op->argB.type = x86Argument::argNone;
	ctx.x86Op++;
}

void EMIT_OP_RPTR(CodeGenGenericContext &ctx, x86Command op, x86Size size, x86Reg index, int multiplier, x86Reg base, unsigned shift)
{
#ifdef NULLC_OPTIMIZE_X86
	NULLC::RedirectAddressComputation(index, multiplier, base, shift);

	// Register reads
	NULLC::ReadRegister(ctx, base);
	NULLC::ReadRegister(ctx, index);

	switch(op)
	{
	default:
		assert(!"unknown instruction");
	}
/*
	if(op != o_push && index == rNONE && NULLC::reg[base].type == x86Argument::argPtr && NULLC::reg[base].ptrSize == sNONE)
	{
		EMIT_OP_RPTR(ctx, op, size, NULLC::reg[base].ptrIndex, NULLC::reg[base].ptrMult, NULLC::reg[base].ptrBase, NULLC::reg[base].ptrNum + shift);
		return;
	}

	if(op == o_idiv || op == o_imul)
	{
		// still invalidate eax and edx
		NULLC::InvalidateDependand(rEAX);
		NULLC::reg[rEAX].type = x86Argument::argNone;
		NULLC::InvalidateDependand(rEDX);
		NULLC::reg[rEDX].type = x86Argument::argNone;
	}
	
	NULLC::regRead[index] = true;
	NULLC::regRead[base] = true;

	if(op == o_call)
	{
		EMIT_REG_KILL(ctx, rEAX);
		EMIT_REG_KILL(ctx, rEBX);
		EMIT_REG_KILL(ctx, rECX);
		EMIT_REG_KILL(ctx, rEDX);
		EMIT_REG_KILL(ctx, rESI);
		NULLC::InvalidateState();
	}

#ifdef _DEBUG
	if(op != o_push && op != o_pop && op != o_neg && op != o_not && op != o_idiv && op != o_call && op != o_jmp)
		assert(!"invalid instruction");
#endif*/

#endif

	ctx.x86Op->name = op;
	ctx.x86Op->argA.type = x86Argument::argPtr;
	ctx.x86Op->argA.ptrSize = size;
	ctx.x86Op->argA.ptrIndex = index;
	ctx.x86Op->argA.ptrMult = multiplier;
	ctx.x86Op->argA.ptrBase = base;
	ctx.x86Op->argA.ptrNum = shift;
	ctx.x86Op->argB.type = x86Argument::argNone;
	ctx.x86Op++;
}

void EMIT_OP_RPTR(CodeGenGenericContext &ctx, x86Command op, x86Size size, x86Reg reg2, unsigned shift)
{
	EMIT_OP_RPTR(ctx, op, size, rNONE, 1, reg2, shift);
}

void EMIT_OP_ADDR(CodeGenGenericContext &ctx, x86Command op, x86Size size, unsigned addr)
{
	EMIT_OP_RPTR(ctx, op, size, rNONE, 1, rNONE, addr);
}

void EMIT_OP_REG_NUM(CodeGenGenericContext &ctx, x86Command op, x86Reg reg1, unsigned num)
{
	if(op == o_movsx)
		op = o_mov;

#ifdef NULLC_OPTIMIZE_X86
	switch(op)
	{
	case o_mov:
		// TODO: skip move if the target already contains the same number

		NULLC::OverwriteRegisterWithValue(ctx, reg1, x86Argument(num));
		break;
	case o_add:
	case o_sub:
	case o_add64:
	case o_sub64:
		// Stack frame setup
		if(reg1 == rESP)
			break;

		NULLC::ReadAndModifyRegister(ctx, reg1);
		break;
	case o_imul:
	case o_imul64:
		NULLC::ReadAndModifyRegister(ctx, reg1);
		break;
	default:
		assert(!"unknown instruction");
	}

	/*
	// Constant propagation
	if(op == o_add && NULLC::reg[reg1].type == x86Argument::argNumber)
	{
		EMIT_OP_REG_NUM(ctx, o_mov, reg1, NULLC::reg[reg1].num + (int)num);
		return;
	}

	// Constant propagation
	if(op == o_sub && NULLC::reg[reg1].type == x86Argument::argNumber)
	{
		EMIT_OP_REG_NUM(ctx, o_mov, reg1, NULLC::reg[reg1].num - (int)num);
		return;
	}

	// Reverse instruction
	if(op == o_sub && (int)num < 0)
	{
		op = o_add;
		num = -(int)num;
	}

	// Reverse instruction
	if(op == o_add && (int)num < 0)
	{
		op = o_sub;
		num = -(int)num;
	}

	if(op == o_cmp)
	{
		if(NULLC::reg[reg1].type == x86Argument::argReg)
			reg1 = NULLC::reg[reg1].reg;
		NULLC::regRead[reg1] = true;
	}
	else
	{
		NULLC::InvalidateDependand(reg1);
		NULLC::reg[reg1].type = x86Argument::argNone;
	}

#ifdef _DEBUG
	if(op != o_add && op != o_sub && op != o_mov && op != o_test && op != o_imul && op != o_shl && op != o_cmp)
		assert(!"invalid instruction");
#endif*/

#endif

	ctx.x86Op->name = op;
	ctx.x86Op->argA.type = x86Argument::argReg;
	ctx.x86Op->argA.reg = reg1;
	ctx.x86Op->argB.type = x86Argument::argNumber;
	ctx.x86Op->argB.num = num;
	ctx.x86Op++;
}

void EMIT_OP_REG_NUM64(CodeGenGenericContext &ctx, x86Command op, x86Reg reg1, uintptr_t num)
{
#ifdef NULLC_OPTIMIZE_X86
	switch(op)
	{
	case o_mov64:
		// TODO: skip move if the target already contains the same number

		NULLC::OverwriteRegisterWithValue(ctx, reg1, x86Argument(num));
		break;
	default:
		assert(!"unknown instruction");
	}
#endif

	ctx.x86Op->name = op;
	ctx.x86Op->argA.type = x86Argument::argReg;
	ctx.x86Op->argA.reg = reg1;
	ctx.x86Op->argB.type = x86Argument::argImm64;
	ctx.x86Op->argB.imm64Arg = num;
	ctx.x86Op++;
}

void EMIT_OP_REG_REG(CodeGenGenericContext &ctx, x86Command op, x86Reg reg1, x86Reg reg2)
{
#ifdef NULLC_OPTIMIZE_X86
	switch(op)
	{
	case o_xor:
	case o_xor64:
		if(reg1 == reg2)
		{
			EMIT_REG_KILL(ctx, reg1);
			NULLC::InvalidateDependand(reg1);
		}
		else
		{
			NULLC::ReadRegister(ctx, reg2);
			NULLC::ReadAndModifyRegister(ctx, reg1);
		}
		break;
	case o_cmp:
	case o_test:
		NULLC::ReadRegister(ctx, reg1);
		NULLC::ReadRegister(ctx, reg2);
		break;
	case o_add:
	case o_sub:
	case o_add64:
	case o_sub64:
		NULLC::ReadAndModifyRegister(ctx, reg1);
		break;
	default:
		assert(!"unknown instruction");
	}
	/*
	if(op == o_xor && reg1 == reg2)
	{
		EMIT_REG_KILL(ctx, reg1);
		NULLC::InvalidateDependand(reg1);
	}
	else if(op == o_test && reg1 == reg2 && NULLC::reg[reg1].type == x86Argument::argReg)
	{
		reg1 = reg2 = NULLC::reg[reg1].reg;
	}
	else if(op != o_sal && op != o_sar)
	{
		if(NULLC::reg[reg2].type == x86Argument::argReg)
			reg2 = NULLC::reg[reg2].reg;
	}

	if(op == o_mov || op == o_movsx)
	{
		if(reg1 == reg2)
		{
			ctx.optimizationCount++;
			return;
		}
		EMIT_REG_KILL(ctx, reg1);
		NULLC::InvalidateDependand(reg1);

		NULLC::reg[reg1] = x86Argument(reg2);
		NULLC::regUpdate[reg1] = unsigned(ctx.x86Op - ctx.x86Base);
		NULLC::regRead[reg1] = false;
	}
	else if((op == o_add || op == o_sub) && NULLC::reg[reg2].type == x86Argument::argNumber)
	{
		EMIT_OP_REG_NUM(ctx, op, reg1, NULLC::reg[reg2].num);
		return;
	}
	else if(op == o_add && NULLC::reg[reg1].type == x86Argument::argNumber)
	{
		EMIT_OP_REG_RPTR(ctx, o_lea, reg1, sNONE, rNONE, 1, reg2, NULLC::reg[reg1].num);
		return;
	}
	else if(op == o_cmp)
	{
		if(NULLC::reg[reg2].type == x86Argument::argPtr && NULLC::reg[reg2].ptrSize == sDWORD)
		{
			EMIT_OP_REG_RPTR(ctx, o_cmp, reg1, sDWORD, NULLC::reg[reg2].ptrIndex, NULLC::reg[reg2].ptrMult, NULLC::reg[reg2].ptrBase, NULLC::reg[reg2].ptrNum);
			return;
		}
		if(NULLC::reg[reg1].type == x86Argument::argPtr && NULLC::reg[reg1].ptrSize == sDWORD)
		{
			EMIT_OP_RPTR_REG(ctx, o_cmp, sDWORD, NULLC::reg[reg1].ptrIndex, NULLC::reg[reg1].ptrMult, NULLC::reg[reg1].ptrBase, NULLC::reg[reg1].ptrNum, reg2);
			return;
		}
		if(NULLC::reg[reg2].type == x86Argument::argNumber)
		{
			EMIT_OP_REG_NUM(ctx, o_cmp, reg1, NULLC::reg[reg2].num);
			return;
		}

		NULLC::regRead[reg1] = true;
	}
	else if(op == o_imul)
	{
		if(NULLC::reg[reg1].type == x86Argument::argNumber && NULLC::reg[reg2].type == x86Argument::argNumber)
		{
			EMIT_OP_REG_NUM(ctx, o_mov, reg1, NULLC::reg[reg1].num * NULLC::reg[reg2].num);
			return;
		}
		if(NULLC::reg[reg2].type == x86Argument::argPtr)
		{
			EMIT_OP_REG_RPTR(ctx, o_imul, reg1, sDWORD, NULLC::reg[reg2].ptrIndex, NULLC::reg[reg2].ptrMult, NULLC::reg[reg2].ptrBase, NULLC::reg[reg2].ptrNum);
			return;
		}
		NULLC::InvalidateDependand(reg1);
		NULLC::reg[reg1].type = x86Argument::argNone;
	}
	else
	{
		NULLC::reg[reg1].type = x86Argument::argNone;
	}

	NULLC::regRead[reg2] = true;*/
#endif

	ctx.x86Op->name = op;
	ctx.x86Op->argA.type = x86Argument::argReg;
	ctx.x86Op->argA.reg = reg1;
	ctx.x86Op->argB.type = x86Argument::argReg;
	ctx.x86Op->argB.reg = reg2;
	ctx.x86Op++;
}

void EMIT_OP_REG_REG(CodeGenGenericContext &ctx, x86Command op, x86XmmReg reg1, x86XmmReg reg2)
{
#ifdef NULLC_OPTIMIZE_X86
	switch(op)
	{
	case o_movsd:
		// Check if source location contains a register
		if(NULLC::xmmReg[reg2].type == x86Argument::argXmmReg)
			reg2 = NULLC::xmmReg[reg2].xmmArg;

		// Skip self-assignment
		if(reg1 == reg2)
		{
			ctx.optimizationCount++;
			return;
		}

		NULLC::OverwriteRegisterWithValue(ctx, reg1, x86Argument(reg2));
		break;
	case o_addsd:
	case o_subsd:
	case o_mulsd:
	case o_divsd:
	case o_cmpeqsd:
	case o_cmpltsd:
	case o_cmplesd:
	case o_cmpneqsd:
		// Check if source location contains a register
		if(NULLC::xmmReg[reg2].type == x86Argument::argXmmReg)
			reg2 = NULLC::xmmReg[reg2].xmmArg;

		NULLC::ReadRegister(ctx, reg2);
		NULLC::ReadAndModifyRegister(ctx, reg1);
		break;
	default:
		assert(!"unknown instruction");
	}
#endif

	ctx.x86Op->name = op;
	ctx.x86Op->argA.type = x86Argument::argXmmReg;
	ctx.x86Op->argA.xmmArg = reg1;
	ctx.x86Op->argB.type = x86Argument::argXmmReg;
	ctx.x86Op->argB.xmmArg = reg2;
	ctx.x86Op++;
}

void EMIT_OP_REG_REG(CodeGenGenericContext &ctx, x86Command op, x86Reg reg1, x86XmmReg reg2)
{
#ifdef NULLC_OPTIMIZE_X86
	switch(op)
	{
	default:
		assert(!"unknown instruction");
	}
#endif

	ctx.x86Op->name = op;
	ctx.x86Op->argA.type = x86Argument::argReg;
	ctx.x86Op->argA.reg = reg1;
	ctx.x86Op->argB.type = x86Argument::argXmmReg;
	ctx.x86Op->argB.xmmArg = reg2;
	ctx.x86Op++;
}

void EMIT_OP_REG_RPTR(CodeGenGenericContext &ctx, x86Command op, x86Reg reg1, x86Size size, x86Reg index, int multiplier, x86Reg base, unsigned shift)
{
#ifdef NULLC_OPTIMIZE_X86
	NULLC::RedirectAddressComputation(index, multiplier, base, shift);

	// Register reads
	NULLC::ReadRegister(ctx, base);
	NULLC::ReadRegister(ctx, index);

	switch(op)
	{
	case o_mov:
	case o_movsx:
	case o_mov64:
		// If write doesn't invalidate the source registers, mark that register contains value from source address
		if(reg1 != base && reg1 != index)
			NULLC::OverwriteRegisterWithValue(ctx, reg1, x86Argument(size, index, multiplier, base, shift));
		else
			NULLC::OverwriteRegisterWithUnknown(ctx, reg1);
		break;
	case o_lea:
		NULLC::OverwriteRegisterWithUnknown(ctx, reg1);
		break;
	default:
		assert(!"unknown instruction");
	}

	/*
	if(index == rNONE && NULLC::reg[base].type == x86Argument::argPtr && NULLC::reg[base].ptrSize == sNONE)
	{
		EMIT_OP_REG_RPTR(ctx, op, reg1, size, NULLC::reg[base].ptrIndex, NULLC::reg[base].ptrMult, NULLC::reg[base].ptrBase, NULLC::reg[base].ptrNum + shift);
		return;
	}

	x86Argument newArg = x86Argument(size, index, mult, base, shift);

	if(op != o_movsx)
	{
		unsigned cIndex = NULLC::MemFind(newArg);
		if(cIndex != ~0u)
		{
			if(NULLC::mCache[cIndex].value.type == x86Argument::argNumber)
			{
				EMIT_OP_REG_NUM(ctx, op, reg1, NULLC::mCache[cIndex].value.num);
				NULLC::MemUpdate(cIndex);
				return;
			}
			if(NULLC::mCache[cIndex].value.type == x86Argument::argReg && op != o_imul && op != o_cmp)
			{
				EMIT_OP_REG_REG(ctx, op, reg1, NULLC::mCache[cIndex].value.reg);
				NULLC::MemUpdate(cIndex);
				return;
			}
		}
	}
	if(op == o_mov && base != rESP)
	{
		for(unsigned i = rEAX; i <= rEDX; i++)
		{
			if(NULLC::reg[i].type == x86Argument::argPtr && NULLC::reg[i] == newArg)
			{
				EMIT_OP_REG_REG(ctx, op, reg1, (x86Reg)i);
				return;
			}
		}
	}
#ifdef _DEBUG
	if(op != o_mov && op != o_lea && op != o_movsx && op != o_or && op != o_imul && op != o_cmp)
		assert(!"invalid instruction");
#endif

	NULLC::regRead[base] = true;
	NULLC::regRead[index] = true;

	if(op == o_mov || op == o_movsx || op == o_lea)
	{
		EMIT_REG_KILL(ctx, reg1);
		NULLC::InvalidateDependand(reg1);
	}
	if(op == o_mov || op == o_movsx || op == o_lea)
	{
		if(reg1 != base && reg1 != index)
			NULLC::reg[reg1] = newArg;
		else
			NULLC::reg[reg1].type = x86Argument::argNone;
		NULLC::regUpdate[reg1] = unsigned(ctx.x86Op - ctx.x86Base);
		NULLC::regRead[reg1] = false;
	}else if(op == o_imul){
		NULLC::reg[reg1].type = x86Argument::argNone;
	}else if(op == o_cmp || op == o_or){
		NULLC::regRead[reg1] = true;
	}*/
#endif

	ctx.x86Op->name = op;
	ctx.x86Op->argA.type = x86Argument::argReg;
	ctx.x86Op->argA.reg = reg1;
	ctx.x86Op->argB.type = x86Argument::argPtr;
	ctx.x86Op->argB.ptrSize = size;
	ctx.x86Op->argB.ptrIndex = index;
	ctx.x86Op->argB.ptrMult = multiplier;
	ctx.x86Op->argB.ptrBase = base;
	ctx.x86Op->argB.ptrNum = shift;
	ctx.x86Op++;
}

void EMIT_OP_REG_RPTR(CodeGenGenericContext &ctx, x86Command op, x86XmmReg reg1, x86Size size, x86Reg index, int multiplier, x86Reg base, unsigned shift)
{
#ifdef NULLC_OPTIMIZE_X86
	switch(op)
	{
	case o_cvtss2sd:
	case o_cvtsd2ss:
		// Register reads
		NULLC::ReadRegister(ctx, base);
		NULLC::ReadRegister(ctx, index);

		NULLC::OverwriteRegisterWithUnknown(ctx, reg1);
		break;
	case o_movss:
	case o_movsd:
		if(unsigned memIndex = NULLC::MemFind(x86Argument(size, index, multiplier, base, shift)))
		{
			memIndex--;

			if(NULLC::memCache[memIndex].value.type == x86Argument::argXmmReg)
			{
				EMIT_OP_REG_REG(ctx, op, reg1, NULLC::memCache[memIndex].value.xmmArg);
				NULLC::MemUpdate(memIndex);
				return;
			}
		}

		// Register reads
		NULLC::ReadRegister(ctx, base);
		NULLC::ReadRegister(ctx, index);

		NULLC::OverwriteRegisterWithValue(ctx, reg1, x86Argument(size, index, multiplier, base, shift));
		break;
	default:
		assert(!"unknown instruction");
	}
#endif

	ctx.x86Op->name = op;
	ctx.x86Op->argA.type = x86Argument::argXmmReg;
	ctx.x86Op->argA.xmmArg = reg1;
	ctx.x86Op->argB.type = x86Argument::argPtr;
	ctx.x86Op->argB.ptrSize = size;
	ctx.x86Op->argB.ptrIndex = index;
	ctx.x86Op->argB.ptrMult = multiplier;
	ctx.x86Op->argB.ptrBase = base;
	ctx.x86Op->argB.ptrNum = shift;
	ctx.x86Op++;
}

void EMIT_OP_REG_RPTR(CodeGenGenericContext &ctx, x86Command op, x86Reg reg1, x86Size size, x86Reg reg2, unsigned shift)
{
	EMIT_OP_REG_RPTR(ctx, op, reg1, size, rNONE, 1, reg2, shift);
}

void EMIT_OP_REG_RPTR(CodeGenGenericContext &ctx, x86Command op, x86XmmReg reg1, x86Size size, x86Reg reg2, unsigned shift)
{
	EMIT_OP_REG_RPTR(ctx, op, reg1, size, rNONE, 1, reg2, shift);
}

void EMIT_OP_REG_ADDR(CodeGenGenericContext &ctx, x86Command op, x86Reg reg1, x86Size size, unsigned addr)
{
	EMIT_OP_REG_RPTR(ctx, op, reg1, size, rNONE, 1, rNONE, addr);
}

void EMIT_OP_REG_ADDR(CodeGenGenericContext &ctx, x86Command op, x86XmmReg reg1, x86Size size, unsigned addr)
{
	EMIT_OP_REG_RPTR(ctx, op, reg1, size, rNONE, 1, rNONE, addr);
}

void EMIT_OP_REG_LABEL(CodeGenGenericContext &ctx, x86Command op, x86Reg reg1, unsigned labelID, unsigned shift)
{
#ifdef NULLC_OPTIMIZE_X86
	NULLC::InvalidateDependand(reg1);
	NULLC::genReg[reg1].type = x86Argument::argNone;
#endif

	ctx.x86Op->name = op;
	ctx.x86Op->argA.type = x86Argument::argReg;
	ctx.x86Op->argA.reg = reg1;
	ctx.x86Op->argB.type = x86Argument::argPtrLabel;
	ctx.x86Op->argB.labelID = labelID;
	ctx.x86Op->argB.ptrNum = shift;
	ctx.x86Op++;
}

void EMIT_OP_RPTR_REG(CodeGenGenericContext &ctx, x86Command op, x86Size size, x86Reg index, int multiplier, x86Reg base, unsigned shift, x86Reg reg2)
{
#ifdef NULLC_OPTIMIZE_X86
	NULLC::RedirectAddressComputation(index, multiplier, base, shift);

	// Register reads
	NULLC::ReadRegister(ctx, base);
	NULLC::ReadRegister(ctx, index);
	NULLC::ReadRegister(ctx, reg2);

	x86Argument arg = x86Argument(size, index, multiplier, base, shift);

	switch(op)
	{
	case o_mov:
	case o_mov64:
		assert(base != rESP);

		NULLC::InvalidateAddressValue(ctx, arg);

		// If the source register value was unknown, now we know that it is stored at destination memory location
		// If the source register value contained a value from an address, now we know that it contains the value from the destination address
		if(NULLC::genReg[reg2].type == x86Argument::argNone || NULLC::genReg[reg2].type == x86Argument::argPtr)
			NULLC::genReg[reg2] = arg;

		// Track target memory value
		NULLC::MemWrite(arg, x86Argument(reg2));
		break;
	default:
		assert(!"unknown instruction");
	}

	/*
	if(index == rNONE && NULLC::reg[base].type == x86Argument::argPtr && NULLC::reg[base].ptrSize == sNONE)
	{
		EMIT_OP_RPTR_REG(ctx, op, size, NULLC::reg[base].ptrIndex, NULLC::reg[base].ptrMult, NULLC::reg[base].ptrBase, NULLC::reg[base].ptrNum + shift, reg2);
		return;
	}

	if(size == sDWORD && NULLC::reg[reg2].type == x86Argument::argNumber)
	{
		EMIT_OP_RPTR_NUM(ctx, op, size, index, multiplier, base, shift, NULLC::reg[reg2].num);
		return;
	}

	if(NULLC::reg[reg2].type == x86Argument::argReg)
	{
		reg2 = NULLC::reg[reg2].reg;
	}
	x86Argument arg(size, index, multiplier, base, shift);
	if(ctx.x86LookBehind &&
		(ctx.x86Op[-1].name == o_add || ctx.x86Op[-1].name == o_sub) && ctx.x86Op[-1].argA.type == x86Argument::argReg && ctx.x86Op[-1].argA.reg == reg2 && ctx.x86Op[-1].argB.type == x86Argument::argNumber &&
		ctx.x86Op[-2].name == o_mov && ctx.x86Op[-2].argA.type == x86Argument::argReg && ctx.x86Op[-2].argA.reg == reg2 && ctx.x86Op[-2].argB == arg)
	{
		x86Command origCmd = ctx.x86Op[-1].name;
		int num = ctx.x86Op[-1].argB.num;
		ctx.x86Op -= 2;
		EMIT_OP_RPTR_NUM(ctx, origCmd, size, index, multiplier, base, shift, num);
		EMIT_OP_REG_RPTR(ctx, o_mov, reg2, size, index, multiplier, base, shift);
		return;
	}
	if(size == sDWORD && base == rESP && shift < (NULLC::STACK_STATE_SIZE * 4))
	{
		unsigned stackIndex = (16 + NULLC::stackTop - (shift >> 2)) % NULLC::STACK_STATE_SIZE;
		x86Argument &target = NULLC::stack[stackIndex];
		if(op == o_mov)
		{
			target = x86Argument(reg2);
			NULLC::stackRead[stackIndex] = false;
			NULLC::stackUpdate[stackIndex] = unsigned(ctx.x86Op - ctx.x86Base);
		}else{
			target.type = x86Argument::argNone;
		}
	}
	NULLC::regRead[reg2] = true;
	NULLC::regRead[base] = true;
	NULLC::regRead[index] = true;
*/
#endif

	ctx.x86Op->name = op;
	ctx.x86Op->argA.type = x86Argument::argPtr;
	ctx.x86Op->argA.ptrSize = size;
	ctx.x86Op->argA.ptrIndex = index;
	ctx.x86Op->argA.ptrMult = multiplier;
	ctx.x86Op->argA.ptrBase = base;
	ctx.x86Op->argA.ptrNum = shift;
	ctx.x86Op->argB.type = x86Argument::argReg;
	ctx.x86Op->argB.reg = reg2;
	ctx.x86Op++;
}

void EMIT_OP_RPTR_REG(CodeGenGenericContext &ctx, x86Command op, x86Size size, x86Reg index, int multiplier, x86Reg base, unsigned shift, x86XmmReg reg2)
{
#ifdef NULLC_OPTIMIZE_X86
	x86Argument arg = x86Argument(size, index, multiplier, base, shift);

	// Register reads
	NULLC::ReadRegister(ctx, base);
	NULLC::ReadRegister(ctx, index);
	NULLC::ReadRegister(ctx, reg2);

	switch(op)
	{
	case o_movss:
	case o_movsd:
		assert(base != rESP);

		NULLC::InvalidateAddressValue(ctx, arg);

		// If the source register value was unknown, now we know that it is stored at destination memory location
		// If the source register value contained a value from an address, now we know that it contains the value from the destination address
		if(NULLC::genReg[reg2].type == x86Argument::argNone || NULLC::genReg[reg2].type == x86Argument::argPtr)
			NULLC::genReg[reg2] = arg;

		// Track target memory value
		NULLC::MemWrite(arg, x86Argument(reg2));
		break;
	default:
		assert(!"unknown instruction");
	}
#endif

	ctx.x86Op->name = op;
	ctx.x86Op->argA.type = x86Argument::argPtr;
	ctx.x86Op->argA.ptrSize = size;
	ctx.x86Op->argA.ptrIndex = index;
	ctx.x86Op->argA.ptrMult = multiplier;
	ctx.x86Op->argA.ptrBase = base;
	ctx.x86Op->argA.ptrNum = shift;
	ctx.x86Op->argB.type = x86Argument::argXmmReg;
	ctx.x86Op->argB.xmmArg = reg2;
	ctx.x86Op++;
}

void EMIT_OP_RPTR_REG(CodeGenGenericContext &ctx, x86Command op, x86Size size, x86Reg reg1, unsigned shift, x86Reg reg2)
{
	EMIT_OP_RPTR_REG(ctx, op, size, rNONE, 1, reg1, shift, reg2);
}

void EMIT_OP_RPTR_REG(CodeGenGenericContext &ctx, x86Command op, x86Size size, x86Reg reg1, unsigned shift, x86XmmReg reg2)
{
	EMIT_OP_RPTR_REG(ctx, op, size, rNONE, 1, reg1, shift, reg2);
}

void EMIT_OP_ADDR_REG(CodeGenGenericContext &ctx, x86Command op, x86Size size, unsigned addr, x86Reg reg2)
{
	EMIT_OP_RPTR_REG(ctx, op, size, rNONE, 1, rNONE, addr, reg2);
}

void EMIT_OP_ADDR_REG(CodeGenGenericContext &ctx, x86Command op, x86Size size, unsigned addr, x86XmmReg reg2)
{
	EMIT_OP_RPTR_REG(ctx, op, size, rNONE, 1, rNONE, addr, reg2);
}

void EMIT_OP_RPTR_NUM(CodeGenGenericContext &ctx, x86Command op, x86Size size, x86Reg index, int multiplier, x86Reg base, unsigned shift, unsigned num)
{
#ifdef NULLC_OPTIMIZE_X86
	NULLC::RedirectAddressComputation(index, multiplier, base, shift);

	x86Argument arg = x86Argument(size, index, multiplier, base, shift);

	// Register reads
	NULLC::ReadRegister(ctx, base);
	NULLC::ReadRegister(ctx, index);

	switch(op)
	{
	case o_mov:
	case o_mov64:
		assert(base != rESP);

		NULLC::InvalidateAddressValue(ctx, arg);

		// Track target memory value
		NULLC::MemWrite(arg, x86Argument(num));
		break;
	default:
		assert(!"unknown instruction");
	}

	/*
	if(index == rNONE && NULLC::reg[base].type == x86Argument::argPtr && NULLC::reg[base].ptrSize == sNONE)
	{
		EMIT_OP_RPTR_NUM(ctx, op, size, NULLC::reg[base].ptrIndex, NULLC::reg[base].ptrMult, NULLC::reg[base].ptrBase, NULLC::reg[base].ptrNum + shift, num);
		return;
	}

	NULLC::regRead[base] = true;
	NULLC::regRead[index] = true;
	*/
#endif

	ctx.x86Op->name = op;
	ctx.x86Op->argA.type = x86Argument::argPtr;
	ctx.x86Op->argA.ptrSize = size;
	ctx.x86Op->argA.ptrIndex = index;
	ctx.x86Op->argA.ptrMult = multiplier;
	ctx.x86Op->argA.ptrBase = base;
	ctx.x86Op->argA.ptrNum = shift;
	ctx.x86Op->argB.type = x86Argument::argNumber;
	ctx.x86Op->argB.num = num;
	ctx.x86Op++;
}

void EMIT_OP_RPTR_NUM(CodeGenGenericContext &ctx, x86Command op, x86Size size, x86Reg reg1, unsigned shift, unsigned num)
{
	EMIT_OP_RPTR_NUM(ctx, op, size, rNONE, 1, reg1, shift, num);
}

void EMIT_OP_RPTR_NUM(CodeGenGenericContext &ctx, x86Command op, x86Size size, unsigned addr, unsigned number)
{
	EMIT_OP_RPTR_NUM(ctx, op, size, rNONE, 1, rNONE, addr, number);
}

void EMIT_REG_READ(CodeGenGenericContext &ctx, x86Reg reg)
{
	(void)ctx;
	(void)reg;

#ifdef NULLC_OPTIMIZE_X86
	NULLC::ReadRegister(ctx, reg);
#endif
}

void EMIT_REG_READ(CodeGenGenericContext &ctx, x86XmmReg reg)
{
	(void)ctx;
	(void)reg;

#ifdef NULLC_OPTIMIZE_X86
	NULLC::ReadRegister(ctx, reg);
#endif
}

void EMIT_REG_KILL(CodeGenGenericContext &ctx, x86Reg reg)
{
#ifdef NULLC_OPTIMIZE_X86
	// Eliminate dead stores to the register
	if(!NULLC::genRegRead[reg] && NULLC::genReg[reg].type != x86Argument::argNone)
	{
		x86Instruction *curr = NULLC::genRegUpdate[reg] + ctx.x86Base;

		if(curr->name != o_none)
		{
			curr->name = o_none;
			ctx.optimizationCount++;
		}
	}

	// Invalidate the register value
	NULLC::genReg[reg].type = x86Argument::argNone;
#else
	(void)ctx;
	(void)reg;
#endif
}

void EMIT_REG_KILL(CodeGenGenericContext &ctx, x86XmmReg reg)
{
#ifdef NULLC_OPTIMIZE_X86
	// Eliminate dead stores to the register
	if(!NULLC::xmmRegRead[reg] && NULLC::xmmReg[reg].type != x86Argument::argNone)
	{
		x86Instruction *curr = NULLC::xmmRegUpdate[reg] + ctx.x86Base;

		if(curr->name != o_none)
		{
			curr->name = o_none;
			ctx.optimizationCount++;
		}
	}

	// Invalidate the register value
	NULLC::xmmReg[reg].type = x86Argument::argNone;
#else
	(void)ctx;
	(void)reg;
#endif
}

void SetOptimizationLookBehind(CodeGenGenericContext &ctx, bool allow)
{
	ctx.x86LookBehind = allow;

#ifdef NULLC_OPTIMIZE_X86
	if(!allow)
	{
		NULLC::KillUnreadRegisters(ctx);

		NULLC::lastInvalidate = unsigned(ctx.x86Op - ctx.x86Base);
		NULLC::InvalidateState();

		NULLC::genRegUpdate[rESP] = 0;
	}
#endif
}

#endif
