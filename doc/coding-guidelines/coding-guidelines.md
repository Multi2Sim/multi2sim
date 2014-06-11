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



Empty lines
-----------

### 
One empty line should appear after the open bracket and before the closing bracket of a `namespace` block.

    namespace
    {
    --- Empty line ---
        [ ... ]
    --- Empty line ---
    }

### 
Two empty lines should appear between function implementations.

    void myFunction1()
    {
            [...]
    }
    --- Empty line 1 ---
    --- Empty line 2 ---
    void myFunction2()
    {
    }

### 
All source and header files should end with a new line character

    [...]
    namespace
    {
    [...]
    }
    --- Empty line ---

### 
Declarations headed with comments must be preceded by an empty line, including types, enumerations, variables, constants, structures, classes, or class members.

    // Constant
    const int my_constant_value = 10;
        --- Empty line ---
    // Comment
    class MyClass
    {
            // This is one field
            int field;
                --- Empty line ---
            // This is another field
            int field2;
                --- Empty line ---
            /// This is another field with some Doxygen documentation.
            int field3;
    };


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


Variable declarations
---------------------

### 
In C++, a variable declaration involves an implicit invocation to its constructor, and thus variable declaration order matters. To unify the way variables and objects are declared, their declarations should happen at the same time as their first assignment or constructor invocation. The following code

    int x;
    MyClass object(10, 20);
    x = 10;

should be replaced by

    MyClass object(10, 20);
    int x = 10;


### 
Looping variables should be declared within the loop header. The following code

    int i;
    for (i = 0; i < 10; i++)
            [ ... ]

should be replaced by

    for (int i = 0; i < 10; i++)
            [ ... ]


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

### 
Both single- and multi-line comments should use the C++ `//` comment notation. The only exception where C-style `/* ... */` comments can be used is the file headers containing the GPL license description.

### 
All code in function bodies should be grouped into regions of 1 to approximately 10 lines, headed by one comment, and followed by an empty line (unless it is the last code region in a code block). This leads to a unified structure across the project with a pattern *i*) comment header, *ii*) 1 to 10 lines of code, *iii*) empty line.

    void Context::Load(const std::vector<std::string> &args,
                    const std::vector<std::string> &env,
                    const std::string &cwd,
                    const std::string &stdin_file_name,
                    const std::string &stdout_file_name)
    {
            // String in 'args' must contain at least one non-empty element
            if (!args.size() || args[0].empty())
                    misc::panic("%s: function invoked with no program name, or with an "
                                    "empty program.", __FUNCTION__);
    
            // Program must not have been loaded before
            if (loader.get() || memory.get())
                    misc::panic("%s: program '%s' has already been loaded in a "
                                    "previous call to this function.",
                                    __FUNCTION__, args[0].c_str());
            
            // Create new memory image. This is a longer multi-line comment that does
            // not exceed a line width of 80 characters, considering each preceding
            // tab as 8 characters toward the limit.
            assert(!memory.get());
            memory.reset(new mem::Memory());
            address_space_index = emu->getAddressSpaceIndex();
    }

### 
The comments in a function body by themselves should express all actions carried out by the function. If all function code was removed, only the remaining comments should build a self-contained &quot;story&quot; that suffices to understand the function behavior.


Doxygen comments
----------------

### 
Public identifier declarations in header files should be preceded by Doxygen-style `///` comments, including types, enumerations, constants, variables, classes, and public/protected class fields. Private class members should use regular `//` comments instead.

### 
Doxygen comments should use a triple-slash notation. In function bodies, argument descriptions and return values should be preceded by the `\param` and `\return` keywords, respectively, followed by a line break and a tab character. The 80-character line width should be respected in all cases.

    class Context
    {
            /// Load a program from a command line into an existing context. The
            /// content is left in a state ready to start running the first x86 ISA
            /// instruction at the program entry.
            ///
            /// \param args
            ///     Command line to be used, where the first argument contains the
            ///     path to the executable ELF file.
            ///
            /// \param env
            ///     Array of environment variables. The environment variables
            ///     actually loaded in the program is the vector of existing
            ///     environment variables in the M2S process, together with any
            ///     extra variable contained in this array.
            ///
            /// \param cwd
            ///     Initial current working directory for the context. Relative
            ///     paths used by the context will be relative to this directory.
            ///
            /// \param stdin_file_name
            ///     File to redirect the standard input, or empty string for no
            ///     redirection.
            ///
            /// \param stdout_file_name
            ///     File to redirect the standard output and standard error output,
            ///     or empty string for no redirection.
            ///
            /// \return
            ///     No value is returned.
            void Load(const std::vector<std::string> &args,
                            const std::vector<std::string> &env,
                            const std::string &cwd,
                            const std::string &stdin_file_name,
                            const std::string &stdout_file_name);
    
            [...]
    }

### 
The arguments of a function can be referred to using the `\a` prefix in a Doxygen comment. Function names can be used without prefixes, since Doxygen automatically identifies them.

    /// Argument \a args of function Load() contains some arguments. Argument \a
    /// env of the same function contains environment variables.


Source Files
============

Copyright notice
----------------

Every source and header file should have an initial copyright notice, including a copy of the GPL 2 license preamble, as shown below. Fields `CURRENT_YEAR`, `AUTHOR_NAME`, and `EMAIL_ADDRESS` should be replaced according to the person creating the file. The copyright notice should be followed by one empty line.

    /*
     *  Multi2Sim
     *  Copyright (C) CURRENT_YEAR  AUTHOR_NAME (YOUR_EMAIL_ADDRESS)
     *
     *  This program is free software; you can redistribute it and/or modify
     *  it under the terms of the GNU General Public License as published by
     *  the Free Software Foundation; either version 2 of the License, or
     *  (at your option) any later version.
     *
     *  This program is distributed in the hope that it will be useful,
     *  but WITHOUT ANY WARRANTY; without even the implied warranty of
     *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     *  GNU General Public License for more details.
     *
     *  You should have received a copy of the GNU General Public License
     *  along with this program; if not, write to the Free Software
     *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
     */
            --- One empty line ---


