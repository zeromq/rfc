---
slug: 21
title: 21/CLASS
name: C Language Style for Scalability
status: stable
editor: Pieter Hintjens <ph@imatix.com>
---

The C Language Style for Scalability (CLASS) defines a consistent style and organization for scalable C library and application code built on modern C compilers and operating systems. CLASS aims to collect industry best practice into one reusable standard.

## License

Copyright (c) 2009-2013 iMatix Corporation

This Specification is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

This Specification is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, see <http://www.gnu.org/licenses>.

## Change Process

This Specification is a free and open standard (see "[Definition of a Free and Open Standard](http://www.digistan.org/open-standard:definition)") and is governed by the Digital Standards Organization's Consensus-Oriented Specification System (COSS) (see "[Consensus Oriented Specification System](http://www.digistan.org/spec:1/COSS)").

## Language

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED",  "MAY", and "OPTIONAL" in this document are to be interpreted as described in RFC 2119 (see "[Key words for use in RFCs to Indicate Requirement Levels](http://tools.ietf.org/html/rfc2119)").

## Goals

CLASS aims to be a complete and reusable style and organization guide for scalable C projects. CLASS collects best practice to solve a set of specific and well-known quality problems with C projects. It evolved from the style guide of the ZeroMQ [CZMQ project](https://github.com/zeromq/czmq). CLASS is plug compatible with the "coding style guide" requirements of the C4 process (see "[Collective Code Construction Contract](http://rfc.zeromq.org/spec:16)").

The specific goals for this Specification are to:

* Give project maintainers and contributors a tool for measuring the quality of patches to a project.
* Give the code the appearance of a single perfect author writing at one moment in time.
* Reduce the number of lines of code and complexity of C projects.
* Ensure total consistency of every line of code, across a large number of projects.
* Reduce the risk of writing C code by solving common C style problems in a minimal fashion.
* Provide a vehicle to collect best practice over time.

### Quality Aspirations

The goal of CLASS is to help programmers write high-quality C code in a consistent fashion. We define "high quality" primarily thus:

* The code is easy to read and understand even if you know little or nothing about its specific context.
* The code is easy to reuse and remix, in whole or in fragments.
* The code is easy to write correctly, and errors are rapidly clear.

Our main tool for achieving this is to identify and apply the best patterns and styles. The reader can learn these once, and then see them repeatedly across a wide body of code. Deviations from familiar patterns can then be taken as a sign of low-quality code that is more likely to contain errors and less remixable.

### Common Problems

With this Specification we aim to address a specific set of problems that hurt the quality, remixability, and economics of many C projects:

* If there is no style guide for a project, the maintainers must either rewrite contributions by hand (which costs extra work), or must abandon any attempt to enforce a consistent "voice" across a project (which hurts code quality and reusability). Thus maintainers need a written style guide to which they can point contributors.

* If there is no reusable style guide, each project founder will improvise their own ad hoc style. This wastes previous experience, and creates unnecessary work for project founders. It also means each project will end up with a different style.

* If a set of projects each have a different style, they cannot share code easily, which damages the economics of remixing. The more projects that use a single style, the lower the cost of code reuse. This is especially important in open source but also applies to projects built internally in an organization.

* With no vehicle for collecting and sharing best practices in C programming, most projects will have mediocre structuring and style, leading to unnecessary complexity, excess lines of code, accumulation of unused code, and other quality problems.

* When project founders create a new project without help, they often make mistakes such as forgetting to define a license for their code. This can create legal problems.

While these problems can affect all software projects, we solve them specifically for C projects.

## General Style

### Definitions

The overall unit of work is a *project* that builds as a library with a set of executable test programs and supplemental command-line tools.

The project is composed of *classes*, where each class covers one area of work and is implemented as a source file with matching header file.

Each class implements a series of *methods*, where each method performs one task and is implemented as a C function.

### Language

The language for all names and comments SHALL be English.

### Naming

Names SHOULD be chosen for ease of readability, and consistency. Unless otherwise specified, the following style rules apply to all given names:

* Names SHALL be short words that are simple, clear, and obvious to the reader.
* Names SHALL be used consistently for any given semantics.
* Names SHOULD NOT be invented words or acronyms.
* Names MAY be abbreviations if used widely.
* Names SHALL NOT be reserved C or C++ keywords.

## Project Style

### Project Focus

The project SHALL focus on one identifiable problem space, which SHALL be stated explicitly in the project <tt>README</tt>.

### Project Name

The project SHALL have these short names and abbreviations:

* A *project short name* used in paths and URLs that identify the project. This would be used for instance in the GitHub project name. In this Specification we will use <tt>myproject</tt> as the example.
* A *project prefix* used for project files, output libraries, and method names. This would be used for instance in the library produced for the project. The prefix MAY be an acronym. In this Specification we will use <tt>myp</tt> as the example.

These names SHALL be noted in the project <tt>README</tt>.

### General Layout

The project SHALL contain at least these files and directories:

* A <tt>README</tt> file that refers to this Specification and provides other necessary information about the project.
* A license file (e.g., <tt>COPYING</tt> or <tt>LICENSE</tt>) that specifies the terms of distribution for the project.
* An <tt>include</tt> directory for all header files.
* A <tt>src</tt> directory for all library source files.
* The *public header file* (<tt>include/myproject.h</tt>).
* Scripts and makefiles to build and test the project on at least one platform.

The project MAY contain these files and directories which MUST have these names if present at all:

* An <tt>AUTHORS</tt> file listing all contributors to the project.
* A <tt>doc</tt> directory containing documentation.
* The *internal header file* (<tt>src/myp_classes.h</tt>).

The project SHOULD install these files:

* The project header files and all class header files that form part of the public API.
* The project library, named with the project prefix (<tt>libmyp.a</tt> on POSIX platforms, <tt>myp.dll</tt> on Windows).
* Command-line tools, if present.

### Dependencies

The project SHALL depend at least on CZMQ (libczmq), which imports ZeroMQ (libzmq), to provide portable APIs around networking, threads, file systems, and other aspects.

### Project Header Files

The project SHALL provide two services via header files:

1. A set of internal definitions to class source files, which a class source file can access with a single <tt>include</tt> statement.
1. A public API that calling applications can access with a single <tt>include</tt> statement.

These two services MAY be combined into one project header file ((<tt>myproject.h</tt>), or MAY be split into an public header file (<tt>include/myproject.h</tt>) and an internal header file (<tt>src/myp_classes.h</tt>). The project MAY further break down these header files if necessary.

The public header file SHALL define a version number for the project as follows:

```
//  MYPROJECT version macros for compile-time API detection
#define MYPROJECT_VERSION_MAJOR 1
#define MYPROJECT_VERSION_MINOR 0
#define MYPROJECT_VERSION_PATCH 0

#define MYPROJECT_MAKE_VERSION(major, minor, patch) \
    ((major) * 10000 + (minor) * 100 + (patch))
#define MYPROJECT_VERSION \
    MYPROJECT_MAKE_VERSION(MYPROJECT_VERSION_MAJOR, \
                           MYPROJECT_VERSION_MINOR, \
                           MYPROJECT_VERSION_PATCH)
```

The project header file SHALL assert the required version numbers for any dependencies immediately after including their respective header files, like this:

```
#include <czmq.h>
#if CZMQ_VERSION < 10203
1. error "myproject needs CZMQ/1.2.3 or later"
#endif
```

Definitions in the public header file are visible to calling applications as well as class source code. The public header file SHALL #include all class header files that form part of the public API for the project.

Definitions in the internal header file are visible only to class source code. The internal header file, if present, SHALL include the public header file, all class header files, and all system and library header files needed by the project. The primary goal here is to keep delicate system-dependent <tt>#include</tt> chains in a single place, and away from class source code.

### Template README File

```
1. Project Title

<One-paragraph statement of the goals of the project, and the problems it aims to solve>

## References

* Contribution policy is defined by C4 (http://rfc.zeromq.org/spec:21).
* Project style guide is defined by CLASS (http://rfc.zeromq.org/spec:14).
 * short name: <title>
 * prefix: <prefix>
* Licensed under <license name>, see COPYING
* Language level: C99
```

### Language Level

The project SHOULD use the C99 language for best clarity, but MAY use the C89 language for compatibility with older platforms. The language level SHALL be noted in the project <tt>README</tt> and all source code SHALL conform to it.

NOTE: Microsoft Visual C/C++ does *not* support C99 and projects must build using C++ language extensions to get access to C99 syntax. Because of this, projects SHOULD NOT use any C99 syntax that is not a strict subset of C++.

### Use of the Preprocessor

Project source code SHOULD NOT include any header files except the project header file. This ensures that all class source code compiles in exactly the same environment.

Project source code SHALL NOT define "magic numbers" (numeric constants); these SHALL be defined in the external or internal header file, as appropriate.

Projects MAY use the preprocessor for these purposes:

* To create backwards compatibility with older code.
* To improve portability by e.g., mapping non-portable system calls into more portable ones.
* To create precise, small macros with high usability.

Projects SHOULD NOT use the preprocessor for other work except when it significantly reduces the complexity of code.

Macro names SHALL be uppercase when they represent constants, and lowercase when they act as functions.

## Class Styles

### File Organization

Each class SHALL be written as two files:

* A header file: <tt>include/myp_myclass.h</tt>
* A source file: <tt>src/myp_myclass.c</tt>

These two files SHALL be the original documentation for the class. Specifically, the class header SHALL define the API for the class, and the class source file SHALL define the implementation of each method.

Class names SHALL follow the General Style for Naming. We will use <tt>myclass</tt> in examples.

Every source and header file SHALL start with an appropriate file header that states at least:

* The name of the class or file and its purpose
* The copyright statement for the class
* The name of the project and a URL if relevant
* The summary license statement

Here is a template file header for an MPLv2 open source project:

```
/*  =========================================================================
    <name> - <description>

    Copyright (c) the Contributors as noted in the AUTHORS file.
    This file is part of MYPROJ, see https://github.com/MYORG/MYPROJ.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    =========================================================================
*/
```

### Class Types

We define two types of class:

* *Stateful classes*, where the class provides methods working on *instances*, which are like "objects" in an object-oriented language.
* *Stateless classes*, where the class provides methods that work purely on data provided by the caller or system.

A stateful class SHALL provide these methods:

* A constructor method <tt>myp_myclass_new ()</tt>
* A destructor method <tt>myp_myclass_destroy ()</tt>
* A self-test method <tt>myp_myclass_test ()</tt>

A stateful class MAY provide these methods, and SHALL use these names when providing such functionality:

* A duplicator method <tt>myp_myclass_dup ()</tt>
* A set of list navigation methods <tt>myp_myclass_first ()</tt> and <tt>myp_myclass_next ()</tt>.
* Print methods <tt>myp_myclass_print ()</tt> and <tt>myp_myclass_fprint ()</tt>.

A stateless class SHALL provide at least this method:

* A self-test method <tt>myp_myclass_test ()</tt>

### Method Names

Method names SHALL follow the General Style for Naming. Method names SHOULD be verbs ("destroy", "insert", "lookup") or adjectives ("ready", "empty", "new"). The method name SHOULD imply the method return type, where verbs return a success/failure indicator, if anything, and adjectives return a value or instance.

### Class Header File

The class header file SHALL have this layout:

* The file header
* An outer <tt>#ifndef</tt> that makes it safe to include the header file multiple times
* Calling conventions for C++
* A forward reference to the class type, for stateful classes
* Prototypes for the class methods

Here is a template header file for stateful classes, not showing the file header:

```
#ifndef __MYMOD_H_INCLUDED__
#define __MYMOD_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

//  Opaque class structure
typedef struct _myp_myclass_t myp_myclass_t;

//  Create a new <class name> instance
CZMQ_EXPORT myp_myclass_t *
    myp_myclass_new (void);

//  Destroy a <class name> instance
CZMQ_EXPORT void
    myp_myclass_destroy (myp_myclass_t **self_p);

//  Self test of this class
void
    myp_myclass_test (bool verbose);

#ifdef __cplusplus
}
#endif

#endif
```

Here is a similar template header file for stateless classes:

```
#ifndef __MYMOD_H_INCLUDED__
#define __MYMOD_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

//  Self test of this class
int
    myp_myclass_test (bool verbose);

#ifdef __cplusplus
}
#endif

#endif
```

All public methods SHALL be declared with <tt>CZMQ_EXPORT</tt> in the class header file so that these methods are properly exported on operating systems that require it.

### Class Source File

The class source file SHALL define:

* The class structure, for stateful classes. This structure SHALL be *opaque* and known only to code in the class source file.

* The class methods, in the same order as defined in the class header: constructor, destructor, other methods, and finally self test.

* Any static functions used in the class methods.

* Any global or static variables needed.

### Class Properties

For stateful classes, the class structure has one or more properties defined as a private C structure in the class source file.

This SHOULD be defined as follows:

```
//  Structure of our class

struct _myclass_t {
    <type> <name>;              //  <description>
};
```

Property names SHALL follow the General Style for Naming. Property names SHOULD be nouns or adjectives (typically used for Boolean properties). We will use <tt>myprop</tt> in examples.

## Method Styles

### General Rules

#### Argument Names

Argument names SHALL be consistent with property names.

#### Return Values

Success/failure SHALL be indicated by returning an int, with values zero or -1 respectively.

Strings SHALL be returned as "char *" when they are passed to the caller, who must free them.

Strings SHALL be returned as "const char *" when the caller may not modify or free them.

Compound return values, e.g. a size-specified buffer, SHOULD be returned as fresh objects of a suitable class. The API SHOULD NOT return compound values via multiple routes, e.g. data via an argument and size via the return code.

### The Self Test Method

In stateless classes, the only standard method is <tt>myp_myclass_test ()</tt>, which SHALL conduct a self test of the class, returning silently on success, and asserting on failure.

The self test method shall take this general form:

```
//  --------------------------------------------------------------------------
//  Runs selftest of class

void
myp_myclass_test (int verbose)
{
    printf (" * myp_myclass: ");
    //  Conduct tests of every method
    printf ("OK\n");
}
```

* The self test method SHALL be a primary source of example code for users of the class.
* The self test method SHOULD cover every other method in the class.

### Stateful Classes

#### The Constructor Method

The constructor SHALL take this general form:

```
//  Create a new myp_myclass instance
myp_myclass_t *
myp_myclass_new (<arguments>)
{
    myp_myclass_t *self = (myp_myclass_t *) zmalloc (sizeof (myp_myclass_t));
    assert (self);
    self->someprop = someprop_new ();
    assert (self->someprop);
    return self;
}
```

* The constructor SHALL initialize all properties in new class instances. Properties SHALL either get a suitable initial value, or be set to zero. Very large properties MAY exceptionally be left uninitialized for performance reasons; such behavior MUST be explicitly noted in the constructor body.

* Any properties that are dynamically allocated SHOULD be allocated in the constructor but MAY be left as null.

* The constructor MAY take one or more arguments, which SHALL correspond to properties to be initialized.

* The constructor SHALL return either a new instance reference, or null, if construction failed.

#### The Destructor Method

The destructor SHALL take this general form:

```
//  Destroy a myp_myclass instance
void
myp_myclass_destroy (myp_myclass_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        myp_myclass_t *self = *self_p;
        someprop_destroy (&self->someprop);
        anotherprop_destroy (&self->anotherprop);
        lastprop_destroy (&self->lastprop);
        free (self);
        *self_p = NULL;
    }
}
```

* The destructor SHALL nullify the provided instance reference.
* The destructor SHALL be idempotent, i.e., can be called safely on the same instance reference more than once.
* The destructor SHALL safely free properties and child class instances that are not null.

#### The Duplicator Method

The class MAY offer a duplicator method which creates a full copy of an instance; if it offers such semantics, the method MUST be called <tt>myp_myclass_dup ()</tt> and take this general form:

```
//  Create a copy of a myp_myclass instance

myp_myclass_t *
myp_myclass_dup (myp_myclass_t *self)
{
    if (self) {
        assert (self);
        myp_myclass_t *copy = myp_myclass_new (...);
        if (copy) {
            //  Initialize copy
        }
        return copy;
    }
    else
        return NULL;
}
```

* The duplicator SHALL return either a new instance reference, or null, if construction failed, in the same manner as the constructor.

* The duplicator SHALL accept a null instance reference, and then return null.

* A duplicated instance SHALL be entirely independent of the original instance (i.e. all properties SHALL also be duplicated).

#### List Navigation Methods

A class MAY act as a list container for other items, which may be child class instances, strings, memory blocks, or other structures.

Such a container class SHALL keep the list cursor position in the instance, and provide the following methods for navigating the list:

```
//  Return first item in the list or null if the list is empty

item_t *
myp_myclass_first (myp_myclass_t *self)
{
    assert (self);
    //  Reset cursor to first item in list
    return item;
}

//  Return next item in the list or null if there are no more items

item_t *
myp_myclass_next (myp_myclass_t *self)
{
    assert (self);
    //  Move cursor to next item in list
    return item;
}
```

* The navigation methods SHALL return null to indicate "no more items".

* The navigation methods SHALL be idempotent, and specifically, calling <tt>myp_myclass_next ()</tt> when at the end of the list SHALL return null each time.

* The class MAY offer <tt>myp_myclass_last ()</tt> and <tt>myp_myclass_prev ()</tt> methods.

* The class MAY offer <tt>myp_myclass_size ()</tt> which returns the list size.

* If the class offers methods to create list items, these SHALL be called <tt>myp_myclass_append ()</tt> (to add to the end of the list) and <tt>myp_myclass_insert ()</tt> (to add to the start of the list).

* If the class offers a method to remove a list item, this SHALL be called <tt>myp_myclass_delete ()</tt>; it SHALL take the item reference as argument, and it SHALL delete the first matching item in the list, if any.

* If the class maintains multiple lists, it SHALL create unique method names for each list by adding a list name, e.g., <tt>myp_myclass_myitem_first ()</tt>.

#### Accessor Methods

The class MAY expose instance properties via its API, in which case this SHALL be done through accessor methods.

To return the value of a property the class SHALL define an accessor method like this:

```
//  Return the value of myprop
<type>
myp_myclass_myprop (myp_myclass_t *self)
{
    assert (self);
    return self->myprop;
}
```

To write the value of a property, if this is permitted, the class SHALL define an accessor method like this:

```
//  Set the value of myprop
void
myp_myclass_set_myprop (myp_myclass_t *self, <type> myprop)
{
    assert (self);
    self->myprop = myprop;
}
```

* Properties exposed by accessor methods MAY not actually exist as such in the instance; they may be calculated rather than simply copied to/from the instance structure.

#### Formatted String Arguments

When a method (such as an accessor method) accepts a string argument as primary argument, it SHOULD use a variable argument list and perform vsnprintf formatting on that string argument.

#### General Methods

The class MAY offer any number of other methods that operate on the instance. These methods shall take this general form:

* The first argument to the method SHALL be the instance reference.
* Other arguments may follow.

A method may take ownership of an object instance and then act as a destructor of the object instance at some later stage. In that case the method SHALL use the same style as the destructor.

#### Return Values

Methods SHOULD use one of the following patterns for returning values to the caller:

* Returning nothing, if no return value is expected.
* Returning a property value, on an accessor method.
* Returning an object instance, on a constructor or duplicator.
* Returning a child value, on a list navigation method.
* Returning zero on success, -1 on failure.
* Returning a freshly-allocated string.

## Code Style

### Thread Safety

* All methods SHALL be thread safe.

* Class instances SHOULD NOT generally be thread safe; a class instance will be owned by a single calling thread.

* In exceptional cases class instances MAY be made thread safe by the addition of mutexes or locks inside methods.

### Heap Use

One of the goals of CLASS is to hide heap use as far as possible within classes. Application programs SHOULD use the heap only through constructors and duplicators (including the library <tt>strdup ()</tt> function). Class methods MAY use the heap with care, but follow these rules:

* When a class instance has been destroyed, all heap memory it used MUST be freed. Classes SHALL NOT leak memory under any conditions except during abnormal termination (e.g., on a failed assertion).

* Non-atomic properties SHOULD be re-allocated (i.e., freed and allocated) in accessor functions that modify them, as needed.

* The instance structure MAY use char arrays instead of heap allocated char pointers.

* When freeing a non-atomic property outside the destructor, a method MUST set the property to null if it does not allocate a new value immediately.

### Static Variables

Classes SHOULD NOT use static variables except in exceptional cases, such as for global variables.

Static variables are not thread safe and they are therefore considered poor practice.

Particularly for representing any temporary state inside a class body, stack variables SHALL be used in place of static variables.

### Static Functions

Functions that are not exported by a class are defined as <tt>static</tt> and named <tt>s_functionname ()</tt> with no use of the project prefix or class name.

Static functions MAY be defined before first use, or MAY be prototyped and defined immediately after first use.

Static functions SHOULD NOT be collected at the end of the class source code.

## Code Style

### Indentation

Indentation SHALL be 4 spaces per level. Tab characters SHALL NOT be used in code.

### Declarations

Functions SHALL be prototyped as follows:

```
<type>
    <name> (<arguments>);
```

Functions SHALL be defined as follows:

```
<type>
<name> (<arguments>)
{
    <body>
}
```

When the project uses C99, stack variables SHALL be defined in-line, as close as possible to their first use, and initialized. For example:

```
myp_myclass_t *myclass = myp_myclass_new ();
char *comma = strchr (surname, '.');
```

When the project uses C89, stack variables SHALL all be defined and initialized at the start of the function or method where they are used.

* Variables and functions SHALL use lower-case names.

* Where necessary, underlines SHALL be used to separate parts of a name.

* Variable names like <tt>i</tt> and <tt>temp</tt> that carry no information SHALL NOT be used.

### Statements

Code lines of more than 80-100 characters SHOULD be folded for readability.

Single-statement blocks SHALL NOT be enclosed in brackets.

This is the form of a single-statement block:

```
if (comma == NULL)
    comma = surname;
```

In <tt>else</tt> statements, the <tt>else</tt> SHALL be put on a line by itself.

Multiple <tt>if</tt>/<tt>else</tt> tests SHALL be stacked vertically to indicate that the order is arbitrary.

This is the form of a stacked <tt>if</tt> statement block:

```
if (command == CMD_HELLO)
    puts ("hello");
else
if (command == CMD_GOODBYE)
    puts ("goodbye");
else
if (command == CMD_ERROR)
    puts ("error");
```

With multi-statement conditional blocks, the closing bracket SHALL be put on a line by itself, aligned with the opening keyword.

This is the form of a stacked <tt>if</tt> statement block with brackets around each conditional block:

```
if (command == CMD_HELLO) {
    puts ("hello");
    myp_peer_reply (peer, CMD_GOODBYE);
}
else
if (command == CMD_GOODBYE) {
    puts ("goodbye");
    myp_peer_reply (peer, CMD_DISCONNECT);
}
else
if (command == CMD_ERROR) {
    puts ("error");
    myp_peer_close (peer);
}
```

This is the form of a <tt>while</tt> statement:

```
char *comma = strchr (surname, ',');
while (comma) {
    *comma = ' ';
    comma = strchr (surname, ',');
}
```

### Comments

Comments on code SHALL be used lightly and where necessary.

In C99 projects the syntax for comments is:

* In-line comments SHALL use the C++ <tt>//</tt> style.

* Multi-line comments MAY use the C <tt>/* ... */</tt> style or MAY use the C++ style.

In C89 projects the syntax for all comments SHALL be the C <tt>/* ... */</tt> style.

* When possible in-line comments shall start at column 33.

* In in-line comments, the <tt>//</tt> or <tt>/*</tt> shall be followed by two spaces.

* Every function shall have a multi-line comment header that briefly explains its purpose.
* Method comment headers SHALL be preceded by a line of hyphens ending at column 78.
* Suitably-marked-up comments before a function MAY be used as source material for reference documentation.

This is the general template for a method comment header:

```
//  --------------------------------------------------------------------------
//  Finds the first item in the list, returns null if the list is empty.

myp_myclass_t *
myp_myclass_first (myp_myclass_t *self)
{
    ...
```

* Every property in a class structure SHALL have a 1-line in-line comment that describes its purpose.
* Comments SHALL NOT be used to compensate for illegible code.
* Code that cannot be reasonable read and understood by the casual reader SHOULD be rewritten, not annotated.
* Properties and functions whose semantics are not clear from their names SHOULD be renamed, not annotated.

### Blank Lines

Blank lines SHALL be used to separate blocks of code to improve readability, in these cases:

* After the closing bracket of a function body and before the comment header for a function.
* To break up blocks of code that exceed 6-8 lines.
* After assertions at the start of a class body.
* After an <tt>if</tt> statement with a single-statement block.
* After multi-line <tt>case</tt> blocks inside a <tt>switch</tt> statement.
* After multi-line comment blocks.

Blank lines SHALL not be used in these cases:

* After the closing bracket of a conditional block.
* To separate individual lines of code that could better be grouped together.

### Vertical Alignment

Code SHALL NOT use extra spacing to create vertical alignment.

```
char *comma = strchr (surname, ',');
while (comma) {
    *comma = ' ';
    comma = strchr (surname, ',');
}
```

### Punctuation

Punctuation SHALL follow English rules as far as possible.

This is the style for unary operators, with a space after but not before the operator:

```
char_nbr++;
```

This is the style for binary operators, with a space before and after the operator:

```
comma = comma + 1;
```

This is the style for the <tt>?:</tt> operator:

```
comma = comma? comma + 1: strchr (name, '.');
```

This is the style for semi-colons, with a space after but not before:

```
for (char_nbr = 0; *char_nbr; char_nbr++)
    char_nbr++;
```

This is the style for parentheses, with a space before the opening, and after the closing parenthesis, with multiple opening or closing parentheses joined together without spaces:

```
node = (node_t *) zmalloc (sizeof (node_t));
if (!node)
    return -1;
```

This is the style for square brackets:

```
comma = name [char_nbr];
```

This is the style for pointer dereferences, with no space before or after the '->':

```
self->name = strdup (name);
```

### Assertions

Classes SHOULD check the validity of arguments using assertions. That is, misuse of the API is considered a programming error, not a run-time error.

* Assertions SHALL be used for their documentary value, for example to warn the reader, "this argument SHALL NOT be null".

* Assertions on arguments SHALL come at the start of the class body and SHALL follow the order of the arguments.

* Assertions MAY be used on return values from function calls if such failures cannot safely be handled by the code.

* Assertions MAY be used on internal state (e.g., instance properties) to assert a mandatory condition for continuing.

* Assertions SHALL NOT be used to trap errors on external conditions, e.g., bad user input, invalid protocol messages, etc.

* Assertions SHOULD be used to trap errors on internal APIs, e.g. invalid messages sent from one thread to another.

* Assertions SHALL NOT have side-effects since the entire statement may be removed by an optimizing compiler.

### Exiting Functions and Goto Statements

The <tt>return</tt> statement MAY be used at any point in a function to return to the caller.

If the function needs to do clean-up (e.g., free a number of properties), the code MAY use <tt>goto</tt> and a single clean-up block at the end of the function. Such a clean-up block SHALL follow the last "normal" return.

A void function SHALL NOT end in an empty <tt>return</tt> statement.

### Recommended Patterns

* The recommended pattern for an open-ended loop is `while (true) {}`, with <tt>break</tt> statements as needed to exit the loop.

* The recommended pattern for array iteration is:

```
for (array_index = 0; array_index < array_size; array_index++) {
    //  Access element [array_index]
}
```

* The recommended pattern for list iteration is:

```
myp_myclass_t *myclass = (myp_myclass_t *) myp_myclass_first (myclass);
while (myclass) {
    //  Do something
    myclass = (myp_myclass_t *) myp_myclass_next (myclass);
}
```

## Portability

### Portable Versus Native Classes

All projects SHALL depend at least on ZeroMQ (libzmq) and CZMQ (libczmq), which provide portable APIs around networking, threads, file systems, and other aspects.

* A class SHALL be either "portable" or "native".

* A portable class SHALL NOT use the preprocessor to compile differently on different systems.

* A native class SHALL export a properly abstracted API that hides system differences, and SHALL use the preprocessor to compile differently on different systems.

* A native class SHALL use the preprocessor macros defined in <tt>czmq_prelude.h</tt>, and specifically the <tt>__WINDOWS__</tt>, <tt>__UNIX__</tt>, and <tt>__UTYPE_ABC</tt> macros.

* A native class SHALL NOT use preprocessor macros supplied by any specific build system. If the CZMQ-supplied macros are not sufficient these can be improved and extended.

* The project architect SHOULD aim to fully separate portable and native classes, so that application developers see and write only portable classes.

This example shows the general style of native code:

```
#if (defined (__UNIX__))
    pid = GetCurrentProcessId ();
#elif (defined (__WINDOWS__))
    pid = getpid ();
#else
    pid = 0;
#endif
```

### Portable Language

The following types and macros are defined by CZMQ and may be used safely in all code:

* <tt>bool</tt>, <tt>true</tt>, <tt>false</tt>: Boolean data type and constants.
* <tt>byte</tt>, <tt>dbyte</tt>, <tt>qbyte</tt>: unsigned 1-, 2-, and 4-octet integers.
* <tt>uint</tt>, <tt>ulong</tt>: unsigned integers and longs.
* <tt>int32_t</tt>, <tt>int64_t</tt>: signed 32-bit and 64-bit integers.
* <tt>uint32_t</tt>, <tt>uint64_t</tt>: unsigned 32-bit and 64-bit integers.
* <tt>streq (s1, s2)</tt>: preferred over <tt>strcmp (s1, s2) == 0</tt>.
* <tt>strneq (s1, s2)</tt>: preferred over <tt>strcmp (s1, s2) != 0</tt>.
* <tt>randof (number)</tt>: return random integer in range 0 .. number - 1.
* <tt>srandom</tt>: typically used like this: <tt>srandom ((unsigned) time (NULL));</tt>
* <tt>inline</tt>, <tt>snprintf</tt>, <tt>vsnprintf</tt>: Windows uses non-POSIX variants with underscores.

### Compiler Warnings

Compiler warnings SHOULD always be treated as fatal. The following is a list of constructs known to cause warnings on some but not all compilers:

* Assigning a void pointer to a typed pointer without a cast. Always cast a void * before assigning it to a typed pointer.

* Failing to return a value in a non-void function. Always end a non-void function with a <tt>return</tt> statement.

## Code Generation

Code generation MAY be used to produce classes mechanically when there is compelling benefit.

* The code generator SHOULD be GSL, from https://github.com/imatix/gsl.

* All code generation scripts SHALL be in the project <tt>src</tt> subdirectory.

* All model data (XML files) SHALL be in the project <tt>src</tt> directory.

* If only parts of a class are generated, these parts SHALL have the extension <tt>.inc</tt> and SHALL be generated into the project <tt>src</tt> directory, and SHALL be included in the class source file using an #include statement.

* Code generation SHALL be done as a manual step. For example, "make code." All generated code SHALL be committed into the project as for hand-written files.

* Code generation SHALL be fully idempotent, that is, generated code SHALL NOT contain any date or time stamps.

* Code generation SHALL be treated as a form of dangerous abstraction that creates significant barriers to readers. A good rule of thumb is that for code generation to be profitable, it should reduce the lines of code written by hand by 80-90%.

* Generated code SHALL contain a warning of this form at the start: "GENERATED SOURCE CODE, DO NOT EDIT".

* Generated code SHALL otherwise conform to this Specification so that it is indistinguishable from hand-written code.

## Security Aspects

### Thread Safety

The use of opaque data structures that are accessed via references is thread safe. However:

* Code SHALL NOT share state between threads except in exceptional and limited cases. Threads SHALL communicate by passing 0MQ messages.

* Classes SHALL not use static variables since this is not re-entrant, thus not thread safe.

* Class instances SHALL NOT be passed between threads except in "hand-off" cases.

* Code SHOULD NOT use mutexes, locks, or other mechanisms to share state between threads.

* Code MUST NOT use non-thread safe system calls such as <tt>basename ()</tt>.

### Buffer Overflows

* Code MUST always truncate over-long data.

* Code MUST NOT use unsafe system calls such as <tt>gets ()</tt>.

### Known Weaknesses

* The heavy reliance on heap memory means that CLASS applications are vulnerable to denial-of-service attacks. Applications can mitigate this risk by enforcing limits on the number of class instances they create.

* The heavy reliance on heap memory makes CLASS unsuitable for embedded systems where all memory use must be static.

* In most CLASS applications it is difficult to handle an "out of memory" error in any way except to abort.
