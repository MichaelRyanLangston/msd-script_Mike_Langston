Hi there, new user! Welcome to MSDscript! :) MSDscript is a programing language, parser, and interpreter package.
 
	Included in this document:

		- Set Up: General set up of MSDscript

			. using cmake and make

		- User Guide: 

			. Using MSDscript as a parser/interpreter to run scripts written in MSDscript.

			. Using MSDscript as a library
		
		- MSDscript Language:

			. Current abstract grammar

			. Brief description of 

	Note: Comments in the source files (.cpp) and header files (.h/.hpp) are also available for specific classes utilized by MSDscript. Please see these for a more detailed description of operation of MSD script.



Set Up

	For the initial set up of MSDscript please follow the step by step instructions.

		1. Place the provided .zip file into the desired folder on your Computer.
		
		2. Decompress the .zip file to get a folder containing all relevant files. This folder may act as the working directory for your project if you so desire. Other wise you will need to specify file paths to relevant files for use in your project.

		3. Navigate into the folder produced by the .zip file. This file should include:

			- CMakeLists.txt

			- read_me.txt
			
			- MSDscriptCommandLine

			- RandomizedTestGenerator

			- which_day.cpp

		4. Type the following command into the command line while in the same folder as the CMakeLists.txt:

			cmake .

		5. Once "cmake ." has finished execution you should have the following files:

			- CMakeCache.txt

			- CMakeFiles

			- cmake_install.cmake

			- Makefile

		6. Type the following command into the command line: 

			make

		7. Once "make" has finished execution you should now have the following files:

			- msdscript (an executable)

			- libmsd.a (a library)

	If you've made it this far, congratulations you now have a working build of MSDscript that you may use! 

	If not follow these trouble shooting guidelines:

		- In the CMakeLists.txt file, check to make sure that you are using a compatible version of cmake (minimum Version 3.10) and c++ ( minimum version -std-c++14)

		- If trouble still persists please submit a bug report describing the issue to the following GitHub repo: https://github.com/UtahMSD/msd-script_Mike_Langston



User Guide
	
	Using "msdscript"

		The executable gleaned in the last portion of the previous set up section (msdscript) is a fully functioning parser and interpreter for the MSDscript language. You can run the executable using the following in the command line:

			- ./msdscript [your command line argument here...]

		You may pass the following items as an argument to MSDscript:

			--help:
				Displays this message.

			--test:
				Runs all tests located in test.cpp.
			
			--interp:
				Takes the given argument, interprets it, and exits with 0 if successful. This mode doesn't prevent stack overflow because of its recursive nature. Not recommend for recursive programs.

			--step: 
				Takes the given argument and interprets it, while preventing stack overflow, and exits with 0 if successful. Recommended for recursive programs.

			--print: 
				Takes the given argument, prints it, and exits with 0 if successful.

		Arguments that are passed and not specified above result in an "Invalid command" error. For information on writing programs in MSDscript, please see the MSDscript Language section in this document. The --step and --interp modes can be used to run programs written in MSDscript by doing the following:

			1. Run MSDscript with either the --interp or --step flag.

			2. Type in your program in the MSDscript Language.

			3. Once finished press enter and control D to run the program entered.

	Using "libmsd.a"

		The "libmsd.a" library contains the main files that are needed to include parsing and interpreting functions into your own project. Additionally the base project needs to be written in either c or c++ in order to call the appropriate commands for the parser and interpreter. To do this you must:

			1. Add the following .h files into your project:
				
				. #include "expr.h"
				
				. #include "parse.h"

				. #include "step.h"

			2. Include either of the following two commands into your project as appropriate:

				. To run using step mode:

					std::cout << Step::interp_by_steps(parse_expr(input))->make_string() + "\n";

				. To run using interpret mode:

					 std::cout << e->interp(EmptyEnv::empty)->make_string() + "\n";

			3. When compiling, add in the following flag to allow the compiler to find needed files:

				-I MSDscriptCommandLine/MSDscriptCommandLine

		To see an example of this please see lines 42 - 44 and lines 76 - 80 in the included which_day.cpp.



MSDscript Language

	The abstract grammar follows the following format. This gives a basic over view of the language as a whole. MSDscript is based off of expressions, mathematical or otherwise. An expression can be the following items:

		<expression> =	<number>  (Number Expression: NumExpr)

				<boolean>  (Boolean Expression: BoolExpr)

				<variable>  (Variable Expression: VarExpr)

				<expression> == <expression>  (Equals Expression: EqExpr)

				<expression> + <expression>  (Addition Expression: AddExpr)

				<expression> * <expression>  (Multiplication Expression: MultExpr)

				_let <variable> = <expression> _in <expression>  (Let Expression: LetExpr)

				_if <expression> _then <expression> _else <expression>  (If Expression: IfExpr)

				_fun ( <variable> ) <expression> (Function Expression: FunExpr)

				<expression> ( <expression> )  (Function Call Expression: CallExpr)

	Most of the Language is right associative, meaning that generally the right hand side of an expression is evaluated first, barring call expressions which are left associative. In the case of a let expression the right hand side of the expression is considered to be the expression to the right of the <variable> a brief explanation and example of each class follows:

		Number Expression:

				A number representation of a value.
					
				Examples:

						4
						
						12234

		Boolean Expression:

				A representation of true or false.
					
				Examples:

						_true 
						
						_false

		Variable Expression:

				A variable representation of an unknown value. Can be a single character or multiple characters.
					
				Examples:

						x

						variable

		Equals Expression:

				An expression that evaluates if two expressions are considered equal. Returns a boolean.

				Examples:

						x == 2

						_true == _false

		Addition Expression:

				An expression that adds two expressions. Follows general PEMDAS order of operations.

				Examples:

						2 + 2

						4 + 5 + 6

		Multiplication Expression:

				An expression that multiplies two expressions. Follows general PEMDAS order of operations.

				Examples:

						2 * 2

						4 * 5 * 6

		Let Expression:

				An expression used to declare a variable name for another expression to be used in another expression.

				Example:
						
						_let x = 3 _in x + 3

		If Expression:

				An expression used to implement basic logic (an if statement). If the first expression returns true then the _then portion is executed. Otherwise the _else is executed. 

				Example:

						_if 6 == 4
						_then 6
						_else 4

		Function Expression:

				An expression used to represent a function.

				Examples:

						_fun (x) x + 4

		Function Call Expression:

				An expression used to call a function. Note: you may bind the function to variable if desired.

				Examples:

						f(5)

						_let function = _fun (y) y + 4 _in function(6)

	The following is an examples of  how recursive programs that could be written.

		This function calculates the factorial of a number:

			_let factrl = _fun (factrl)

						_fun (x) 

							_if x == 1 

							_then 1 

							_else x * factrl(factrl)(x + -1)

			_in  factrl(factrl)(10)



Please submit any issues or problems to the bug reporting at the following GitHub:

	https://github.com/UtahMSD/msd-script_Mike_Langston	

Thank you for downloading MSDscript! Enjoy!!


											---End of Document---
			