#include "TestBase.h"

#include <stdint.h>

const char	*testMissingTests = 
"long a1 = 01, a2 = 0377, a3 = 01777777, a4 = 017777777777777777;\r\n\
long b1 = 0b, b2 = 1b, b3 = 1111111111100001010100101b, b4 = 101010101101010101011010101010011101011010010101110101010101001b;\r\n\
\r\n\
double c1 = 1e-3, c2 = 1e6, c3=123e2, c4=0.121e-4;\r\n\
\r\n\
char[] d1 = \"\\\\\\\'\\0\\\"\";\r\n\
int d1size = d1.size;\r\n\
\r\n\
int d2 = !4, d3 = ~5, d4 = -12;\r\n\
float e2 = -1.0f;\r\n\
double e3 = -3.0;\r\n\
long e4 = !324324234324234423l, e5 = ~89435763476541l, e6 = -1687313675313735l;\r\n\
int f1 = 2 << 4;\r\n\
long f2 = 3l << 12l;\r\n\
\r\n\
int f4 = 0 - f1;\r\n\
double f5 = 2 * 3.0, f6 = f1 - 0.0;\r\n\
\r\n\
return (d1[0] == '\\\\') + (d1[1] == '\\\'') + (d1[2] == 0) + (d1[3] == '\\\"') + (d1[4] == 0);";
TEST("Group of tests", testMissingTests, "5")
{
	CHECK_LONG("a1", 0, 1, lastFailed);

	CHECK_LONG("a2", 0, 255, lastFailed);
	CHECK_LONG("a3", 0, 524287, lastFailed);
	CHECK_LONG("a4", 0, 562949953421311ll, lastFailed);

	CHECK_LONG("b1", 0, 0, lastFailed);
	CHECK_LONG("b2", 0, 1, lastFailed);
	CHECK_LONG("b3", 0, 33538725, lastFailed);
	CHECK_LONG("b4", 0, 6154922420991617705ll, lastFailed);

	CHECK_DOUBLE("c1", 0, 1e-3, lastFailed);
	CHECK_DOUBLE("c2", 0, 1e6, lastFailed);
	CHECK_DOUBLE("c3", 0, 123e2, lastFailed);
	CHECK_DOUBLE("c4", 0, 0.121e-4, lastFailed);

	CHECK_INT("d1size", 0, 5, lastFailed);

	CHECK_INT("d2", 0, !4, lastFailed);
	CHECK_INT("d3", 0, ~5, lastFailed);
	CHECK_INT("d4", 0, -12, lastFailed);

	CHECK_FLOAT("e2", 0, -1.0f, lastFailed);
	CHECK_DOUBLE("e3", 0, -3.0, lastFailed);

	CHECK_LONG("e4", 0, !324324234324234423ll, lastFailed);
	CHECK_LONG("e5", 0, ~89435763476541ll, lastFailed);
	CHECK_LONG("e6", 0, -1687313675313735ll, lastFailed);

	CHECK_INT("f1", 0, 2 << 4, lastFailed);
	CHECK_LONG("f2", 0, 3ll << 12ll, lastFailed);

	CHECK_INT("f4", 0, -(2 << 4), lastFailed);
	CHECK_DOUBLE("f5", 0, 6.0, lastFailed);
	CHECK_DOUBLE("f6", 0, (2 << 4), lastFailed);
}

