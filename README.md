# Tiny-Compiler

## Authors
- **Kenny Nguyen**
- **Jared Zayas**

## Overview
This project is a simple compiler program designed to perform lexical analysis, parsing, and code generation for a specified programming language. The compiler processes an input source file, conducts the necessary analysis and generation steps, and produces a machine code output file.

## Features
- **Lexical Analysis**: Tokenizes the input source code into meaningful symbols.
- **Parsing**: Analyzes the tokenized symbols according to the grammar of the language.
- **Code Generation**: Produces machine code based on the parsed symbols.

## Usage
### Prerequisites
Ensure you have the necessary environment to compile and run the program, such as a C compiler (e.g., gcc).

### Steps to Compile and Run
1. **Open the command line or terminal**: 
   Open your terminal or command prompt.

2. **Navigate to the program directory**: 
   Change to the directory where the program source code is located.

3. **Compile the source code**:
  Use the appropriate compiler command to compile the source code. For example:
  gcc parsercodegen.c -o parsercodegen

4. **Run the compiled program**:
  Execute the compiled program, specifying the input source file as a command-line argument. For example:
  "./parsercodegen input.txt"

5. **Check the terminal output**:
  Monitor the terminal for any compilation errors or warnings. The program will display messages indicating the success of the compilation process or any encountered issues.

6. **Output file generation**:
  If the compilation is successful, the program will generate a machine code output file. The filename and format will be specified in the assignment instructions or as defined within the code.