Structure of a source (*.cc*) file
----------------------------------

### 
Source files should have the *.cc* name extension.

### 
The copyright notice in a source file is immediately preceded by one or more `#include` directives. The source file should only contain those `#include` directives that are strictly necessary, avoiding copying and pasting groups from other files, in order to minimize unnecessary dependencies.

### 
All `#include` directives should be classified in three groups separated by one empty line. The `#include` groups should be separated from the rest of the file using two empty lines. The groups should appear in the following order:

- Standard include files found in the default include directory for the compiler (`<iostream>`, `<cstring>`, etc.), using the notation based on `<...>` brackets. The list of files in this group should be ordered alphabetically.

- Include files located in other directories of the Multi2Sim or Multi2C tree. These files are expressed as a relative paths to the `multi2sim/src` or `multi2sim/m2c` directory, using the `<...>` bracket notation. The list of files in this group should be ordered alphabetically.

- Include files located in the same directory as the source file, using the double quote `"..."` notation. The list of files in this group should be ordered alphabetically.

For example:

    /*
     *  Multi2Sim
     *  Copyright (C) 2012
     *       [...]
     */

            --- One empty line ---

    #include <cassert>
    #include <cstring>
    #include <iostream>

            --- One empty line ---

    #include <arch/x86/asm/Inst.h>
    #include <lib/cpp/Debug.h>
    #include <lib/cpp/ELFReader.h>

            --- One empty line ---

    #include "Context.h"
    #include "Emu.h"

            --- Two empty lines ---

    [ Body of the source file ]

            --- One empty line at the end of the file ---



Structure of a header file
--------------------------


Include guards
--------------


Multiple inclusions
-------------------




Class Declarations
==================

Access specifiers
-----------------

### 
A class declaration should contain its private, protected, and public fields in this order, when possible. The `private` access specified should be omitted (this is the default access).

    class MyClass
    {
            int private_field;
    
    protected:
            int protected_field_1;
            int protected_field_2;
    
    public:
            void PublicFunction();
    }


Inline functions
----------------

### 
Function with an empty or very simple implementation (e.g., 1 line of code) should be inlined in the header file. This includes global function, class functions, or class constructors and destructors. Inline functions do not need to comply with the standard guidelines for new lines. They should still respect the maximum line width.

    class MyClass
    {
            void sayHello() { std::cout << "Hello\n"; }
    }



Getters and setters
-------------------

### 
Function fields should be declared private. Depending on their visibility, public getters and setters should be included in the class to access a field. Exceptionally, a field can be declared public when it is very frequently accessed for both read and write operation, making the use of getters and setters result in too verbose code.


### 
Getters should be declared as inline functions. A getter function name should begin with `get` (lower case). The function prototype should end with the `const` keyword to specify that it does not modify the object state.

    class MyClass
    {
            /// Return the value
            int getValue() const { return value; }
    }


### 
Setters should be declared as inline functions, unless their implementation involves complex value verification steps. The argument name for a setter should be the same as the field it affects, using `this` to distinguish between the class field and the function argument. A setter function name should begin with `set` (lower case).

    class MyClass
    {
            /// Set the value
            void setValue(int value) { this->value = value; }
    }



The *dump* method
-----------------

### 
Most classes should provide a `dump()` function with the following prototype:

    class MyClass
    {
            /// Dump object content
            void dump(std::ostream &os = std::cout) const;
    }

The function dumps the content of the object into an output stream, or `std::cout` if none is provided. The `const` suffix specifies that the internal object state is unmodified by the function.

### 
All classes with a `dump()` function should provide an alternative syntax based on the `<<` operand. This can be achieved with the inline operator overload shown below. The `dump()` and the `<<` operator functions can be grouped under the same comment, with no empty line between them.

    class MyClass
    {
            /// Dump object content
            void dump(std::ostream &os = std::cout) const;
            friend std::ostream &operator<<(std::ostream &os, const MyClass &object) {
                    object.dump(os);
                    return os;
            }
    }



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

### 
In those cases where one specific variable cannot be identified that points to a region of dynamic memory during its entire lifetime, shared smart pointers should be used. Shared pointers contain internal reference counters. The last shared pointer being destroyed (i.e., losing scope or being a member of an object being destroyed) will free the dynamic memory that it points to. In most cases, a unique pointer can be used instead.

An example of shared pointers is given can be found in Multi2Sim emulators, where multiple contexts can share one memory object. The memory object must be freed only when the last context using it is destroyed. This context is not necessarily the context that originally created the memory object.



Container Data Structures
=========================

Container traversal
-------------------

### 
Container data structures can be traversed using C++11 `:` notation, combined with the `auto` type.

    std::list<int> my_list;
    [ ... ]
    for (auto x : my_list)
            std::cout << x << '\n';

### 
Containers of complex objects should be traversed using the `auto &` type instead, which obtains references to each object, rather than copying it in a local variable for each iteration of the loop. This is strictly necessary when the list is composed of unique pointers to other objects, since an `std::unique_ptr` object cannot be replicated.

std::list<std::unique_ptr<Context>> context_list;
[ ... ]
for (auto &context : context_list)
        std::cout << context;

The example above is used to illustrate the effect of `auto &`. In that particular code, it would be clearer to specify the exact type of the loop traversal variable, replacing `auto &context` by `Context &context`.



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