const char	*testMissingTests2 =
"auto a = {1,2,3,4};\r\n\
int[] b = a;\r\n\
int[] ref c = &b;\r\n\
int d = c[2];\r\n\
int e = a.size;\r\n\
\r\n\
int[] f = *c;\r\n\
int[4] ref g = &a;\r\n\
int[] h = *g;\r\n\
int j = g.size;\r\n\
\r\n\
auto n1 = auto(int a){ return -a; };\r\n\
auto n2 = auto(){}, n4 = int ff(){ return 0; };\r\n\
int ref(int) n1_ = n1;\r\n\
void ref() n2_ = n2;\r\n\
int ref(int) n11 = int n11f(int a){ return ~a; }, n33;\r\n\
void ref() n22 = void n22f(){}, n44;\r\n\
int k1 = n1(5), k2 = n1_(12), k3 = n11(7);\r\n\
n33 = auto(int n){ return n*1024; };\r\n\
n44 = auto(){};\r\n\
int k4 = n33(4);\r\n\
n44();\r\n\
return 1;";
TEST("Group of tests 2", testMissingTests2, "1")
{
	CHECK_INT("d", 0, 3, lastFailed);
	CHECK_INT("e", 0, 4, lastFailed);

	CHECK_INT("k1", 0, -5, lastFailed);
	CHECK_INT("k2", 0, -12, lastFailed);
	CHECK_INT("k3", 0, ~7, lastFailed);
	CHECK_INT("k4", 0, 4096, lastFailed);
}

const char	*testMissingTests3 =
"int max(int a, b, c){ return a > b ? (a > c ? a : c) : (b > c ? b : c); }\r\n\
int max(int a, b){ if(a > b) return a; else return b; }\r\n\
int max2(int a, b, c){ if(a > b) return max(a, c); else return max(b, c); }\r\n\
int a1 = max(1, 2, 3), a2 = max(1, 0, 2), a3 = max(7, 6, 5);\r\n\
int b1 = max2(1, 2, 3), b2 = max2(1, 0, 2), b3 = max2(7, 6, 5);\r\n\
\r\n\
int c1 = 1;\r\n\
do{\r\n\
c1 *= 2;\r\n\
}while(c1 < 500);\r\n\
\r\n\
class A{ char x, y; }\r\n\
\r\n\
int opt1 = ((12 / 3) < 8) != ((5 >= 7) == (5 <= 3)) + (18 >> 2) % (4 & 4 ^ 0 | 0xffff) + (5 && 1 || 3 ^^ 2);\r\n\
long opt2 = 18l >> 2l % 5l ^ 9l | 12l & 13l;\r\n\
long opt3 = 1l && 1l || 1l ^^ 1l;\r\n\
double opt4 = 8.0 % 3.0;\r\n\
return 1;";
TEST("Group of tests 3", testMissingTests3, "1")
{
	CHECK_INT("a1", 0, 3, lastFailed);
	CHECK_INT("a2", 0, 2, lastFailed);
	CHECK_INT("a3", 0, 7, lastFailed);

	CHECK_INT("b1", 0, 3, lastFailed);
	CHECK_INT("b2", 0, 2, lastFailed);
	CHECK_INT("b3", 0, 7, lastFailed);

	CHECK_INT("c1", 0, 512, lastFailed);

	CHECK_INT("opt1", 0, (1 != 1 + (18 >> 2) % (((4 & 4) ^ 0) | 0xffff) + 1) ? 1 : 0, lastFailed);
	CHECK_LONG("opt2", 0, (18l >> 2l % 5l ^ 9l) | (12l & 13l), lastFailed);
	CHECK_LONG("opt3", 0, 1, lastFailed);

	CHECK_DOUBLE("opt4", 0, 2.0, lastFailed);
}

