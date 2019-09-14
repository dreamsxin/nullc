#include "InstructionTreeRegVm.h"

#include <assert.h>

const char* GetInstructionName(RegVmInstructionCode code)
{
	switch(code)
	{
	case rviNop:
		return "nop";
	case rviLoadByte:
		return "loadb";
	case rviLoadWord:
		return "loadw";
	case rviLoadDword:
		return "load";
	case rviLoadLong:
		return "loadq";
	case rviLoadFloat:
		return "loadf";
	case rviLoadDouble:
		return "loadd";
	case rviLoadImm:
		return "loadimm";
	case rviLoadImmLong:
		return "loadimmqh";
	case rviLoadImmDouble:
		return "loadimmdh";
	case rviStoreByte:
		return "storeb";
	case rviStoreWord:
		return "storew";
	case rviStoreDword:
		return "store";
	case rviStoreLong:
		return "storeq";
	case rviStoreFloat:
		return "storef";
	case rviStoreDouble:
		return "stored";
	case rviCombinedd:
		return "combdd";
	case rviBreakupdd:
		return "breakdd";
	case rviMov:
		return "mov";
	case rviDtoi:
		return "dtoi";
	case rviDtol:
		return "dtol";
	case rviDtof:
		return "dtof";
	case rviItod:
		return "itod";
	case rviLtod:
		return "ltod";
	case rviItol:
		return "itol";
	case rviLtoi:
		return "ltoi";
	case rviIndex:
		return "index";
	case rviGetAddr:
		return "getaddr";
	case rviSetRange:
		return "setrange";
	case rviJmp:
		return "jmp";
	case rviJmpz:
		return "jmpz";
	case rviJmpnz:
		return "jmpnz";
	case rviPop:
		return "pop";
	case rviPopq:
		return "popq";
	case rviPush:
		return "push";
	case rviPushQword:
		return "pushq";
	case rviPushImm:
		return "pushimm";
	case rviPushImmq:
		return "pushimmq";
	case rviCall:
		return "call";
	case rviCallPtr:
		return "callp";
	case rviReturn:
		return "ret";
	case rviAdd:
		return "add";
	case rviAddImm:
		return "addimm";
	case rviSub:
		return "sub";
	case rviMul:
		return "mul";
	case rviDiv:
		return "div";
	case rviAddm:
		return "addm";
	case rviSubm:
		return "subm";
	case rviMulm:
		return "mulm";
	case rviDivm:
		return "divm";
	case rviPow:
		return "pow";
	case rviMod:
		return "mod";
	case rviLess:
		return "less";
	case rviGreater:
		return "greater";
	case rviLequal:
		return "lequal";
	case rviGequal:
		return "gequal";
	case rviEqual:
		return "equal";
	case rviNequal:
		return "nequal";
	case rviShl:
		return "shl";
	case rviShr:
		return "shr";
	case rviShlm:
		return "shlm";
	case rviShrm:
		return "shrm";
	case rviBitAnd:
		return "bitand";
	case rviBitOr:
		return "bitor";
	case rviBitXor:
		return "bitxor";
	case rviLogXor:
		return "logxor";
	case rviAddl:
		return "addl";
	case rviAddImml:
		return "addimml";
	case rviSubl:
		return "subl";
	case rviMull:
		return "mull";
	case rviDivl:
		return "divl";
	case rviAddlm:
		return "addlm";
	case rviSublm:
		return "sublm";
	case rviMullm:
		return "mullm";
	case rviDivlm:
		return "divlm";
	case rviPowl:
		return "powl";
	case rviModl:
		return "modl";
	case rviLessl:
		return "lessl";
	case rviGreaterl:
		return "greaterl";
	case rviLequall:
		return "lequall";
	case rviGequall:
		return "gequall";
	case rviEquall:
		return "equall";
	case rviNequall:
		return "nequall";
	case rviShll:
		return "shll";
	case rviShrl:
		return "shrl";
	case rviShllm:
		return "shllm";
	case rviShrlm:
		return "shrlm";
	case rviBitAndl:
		return "bitandl";
	case rviBitOrl:
		return "bitorl";
	case rviBitXorl:
		return "bitxorl";
	case rviLogXorl:
		return "logxorl";
	case rviAddd:
		return "addd";
	case rviSubd:
		return "subd";
	case rviMuld:
		return "muld";
	case rviDivd:
		return "divd";
	case rviAdddm:
		return "adddm";
	case rviSubdm:
		return "subdm";
	case rviMuldm:
		return "muldm";
	case rviDivdm:
		return "divdm";
	case rviAddfm:
		return "addfm";
	case rviSubfm:
		return "subfm";
	case rviMulfm:
		return "mulfm";
	case rviDivfm:
		return "divfm";
	case rviPowd:
		return "powd";
	case rviModd:
		return "modd";
	case rviLessd:
		return "lessd";
	case rviGreaterd:
		return "greaterd";
	case rviLequald:
		return "lequald";
	case rviGequald:
		return "gequald";
	case rviEquald:
		return "equald";
	case rviNequald:
		return "nequald";
	case rviNeg:
		return "neg";
	case rviNegl:
		return "negl";
	case rviNegd:
		return "negd";
	case rviBitNot:
		return "bitnot";
	case rviBitNotl:
		return "bitnotl";
	case rviLogNot:
		return "lognot";
	case rviLogNotl:
		return "lognotl";
	case rviConvertPtr:
		return "convertptr";
	case rviCheckRet:
		return "checkret";
	case rviFuncAddr:
		return "funcaddr";
	case rviTypeid:
		return "typeid";
	default:
		assert(!"unknown instruction");
	}

	return "";
}
