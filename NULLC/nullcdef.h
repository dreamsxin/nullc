#ifndef NULLC_DEF_INCLUDED
#define NULLC_DEF_INCLUDED

#pragma pack(push, 4)

// Wrapper over NULLC array, for use in external functions
struct NULLCArray
{
	char			*ptr;
	unsigned int	len;
};

// Wrapper over NULLC auto ref class for use in external functions
struct NULLCRef
{
	unsigned int	typeID;
	char			*ptr;
};

// Wrapper over NULLC function pointer for use in external functions
struct NULLCFuncPtr
{
	void			*context;
	unsigned int	id;
};

// Wrapper over NULLC auto[] class for use in external functions
struct NULLCAutoArray
{
	unsigned int	typeID;
	char			*ptr;
	unsigned int	len;
};

#pragma pack(pop)

#define NULLC_MAX_VARIABLE_NAME_LENGTH 2048
#define NULLC_MAX_TYPE_NAME_LENGTH 8192
#define NULLC_DEFAULT_GLOBAL_MEMORY_LIMIT 1024 * 1024 * 1024
#define NULLC_ERROR_BUFFER_SIZE 64 * 1024
#define NULLC_OUTPUT_BUFFER_SIZE 8 * 1024
#define NULLC_TEMP_OUTPUT_BUFFER_SIZE 16 * 1024
#define NULLC_MAX_GENERIC_INSTANCE_DEPTH 64
#define NULLC_MAX_TYPE_SIZE	256 * 1024 * 1024

//#define NULLC_VM_PROFILE_INSTRUCTIONS
//#define NULLC_STACK_TRACE_WITH_LOCALS
//#define NULLC_VM_CALL_STACK_UNWRAP

#if defined(_MSC_VER) && defined(_DEBUG)
//#define VERBOSE_DEBUG_OUTPUT
//#define IMPORT_VERBOSE_DEBUG_OUTPUT
//#define LINK_VERBOSE_DEBUG_OUTPUT
#endif

#if !defined(__CELLOS_LV2__) && !defined(__DMC__) && !defined(ANDROID)
	#define NULLC_AUTOBINDING
#endif

#if defined(__linux)
	#define NULLC_BIND extern "C" __attribute__ ((visibility("default")))
#else
	#define NULLC_BIND extern "C" __declspec(dllexport)
#endif

#if (defined(_MSC_VER) || defined(__DMC__) || defined(__linux)) && !defined(_M_X64) && !defined(NULLC_NO_EXECUTOR) && !defined(__x86_64__) && !defined(__arm__) && !defined(__aarch64__)
	#define NULLC_BUILD_X86_JIT
	#define NULLC_OPTIMIZE_X86
#endif

//#define NULLC_LLVM_SUPPORT

typedef unsigned char nullres;

#define NULLC_VM	0
#define NULLC_X86	1
#define NULLC_LLVM	2

#ifdef __x86_64__
	#define _M_X64
#endif

#ifdef __aarch64__
	#define _M_X64
#endif

#ifdef _M_X64
	#define NULLC_PTR_TYPE TYPE_LONG
	#define NULLC_PTR_SIZE 8
#else
	#define NULLC_PTR_TYPE TYPE_INT
	#define NULLC_PTR_SIZE 4
#endif


#if defined(__GNUC__)
	#define NULLC_PRINT_FORMAT_CHECK(format, args) __attribute__((__format__(__printf__, format, args)))
#else
	#define NULLC_PRINT_FORMAT_CHECK(format, args)
#endif

#endif