const char	*testMissingTests4 =
"class matrix\r\n\
{\r\n\
    float[] arr;\r\n\
    int width, height;\r\n\
}\r\n\
matrix matrix(int width, height){ matrix ret; ret.arr = new float[width * height]; ret.width = width; ret.height = height; return ret; }\r\n\
class row\r\n\
{\r\n\
    matrix ref parent;\r\n\
    int x;\r\n\
}\r\n\
row operator[](matrix ref a, int x){ row ret; ret.x = x; ret.parent = a; return ret; }\r\n\
float ref operator[](row a, int y){ return &a.parent.arr[y * a.parent.width + a.x]; }\r\n\
\r\n\
matrix m = matrix(4, 4);\r\n\
m[1][3] = 4;\r\n\
\r\n\
int operator+(float a, int n){ return int(a) + n; }\r\n\
\r\n\
int min(int a, b){ return a < b ? a : b; }\r\n\
int ref rsize = new int;\r\n\
int[] operator+(int[] a, b){ int[] res = new int[min(a.size, b.size)]; for(int i = 0; i < res.size; i++) res[i] = a[i] + b[i]; *rsize = res.size; return res; }\r\n\
auto k = { 1, 2, 3, 4 } + { 4, 2, 5, 3 };\r\n\
int[] pass(int[] a){ return a; }\r\n\
auto i = pass({ 4, 7 });\r\n\
int ksize = k.size; int isize = i.size;\r\n\
int k1 = k[0], k2 = k[1], k3 = k[2], k4 = k[3];\r\n\
return m[1][3] + 2;";
TEST("Group of tests 4", testMissingTests4, "6")
{
	CHECK_INT("ksize", 0, 4, lastFailed);
	CHECK_INT("isize", 0, 2, lastFailed);

	CHECK_INT("k1", 0, 5, lastFailed);
	CHECK_INT("k2", 0, 4, lastFailed);
	CHECK_INT("k3", 0, 8, lastFailed);
	CHECK_INT("k4", 0, 7, lastFailed);
}

int CheckAlignment(NULLCRef ptr, int alignment)
{
	intptr_t asInt = (intptr_t)ptr.ptr;
	return asInt % alignment == 0;
}

LOAD_MODULE_BIND(test_alignment, "test.alignment", "int CheckAlignment(auto ref ptr, int alignment);")
{
	nullcBindModuleFunction("test.alignment", (void(*)())CheckAlignment, "CheckAlignment", 0);
}

const char	*testMissingTests5a =
"import test.alignment;\r\n\
long a = 3, a2 = 1;\r\n\
long b1 = a ** 27;\r\n\
long b2 = a2 ** 16884;\r\n\
long b3 = a ** -0;\r\n\
long b4 = a ** -1;\r\n\
char f;\r\n\
int k(){ align(16) char a; return CheckAlignment(&a, 16); }\r\n\
int r(){ align(16) char a; return k(); }\r\n\
int good = r();\r\n\
int k2(){ align(16) char a; return CheckAlignment(&a, 16); }\r\n\
int r2(){ align(16) char a; return k2(); }\r\n\
good += r2();\r\n\
return good;";
TEST("Group of tests 5", testMissingTests5a, "2")
{
	CHECK_LONG("b1", 0, 7625597484987ll, lastFailed);
	CHECK_LONG("b2", 0, 1, lastFailed);
	CHECK_LONG("b3", 0, 1, lastFailed);
	CHECK_LONG("b4", 0, 0, lastFailed);
}

const char	*testStaticIf1 =
"int a = 4;\r\n\
@if(typeof(a) != int)\r\n\
{\r\n\
	int[-1] f;\r\n\
}\r\n\
return a;";
TEST_RESULT("Static if test 1", testStaticIf1, "4");

const char	*testStaticIf2 =
"int a = 4;\r\n\
@if(typeof(a) == int)\r\n\
{\r\n\
	a = 5;\r\n\
}else{\r\n\
	int[-1] f;\r\n\
}\r\n\
return a;";
TEST_RESULT("Static if test 2", testStaticIf2, "5");

const char	*testStaticIf3 =
"int a = 4;\r\n\
@if(typeof(a) != int)\r\n\
{\r\n\
	int[-1] f;\r\n\
}else{\r\n\
	 a = 5;\r\n\
}\r\n\
return a;";
TEST_RESULT("Static if test 3", testStaticIf3, "5");

const char	*testStaticIf4 =
"auto foo(generic a)\r\n\
{\r\n\
	@if(typeof(a) == int)\r\n\
		return 1;\r\n\
	return 0;\r\n\
}\r\n\
\r\n\
auto bar(generic a)\r\n\
{\r\n\
	@if(typeof(a) == int)\r\n\
		return 1;\r\n\
	else\r\n\
		return 0;\r\n\
}\r\n\
\r\n\
int a = foo(4);\r\n\
int b = foo(4.0f);\r\n\
int c = bar(3);\r\n\
int d = bar(4l);\r\n\
return 1;";
TEST("Static if test 4", testStaticIf4, "1")
{
	CHECK_INT("a", 0, 1, lastFailed);
	CHECK_INT("b", 0, 0, lastFailed);
	CHECK_INT("c", 0, 1, lastFailed);
	CHECK_INT("d", 0, 0, lastFailed);
}

