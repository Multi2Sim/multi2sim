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







Brackets
--------


Use of code blocks
------------------

Name spaces
-----------



Identifiers
-----------


Symbol naming
-------------


Prefixes
--------


Integer types
-------------




Code documentation
==================


Comments
--------

### 

This is the following text.

Doxygen Comments
----------------


Source and header files
-----------------------


Structure of a header file
--------------------------

Structure of a source file
--------------------------


Avoid multiple inclusions
-------------------------




Class declarations
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



Smart pointers
==============

Unique pointers
---------------

Shared pointers
---------------


