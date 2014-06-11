<link rel="stylesheet" type="text/css" href="style.css" />

<div class="title">

C++ Coding Guidelines<br>
for Multi2Sim and Multi2C

</div>

<div class="version">

Revision 1.0<br>
Updated June 2014

</div>


<div class="pagebreak">
</div>




Code Formatting
===============

Indenting
---------

### 
Indentation should be done with one tab character per level. Spaces should not be used in any case in the beginning of lines. Each tab character counts as 8 regular characters toward the line width limit. To comply with the line width limit in a consistent manner across editors, it is recommended that editors be configured to display tabs using the width of 8 characters, but in no case replacing them by actual spaces.

    .       .       .       .       .       .       .       .       .       .       .
    if (x == 20)
            callFunction();

### 
Space characters should not occur in the beginning of the line. Tab characters should not occur once the line has started.



Line wrapping
-------------

### 
In most cases, lines should have at the most 80 characters. Each tab character in the beginning of the line counts as 8 characters toward the line length limit. The maximum line with can be exceptionally exceeded in the cases described below.


### 
A broken line should introduce two additional indentation levels (i.e., tab characters) in the next line of code. Lines do not necessarily have to be broken right before the last word. Instead, a proper position should be chosen for the line break to aid code readability. For example, the code below breaks the function header line right before the third argument, even though there is still some room left in the first line.

    .       .       .       .       .       .       .       .       .       .       .
    int LongFunctionWithManyArguments(int argument_1, char *arg2,
                    PrettyLongTypeName *arg3)
    {
            // Function body
            if (long_condition_left == long_condition_right &&
                            other_long_condition_left == other_long_condition_right)
            {
                    // Then block
            }
    }


Spaces
------

### 
Conditions and expressions in `if` statements and `for`/`while` loops should be preceded by a space. The space should appear on the left of the opening parenthesis of the condition. There should be no space separating the parentheses and their content.

    if (x == 10)
    while (i < 5)
    for (i = 0; i < 10; i++)


### 
The left parenthesis used in formal arguments of function declarations should not be preceded by a space. The left parenthesis used around actual arguments of a function call should not be preceded by a space either. In both cases, the parentheses should not be separated with spaces from their content.

    void FunctionDeclaration(int arg1, int arg2);
    FunctionCall(arg1, arg2);


### 
No spaces are used after an opening parenthesis or before a closing parenthesis on conditions or subexpressions.

    if (a < b)
            x = (a * (5 + c - (d + e)));


### 
Spaces should be used on both sides of all arithmetic/logic/bitwise operators and assignments.

    var1 = var2;
    for (x = 0; x < 10; x++)
            a += 2;
    var1 = a < 3 ? 10 : 20;
    result = (a + 3) / 5;


### 
Pre-increment and post-increment operators use no space by themselves. They are preceded or followed by a space only if a surrounding operator forces it.

    a++;
    b = ++x;


### 
In a type cast expression, the type name should not be separated from the surrounding parentheses by spaces. A space should appear after the right parenthesis, separating the type cast operator from the affected value.

    std::cout << (long long) value;
    myFuncCall((TypeName) value);


### 
The unary `&` operator should be preceded, but not followed, by a space, both when it acts as an *address-of* and as a *reference* operator. Similarly, the unary `*` operator should be preceded, but not followed, by a space, both when it acts as a *dereference* and as a *pointer* operator.

    void sayHello(std::ostream &os);
    void printIntPtr(int *x_ptr);

    int x;
    int &x_ref = x;
    int *x_ptr = &x;
    int y = *x_ptr;




Code blocks
-----------

### 
Opening brackets should always be the only character in a line, besides the preceding tabs. Every block of code should contain a new line before the opening bracket. This applies for function bodies, `if`/`else`/`while`/`for`/`do` blocks, class declarations, structures, enumerations, and stand-alone blocks.

    void myFunction(int arg1, char *arg2)
    {
            // Function body
    }

    if (cond)
    {
            // Multi-statement 'if' block
    }
    else
    {
            // Multi-statement 'else' block
    }

    while (1)
    {
            // Multi-statement 'while' loop
    }


### 
The content of `if` statements and `for`/`while` loops should not appear in a code block surrounded by brackets when it is composed by one single statement. Instead, only this statement should appear on the next line at a higher level of indentation.

    for (i = 0; i < 10; i++)
            only_one_line_example();

    if (!x)
            a = 1;
    else
            b = 1;

    while (isPendingTask())
            tasks.remove();


### 
An exception for the previous rule is an `if`-`else` structure where only one of the blocks is composed by one single statement. For symmetry, both blocks can appear surrounded by brackets, as long as at least one of them is composed by more than one statement.

    if (x == 10)
    {
            a = 10;
            b = 20;
    }
    else
    {
            UseBracketsForSymmetry();
    }