const char	*testStaticIf5 =
"auto foo1(generic a)\r\n\
{\r\n\
	@if(typeof(a) == int)\r\n\
		int[-1] a;\r\n\
	else if(typeof(a) == double)\r\n\
		int[-1] a;\r\n\
	else\r\n\
		return 1;\r\n\
	return 0;\r\n\
}\r\n\
auto foo2(generic a)\r\n\
{\r\n\
	@if(typeof(a) == int)\r\n\
		int[-1] a;\r\n\
	else if(typeof(a) == double)\r\n\
		return 1;\r\n\
	else\r\n\
		int[-1] a;\r\n\
	return 0;\r\n\
}\r\n\
auto foo3(generic a)\r\n\
{\r\n\
	@if(typeof(a) == int)\r\n\
		return 1;\r\n\
	else if(typeof(a) == double)\r\n\
		int[-1] a;\r\n\
	else\r\n\
		int[-1] a;\r\n\
	return 0;\r\n\
}\r\n\
return foo1(4l) + foo2(5.0) + foo3(4);";
TEST_RESULT("Static if test 5", testStaticIf5, "3");

const char	*testStaticIf6 =
"@if(1)\r\n\
{\r\n\
	int a;\r\n\
}else if(0){\r\n\
	float a;\r\n\
}\r\n\
return typeof(a) == int;";
TEST_RESULT("Static if test 6", testStaticIf6, "1");

const char	*testStaticIf7 =
"@if(0)\r\n\
{\r\n\
	int a;\r\n\
}else if(1){\r\n\
	float a;\r\n\
}\r\n\
return typeof(a) == float;";
TEST_RESULT("Static if test 7", testStaticIf7, "1");

const char	*testStaticIf8 =
"@if(1)\r\n\
{\r\n\
}else\r\n\
 auto(){};\r\n\
return 1;";
TEST_RESULT("Static if test 8", testStaticIf8, "1");

const char	*testStaticIf9 =
"@if(1)\r\n\
{\r\n\
}else if(1)\r\n\
 auto(){};\r\n\
return 1;";
TEST_RESULT("Static if test 9", testStaticIf9, "1");

const char	*testStaticIf10 =
"@if(1)\r\n\
	int a;\r\n\
else\r\n\
	float a;\r\n\
return typeof(a) == int;";
TEST_RESULT("Static if test 10", testStaticIf10, "1");

const char	*testAlignment =
"import test.alignment;\r\n\
align(16) int a;\r\n\
return CheckAlignment(&a, 16);";
TEST_RESULT("Global variable alignment", testAlignment, "1");

const char	*testAlignment2 =
"import test.alignment;\r\n\
align(16) class X{ char a; }\r\n\
char n; X[2] a;\r\n\
return CheckAlignment(&a[0], 16);";
TEST_RESULT("Array alignment 2", testAlignment2, "1");

const char	*testAlignment3 =
"import test.alignment;\r\n\
class X{ double x; int y; }\r\n\
class Y{ X x; int y; }\r\n\
Y y;\r\n\
return CheckAlignment(&y.x.x, 8) + CheckAlignment(&y.x.y, 4) + CheckAlignment(&y.y, 4);";
TEST_RESULT("Array alignment 3", testAlignment3, "3");

const char	*testAlignmentPadding =
"align(16) class X{ char a; }\r\n\
return sizeof(X);";
TEST_RESULT("Type padding for correct array element alignment", testAlignmentPadding, "16");

const char	*testAlignmentPadding2 =
"align(16) class X{ char a; }\r\n\
class Y{ char x; X v; }\r\n\
return sizeof(Y);";
TEST_RESULT("Type padding for correct array element alignment 2", testAlignmentPadding2, "32");

const char	*testAlignmentPadding3 =
"align(16) class X{ char a; }\r\n\
class Y{ char x; X v; char z; }\r\n\
return sizeof(Y);";
TEST_RESULT("Type padding for correct array element alignment 3", testAlignmentPadding3, "48");

struct AlignedStruct
{
	char x;
	double y;
	char z;
};

int CheckAlignmentStruct(NULLCRef ptr)
{
	AlignedStruct *obj = (AlignedStruct*)ptr.ptr;
	return obj->x == 0x34 && obj->y == 32.0 && obj->z == 0x45;
}

LOAD_MODULE_BIND(test_alignment_struct, "test.alignment.struct", "int CheckAlignmentStruct(auto ref ptr);")
{
	nullcBindModuleFunction("test.alignment.struct", (void(*)())CheckAlignmentStruct, "CheckAlignmentStruct", 0);
}

const char	*testAlignmentPadding4 =
"import test.alignment.struct;\r\n\
class X{ char x; double v; char z; }\r\n\
X x;\r\n\
x.x = 0x34;\r\n\
x.v = 32.0;\r\n\
x.z = 0x45;\r\n\
return CheckAlignmentStruct(&x);";
TEST_RESULT("Type padding for correct array element alignment 4", testAlignmentPadding4, "1");

struct TestAlignment5StructX{ double x; int y; };
struct TestAlignment5StructY{ TestAlignment5StructX x; int y; };

int TestAlignment5StructYSizeof()
{
	return sizeof(TestAlignment5StructY);
}

LOAD_MODULE_BIND(test_alignment_sizeof, "test.alignment.size", "int TestAlignment5StructYSizeof();")
{
	nullcBindModuleFunction("test.alignment.size", (void(*)())TestAlignment5StructYSizeof, "TestAlignment5StructYSizeof", 0);
}

const char	*testAlignmentPadding5 =
"import test.alignment.size;\r\n\
class X{ double x; int y; }\r\n\
class Y{ X x; int y; }\r\n\
return sizeof(Y) == TestAlignment5StructYSizeof();";
TEST_RESULT("Type padding for correct array element alignment 5", testAlignmentPadding5, "1");

const char	*testAlignmentPadding6 =
"align(2) class X{ char a; int b; char c; }\r\n\
return sizeof(X);";
TEST_RESULT("Smaller explicit alignment overrides members with larger alignment", testAlignmentPadding6, "8");

const char	*testAlignmentPadding7 =
"class X{ char a; align(4) char b; }\r\n\
return sizeof(X);";
TEST_RESULT("Class member alignment specification", testAlignmentPadding7, "8");

const char	*testAlignmentPadding8 =
"class X extendable\r\n\
{\r\n\
	char a;\r\n\
	align(4) char b;\r\n\
}\r\n\
class Y: X\r\n\
{\r\n\
	char c;\r\n\
}\r\n\
X x;\r\n\
Y y;\r\n\
\r\n\
return sizeof(X) == 12 && sizeof(Y) == 16;";
TEST_RESULT("Alignment and inhertance", testAlignmentPadding8, "1");

LOAD_MODULE(test_alignment_pading, "test.alignment.padding",
"class X{ char a; align(16) int b; char c; }\r\n\
X x;\r\n\
x.a = 75;\r\n\
x.b = 5603;\r\n\
x.c = 120;\r\n\
int a = sizeof(X);");
const char *testAlignmentPadding9 =
"import test.alignment.padding;\r\n\
return sizeof(X) == 32 && x.a == 75 && x.b == 5603 && x.c == 120;";
TEST_RESULT("Correct alignment of an imported class", testAlignmentPadding9, "1");

const char	*testAlignmentPadding10 =
"class X{ char x; int y; char z; }\r\n\
noalign class Y{ char x; int y; char z; }\r\n\
align(1) class Z{ char x; int y; char z; }\r\n\
return sizeof(X) == 12 && sizeof(Y) == 8 && sizeof(Z) == 8;";
TEST_RESULT("noalign test 1", testAlignmentPadding10, "1");