Name spaces
-----------

### 
All symbols should be declared within a C++ `namespace`, including functions, variables, constants, enumerations, and types. A `namespace` should embrace the entire content of every `.h` and `.cc` file.


### 
The open bracket after `namespace` should appear in a new line. The content of the `namespace` body is an exception of a code block where the content should not be indented. Since a name space spans entire files, this avoids having most of the code at one default level of indentation. The closing bracket should be followed by two spaces and a comment specifying what `namespace` is being closed.

    namespace x86
    {
    
    void myFunction();
    extern int my_var;
    
    }  // namespace x86


### 
Name spaces should always be accessed explicitly, avoiding `using namespace` statements in order to prevent name space conflicts. Name space `std` should also be always explicitly stated (`std::string`, `std::cout`, ...).



Identifiers
===========


Symbol naming
-------------

### 
Variables and class fields should be named using lower case characters, separating words with underscore `_` characters (e.g., `my_variable` or `field_name`).


### 
Functions, classes, enumerations, constants, and types should be named using capitalized words without underscore characters (e.g., `myFunction` or `TypeName`.


### 
The first word of a function name can start with a lower-case or upper-case character. Generally, a lower-case character is used when the first word is a verb determining the function behavior (e.g., `getValue()`, `setName()`, `dump()`).


### 
All symbol names should be representative of their conceptual meaning, avoiding short generic symbols such as `x` or `var`. Exceptions are typical looping variables (`i`, `j`, ...).


### 
Abbreviated symbol names should be avoided. A long verbose symbol name is preferable over an incomplete abbreviated name (e.g., `MyFunctionName` better than `MyFuncName`). Using unabbreviated identifiers makes it easier to remember their names.



Prefixes
--------

### 
All public symbols declared in a header file under the same `namespace` should have a common prefix. This involves enumerations, enumeration items, structures, classes, and global variables. Only symbols under structures, unions, or classes can omit the prefix, since these elements act as internal name spaces.


    // Hypothetical content of file Context.h
    namespace x86
    {
    
    enum ContextState
    {
            ContextStateInvalid = 0,
            ContextStateRunning,
            ContentStateSuspended
    };
    
    // Global variable using prefix 'context_'
    extern int context_global_var;
    
    class Context
    {
    public:
            int field_without_prefix;
    }
    
    }




Types
=====


Type categories
---------------

### 
All type names should omit their category when used. For example, when using a type defined as `enum ContextState` in a variable declaration, the type name should appear as `ContextState`, and not `enum ContextState`. The same rule applies for `union` types, `struct` types, and `class` types.



The *auto* type
---------------

### 
The *auto* type should be avoided, in order to explicitly state the type of each newly declared variable for code readability. Exceptions are those cases where the variable type is obvious, and it has an overly long name, such as container iterators. For example:

    std::list<std::unique_ptr<x86::Inst>>::iterator it = list.begin();

should be replaced by

    auto it = list.begin();




Integer types
-------------

### 
Integer variables should be declared using built-in integer types, and avoiding integer types defined in `stdint.h` (`uint8_t`, `int8_t`, `uint16_t`, etc). The following types should be used for unsigned integers of different sizes:

- `unsigned char` &mdash; 8-bit unsigned integer
- `unsigned short` &mdash; 16-bit unsigned integer
- `unsigned int` (or simply `unsigned`) &mdash; 32-bit unsigned integer
- `unsigned long long` &mdash; 64-bit unsigned integer

The following types should be used for signed integers of different sizes:

- `char` &mdash; 8-bit signed integer;
- `short` &mdash; 16-bit signed integer;
- `int` &mdash; 32-bit signed integer;
- `long long` &mdash; 64-bit signed integer;


###  
Integer types `long` and `unsigned long` should be avoided, since they represent integers of different sizes in 32- and 64-bit platforms. They should only be used when an integer type is intentionally dependent on the machine word size.





Code documentation
==================


Comments
--------



Doxygen Comments
----------------


Source Files
============


Structure of a header file
--------------------------

Structure of a source file
--------------------------


Multiple inclusions
-------------------------




Class Declarations
==================

Access specifiers
-----------------

Inline functions
----------------

Getters and setters
-------------------

The *Dump* method
-----------------

Static variables and functions
------------------------------


Singletons
----------



Dynamic Memory
==============

Avoid dynamic memory
--------------------

### 
Dynamic memory should be avoided in those cases where static memory has an equivalent behavior. For example:

    class Inst
    {
            int *v;
            myClass *o;
    public:
            Inst()
            {
                    v = new int[100];
                    o = new MyClass();
            }
    
            ~Inst()
            {
                    delete v;
                    delete o;
            }
    
            [...]
    }

The previous code allocates array `v` when the class is instantiated, and deallocates it when the object is freed. This is equivalent to creating a field `v` with 100 elements using static allocation. The same applies for variable `o` of type `myClass`, as the code below shows. Notice that the constructor of class `MyClass` is automatically invoked when an object of type `Inst` is instantiated; similarly, the destructor of class `MyClass` is automatically invoked for `o` before the container object of type `Inst` is destroyed.

    class Inst
    {
            int v[100];
            MyClass o;
    public:
            [...]
    }


Forbidden *delete*
------------------

### 
As the title suggests, *delete* is forbidden. Memory deallocation should never happen explicitly using the *delete* operator. In those cases where dynamic memory must be used (i.e., cannot be replaced by static allocations as suggested above), the newly allocated object should be captured by a *smart pointer*, either of type `std::unique_ptr` or `std::shared_ptr`.

A smart pointer is a C++ class that mimics the behavior of a raw pointer through operator overloading (dereference and assignment), while providing additional memory management mechanisms.


Unique pointers
---------------

### 
An `std::unique_ptr` pointer can be used to capture a newly allocated region of memory. When this variable is destroyed &mdash;that is, either is loses scope, or it is a member of a class being destroyed&mdash;, it will automatically free the memory region that it points to.

Only one variable of type `std::unique_ptr` can point to one particular region of memory, but there can be many regular pointers pointing to the same region. In order for unique pointers to be suitable, the programmer must be able to guarantee that the `std::unique_ptr` variable is always the last one accessing the memory region, that is, no other regular pointer will access this memory after the `std::unique_ptr` is destroyed. In most cases of dynamic memory allocation, it is possible to use unique pointers.

As an example, consider the following code using an explicit memory deallocation with `delete`:

    int myFunc()
    {
            int *v = new int[10];     // Allocate array
            [ ... ]                   // Use it
            delete v;                 // Free it
    }

The following code leverages a unique pointer to automatically free array `v` when it loses scope, i.e., when function `myFunc` returns:

    int myFunc()
    {
            std::unique_ptr<int> v = new int[10];
            [...]
    }


Shared pointers
---------------




Container Data Structures
=========================

Container traversal
-------------------




Data Files (*.dat*)
===================

Instruction set definitions
---------------------------

### 
Instruction sets (ISAs) of simulated architectures are defined on Multi2Sim using data (*.dat*) files in order to centralize all information related with instructions. This information is used in different positions of the code, including disassemblers, emulators, or assemblers. A data file is formed of a sequence of `DEFINST` macro uses, all of them with a common set of arguments. These arguments typically include an instruction name, a format string, and a value for the *opcode* field of the specific ISA encoding.

    DEFINST(Add,
            "add %dst, %src1, %src2",
            15)

    DEFINST(Sub,
            "sub %dst, %src1, %src2",
            16)


### 
A source or header file in Multi2Sim or Multi2C can use the information included in the data file by redefining the `DEFINST` macro and including the entire content of the file with an `#include` directive. For example, a disassembler can create an enumeration named `InstOpcode` that contains one constant per instruction, uniquely identifying them. The following code produces such enumeration:

    enum InstOpcode
    {
            InstOpcodeInvalid = 0,
    #define DEFINST(name, fmt, op) InstOpcode##name,
    #include "asm.dat"
    #undef DEFINST
            InstOpcodeCount
    };

Once the compiler directives are expanded, the lines above produce the following code:

    enum InstOpcode
    {
            InstOpcodeInvalid = 0,
            InstOpcodeAdd,
            InstOpcodeSub,
            InstOpcodeCount
    };


### 
The information contained in a data file can also be used in a source file to populate, for example, a table containing information about all instructions in the ISA. Consider the following code:

    struct InstInfo
    {
            InstOpcode opcode;
            std::string name;
            std::string fmt;
            int op;
    };

    InstInfo inst_info_table[InstOpcodeCount] = {
            { InstOpcodeInvalid, "", "", 0 },
    #define DEFINST(opcode, name, fmt, op) { InstOpcode##name, #name, fmt, op },
    #include "asm.dat"
    #endif
    };

Once the compiler directives are expanded, the lines above produce the following code:

    InstInfo inst_info_table[InstOpcodeCount] = {
            { InstOpcodeInvalid, "", "", 0 },
            { InstOpcodeAdd, "Add", "add %dst, %src1, %src2", 15 },
            { InstOpcodeSub, "Sub", "sub %dst, %src1, %src2", 16 }
    };


### 
The data file scheme can be used for instruction set definition, definition of the system call interface in a CPU architecture, or definition of a runtime-driver interface in an OpenCL/OpenGL/CUDA execution framework.