const char	*testAlignmentPadding11 =
"class X{ char x; short y; char z; }\r\n\
noalign class Y{ char x; short y; char z; }\r\n\
align(1) class Z{ char x; short y; char z; }\r\n\
return sizeof(X) == 8 && sizeof(Y) == 4 && sizeof(Z) == 4;";
TEST_RESULT("noalign test 2", testAlignmentPadding11, "1");

const char	*testAlignmentPadding12 =
"import test.alignment;\r\n\
class X{ char a; align(4) char b; }\r\n\
class Y{ char a; X x; }\r\n\
Y y;\r\n\
return CheckAlignment(&y.x.b, 4);";
TEST_RESULT("Class alignment inside a class", testAlignmentPadding12, "1");

const char	*testAlignmentHeap1 =
"import test.alignment;\r\n\
align(2) class X{ char x; }\r\n\
auto x = new X;\r\n\
auto y = new X;\r\n\
return CheckAlignment(x, 2) + CheckAlignment(y, 2);";
TEST_RESULT("Alignment of objects in heap 1", testAlignmentHeap1, "2");

const char	*testAlignmentHeap2 =
"import test.alignment;\r\n\
align(4) class X{ char x; }\r\n\
auto x = new X;\r\n\
auto y = new X;\r\n\
return CheckAlignment(x, 4) + CheckAlignment(y, 4);";
TEST_RESULT("Alignment of objects in heap 2", testAlignmentHeap2, "2");

const char	*testAlignmentHeap3 =
"import test.alignment;\r\n\
align(8) class X{ char x; }\r\n\
auto x = new X;\r\n\
auto y = new X;\r\n\
return CheckAlignment(x, 8) + CheckAlignment(y, 8);";
TEST_RESULT("Alignment of objects in heap 3", testAlignmentHeap3, "2");

const char	*testAlignmentHeap4 =
"import test.alignment;\r\n\
align(16) class X{ char x; }\r\n\
auto x = new X;\r\n\
auto y = new X;\r\n\
return CheckAlignment(x, 16) + CheckAlignment(y, 16);";
TEST_RESULT("Alignment of objects in heap 4", testAlignmentHeap4, "2");

const char	*testAlignmentHeap5 =
"import test.alignment;\r\n\
align(16) class X{ char[73] x; }\r\n\
auto x = new X;\r\n\
auto y = new X;\r\n\
return CheckAlignment(x, 16) + CheckAlignment(y, 16);";
TEST_RESULT("Alignment of objects in heap 5", testAlignmentHeap5, "2");

const char	*testAlignmentHeap6 =
"import test.alignment;\r\n\
align(16) class X{ int x; }\r\n\
auto x = new X[4];\r\n\
return CheckAlignment(&x[0], 16) + CheckAlignment(&x[1], 16) + CheckAlignment(&x[2], 16) + CheckAlignment(&x[3], 16);";
TEST_RESULT("Alignment of objects in heap 6", testAlignmentHeap6, "4");

const char	*testAlignmentHeap7 =
"import test.alignment;\r\n\
align(8) class X{ int x; }\r\n\
auto x = new X[4];\r\n\
return CheckAlignment(&x[0], 8) + CheckAlignment(&x[1], 8) + CheckAlignment(&x[2], 8) + CheckAlignment(&x[3], 8);";
TEST_RESULT("Alignment of objects in heap 7", testAlignmentHeap7, "4");

const char	*testAlignmentHeap8 =
"import test.alignment;\r\n\
align(16) class X{ char[1024] x; }\r\n\
int aligned = 0;\r\n\
for(int i = 0; i < 1024; i++)\r\n\
{\r\n\
	auto x = new X;\r\n\
	aligned += CheckAlignment(x, 16);\r\n\
}\r\n\
return aligned;";
TEST_RESULT("Alignment of objects in heap 6 (large objects)", testAlignmentHeap8, "1024");

const char	*testAlignmentDerived1 =
"import test.alignment;\r\n\
class X{ char x; int ref y; }\r\n\
X x;\r\n\
return CheckAlignment(&x.y, 4);";
TEST_RESULT("Alignment of derived types 1", testAlignmentDerived1, "1");

const char	*testAlignmentDerived2 =
"import test.alignment;\r\n\
class X{ char x; auto ref y; }\r\n\
X x;\r\n\
return CheckAlignment(&x.y, 4);";
TEST_RESULT("Alignment of derived types 2", testAlignmentDerived2, "1");

const char	*testAlignmentDerived3 =
"import test.alignment;\r\n\
class X{ char x; int[] y; }\r\n\
X x;\r\n\
return CheckAlignment(&x.y, 4);";
TEST_RESULT("Alignment of derived types 3", testAlignmentDerived3, "1");

const char	*testAlignmentDerived4 =
"import test.alignment;\r\n\
class X{ char x; auto[] y; }\r\n\
X x;\r\n\
return CheckAlignment(&x.y, 4);";
TEST_RESULT("Alignment of derived types 4", testAlignmentDerived4, "1");

const char	*testAlignmentDerived5 =
"import test.alignment;\r\n\
class X{ char x; int ref() y; }\r\n\
X x;\r\n\
return CheckAlignment(&x.y, 4);";
TEST_RESULT("Alignment of derived types 4", testAlignmentDerived5, "1");

const char	*testSideEffectOrdering1 =
"int a = 5;\r\n\
int set(int x){ a = x; return a; }\r\n\
int get(){ return a; }\r\n\
int ref getr(){ return &a; }\r\n\
return (getr() = 10) + get() + get();";
TEST_RESULT("Side-effect ordering test 1", testSideEffectOrdering1, "30");

const char	*testSideEffectOrdering2 =
"int a = 5;\r\n\
int set(int x){ a = x; return a; }\r\n\
int get(){ return a; }\r\n\
int ref getr(){ return &a; }\r\n\
return get() + (getr() = 10) + get();";
TEST_RESULT("Side-effect ordering test 2", testSideEffectOrdering2, "25");

const char	*testSideEffectOrdering3 =
"int a = 5;\r\n\
int set(int x){ a = x; return a; }\r\n\
int get(){ return a; }\r\n\
int ref getr(){ return &a; }\r\n\
return get() + get() + (getr() = 10);";
TEST_RESULT("Side-effect ordering test 3", testSideEffectOrdering3, "20");

const char	*testSideEffectOrdering4 =
"int a = 5;\r\n\
int set(int x){ a = x; return a; }\r\n\
set(5); // optimization barrier\r\n\
return (a = 10) + a + a;";
TEST_RESULT("Side-effect ordering test 4", testSideEffectOrdering4, "30");

const char	*testSideEffectOrdering5 =
"int a = 5;\r\n\
int set(int x){ a = x; return a; }\r\n\
set(5); // optimization barrier\r\n\
return a + (a = 10) + a;";
TEST_RESULT("Side-effect ordering test 5", testSideEffectOrdering5, "25");

const char	*testSideEffectOrdering6 =
"int a = 5;\r\n\
int set(int x){ a = x; return a; }\r\n\
set(5); // optimization barrier\r\n\
return a + a + (a = 10);";
TEST_RESULT("Side-effect ordering test 6", testSideEffectOrdering6, "20");

const char	*testNullPointerTypeUse =
"__nullptr t;\r\n\
__nullptr u;\r\n\
int ref a = t;\r\n\
a = new int(4);\r\n\
t = u;\r\n\
a = t;\r\n\
return !a;";
TEST_RESULT("nullptr type being explicitly used", testNullPointerTypeUse, "1");

const char	*testChainedPhiLegalizeIssue =
"int get(){ return 1987; }\r\n\
int year = get();\r\n\
int result = (year % 4 == 0) ? ((year % 400 == 0) ? 28 : 29) : 28;\r\n\
return result;";
TEST_RESULT("Issue with chained phi instruction legalization", testChainedPhiLegalizeIssue, "28");

const char *testTypeAliasLexemeSkip =
"int foo(@T x){ @U y = 4; return x * y; } return foo(4);";
TEST_RESULT("Type alias definition should not be skipped", testTypeAliasLexemeSkip, "16");

const char	*testFuzzingCrash1 =
"1||1&&2; return 1;";
TEST_RESULT("Fuzzing crash result 1", testFuzzingCrash1, "1");

const char	*testFuzzingCrash2 =
"1&&1||0&&1; return 1;";
TEST_RESULT("Fuzzing crash result 2", testFuzzingCrash2, "1");

const char	*testFuzzingCrash3 =
"1||1^^1&&1||1; return 1;";
TEST_RESULT("Fuzzing crash result 3", testFuzzingCrash3, "1");

const char	*testFuzzingCrash4 =
"1||1||1^^1&&1||1; return 1;";
TEST_RESULT("Fuzzing crash result 4", testFuzzingCrash4, "1");

const char	*testFuzzingCrash5 =
"return 1; for(;1;) return 0;";
TEST_RESULT("Fuzzing crash result 5", testFuzzingCrash5, "1");

const char	*testFuzzingCrash6 =
"int f(){ return 1; }\r\n\
int i = f();\r\n\
i ^= !16150l;\r\n\
return i;";
TEST_RESULT("Fuzzing crash result 6", testFuzzingCrash6, "1");

const char	*testManualCast1 =
"auto s = \"hello\"; char[] b = char[](s); return b[2];";
TEST_RESULT("Manual type cast 1", testManualCast1, "108");

const char	*testManualCast2 =
"auto s = \"hello\"; char[] b = (char[])(s); return b[2];";
TEST_RESULT("Manual type cast 2", testManualCast2, "108");

const char	*testManualCast3 =
"auto foo(generic x){ return x * 2; } auto f = int ref(int)(foo); return f(2);";
TEST_RESULT("Manual type cast 3", testManualCast3, "4");

const char	*testManualCast4 =
"auto foo(generic x){ return x * 2; } auto f = (int ref(int))(foo); return f(2);";
TEST_RESULT("Manual type cast 4", testManualCast4, "4");

const char	*testManualCast5 =
"auto a = int ref(4); return *a;";
TEST_RESULT("Manual type cast 5", testManualCast5, "4");

const char	*testManualCast6 =
"auto a = (int ref)(4); return *a;";
TEST_RESULT("Manual type cast 6", testManualCast6, "4");

const char	*testManualCast7 =
"int foo(int x){ return x * 2; } auto ref a = foo; return int ref(int)(a)(2);";
TEST_RESULT("Manual type cast 7", testManualCast7, "4");

const char	*testConsitionVariable1 =
"int f(int x){ return x; }\r\n\
int b = 0;\r\n\
if(int a = f(15))\r\n\
	b = a * 2;\r\n\
return b;";
TEST_RESULT("Variable definition inside a condition 1", testConsitionVariable1, "30");

const char	*testCharSignExtension1 =
"char a = -1;\r\n\
int b = a;\r\n\
\r\n\
char a2 = 255;\r\n\
int b2 = a2;\r\n\
\r\n\
char a3 = ' ' - '0';\r\n\
int b3 = char(' ' - '0');\r\n\
\r\n\
int a4_ = 255;\r\n\
char a4 = a4_;\r\n\
int b4 = a4;\r\n\
\r\n\
return b + b2 + b3 + b4;";
TEST_RESULT("Char loads are sign-extended 1", testCharSignExtension1, "-19");

const char	*testShortSignExtension1 =
"short a = -1;\r\n\
int b = a;\r\n\
\r\n\
char a2 = 65535;\r\n\
int b2 = a2;\r\n\
\r\n\
int a3_ = 65535;\r\n\
char a3 = a3_;\r\n\
int b3 = a3;\r\n\
\r\n\
return b + b2 + b3;";
TEST_RESULT("Short loads are sign-extended 1", testShortSignExtension1, "-3");

const char	*testInvalidOptimization =
"int foo(int c){ return 1 - 1 / c; } return foo(2);";
TEST_RESULT("Invald optimization (sub to dec transform)", testInvalidOptimization, "1");
