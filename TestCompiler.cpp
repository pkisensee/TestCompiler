///////////////////////////////////////////////////////////////////////////////
//
//  TestCompiler.cpp
//
//  Copyright © Pete Isensee (PKIsensee@msn.com).
//  All rights reserved worldwide.
//
//  Permission to copy, modify, reproduce or redistribute this source code is
//  granted provided the above copyright notice is retained in the resulting 
//  source code.
// 
//  This software is provided "as is" and without any express or implied
//  warranties.
//
///////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <iostream>
#include <sstream>

#include "Chunk.h"
#include "File.h"
#include "Interpreter.h"
#include "Parser.h"
#include "Value.h"
#include "VirtualMachine.h"
#include "Util.h"

using namespace PKIsensee;

#ifdef _DEBUG
#define test(e) assert(e)
#else
#define test(e) static_cast<void>( (e) || ( Util::DebugBreak(), 0 ) )
#endif

int __cdecl main()
{
  /*
  _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF |        // enable debug heap allocations
                  _CRTDBG_DELAY_FREE_MEM_DF |   // enable detect of use after free
                  _CRTDBG_LEAK_CHECK_DF |       // enable leak checking at program exit
                  _CRTDBG_CHECK_ALWAYS_DF );    // call _CrtCheckMemory at every alloc/free
  */
  Parser parser;

  parser.Parse( "[80s Pop]{ Genre=='Pop' and Year > 1990 and Year<=2000 }" );
  std::cout << parser << '\n';
  test( parser.AllTokensValid() );
  size_t i = 0;
  test( parser.GetToken( i++ ) == Token( TokenType::OpenBracket, "[" ) );
  test( parser.GetToken( i++ ) == Token( TokenType::Number, "80" ) );
  test( parser.GetToken( i++ ) == Token( TokenType::Identifier, "s" ) );
  test( parser.GetToken( i++ ) == Token( TokenType::Identifier, "Pop" ) );
  test( parser.GetToken( i++ ) == Token( TokenType::CloseBracket, "]" ) );
  test( parser.GetToken( i++ ) == Token( TokenType::OpenBrace, "{" ) );
  test( parser.GetToken( i++ ) == Token( TokenType::Identifier, "Genre" ) );
  test( parser.GetToken( i++ ) == Token( TokenType::IsEqual, "==" ) );
  test( parser.GetToken( i++ ) == Token( TokenType::String, "Pop" ) );
  test( parser.GetToken( i++ ) == Token( TokenType::And, "and" ) );
  test( parser.GetToken( i++ ) == Token( TokenType::Identifier, "Year" ) );
  test( parser.GetToken( i++ ) == Token( TokenType::GreaterThan, ">" ) );
  test( parser.GetToken( i++ ) == Token( TokenType::Number, "1990" ) );
  test( parser.GetToken( i++ ) == Token( TokenType::And, "and" ) );
  test( parser.GetToken( i++ ) == Token( TokenType::Identifier, "Year" ) );
  test( parser.GetToken( i++ ) == Token( TokenType::LessThanEqual, "<=" ) );
  test( parser.GetToken( i++ ) == Token( TokenType::Number, "2000" ) );
  test( parser.GetToken( i++ ) == Token( TokenType::CloseBrace, "}" ) );

  parser.Parse( "[Dinner]{ return Mood != 'Dinner'; }" );
  std::cout << parser << '\n';
  test( parser.AllTokensValid() );

  parser.Parse( "[By Composer](string composer){ Composer == composer }" );
  std::cout << parser << '\n';
  test( parser.AllTokensValid() );

  parser.Parse( "'String Literal A', \"String Literal B\"" );
  std::cout << parser << '\n';
  test( parser.AllTokensValid() );

  parser.Parse( "orchid == nottingham" ); // don't mistake keywords or and not in identifiers
  std::cout << parser << '\n';
  test( parser.AllTokensValid() );
  i = 0;
  test( parser.GetToken( i++ ) == Token( TokenType::Identifier, "orchid" ) );
  test( parser.GetToken( i++ ) == Token( TokenType::IsEqual, "==" ) );
  test( parser.GetToken( i++ ) == Token( TokenType::Identifier, "nottingham" ) );

  parser.Parse( "facts0_ == true" );
  std::cout << parser << '\n';
  test( parser.AllTokensValid() );
  i = 0;
  test( parser.GetToken( i++ ) == Token( TokenType::Identifier, "facts0_" ) );
  test( parser.GetToken( i++ ) == Token( TokenType::IsEqual, "==" ) );
  test( parser.GetToken( i++ ) == Token( TokenType::True, "true" ) );

  parser.Parse( "foo.bar = 42 + 1 / 24 * 16 + false;" );
  std::cout << parser << '\n';
  test( parser.AllTokensValid() );
  test( parser.GetToken( 1 ) == Token( TokenType::Dot, "." ) );
  test( parser.GetToken( 5 ) == Token( TokenType::Plus, "+" ) );
  test( parser.GetToken( 8 ) == Token( TokenType::Number, "24" ) );
  test( parser.GetToken( 12 ) == Token( TokenType::False, "false" ) );

  parser.Parse( "1 + 2 * 3" );
  std::cout << parser;
  test( parser.AllTokensValid() );
  test( parser.GetToken( 3 ) == Token( TokenType::Multiply, "*" ) );
  auto ast = parser.GetAST();
  std::stringstream strStream;
  strStream << *ast;
  std::cout << strStream.str() << '\n';
  test( strStream.str() ==
    "+ [Plus]\n"
    "  1\n"
    "  * [Multiply]\n"
    "    2\n"
    "    3\n" );

  parser.Parse( "(42 + 1) / (2 * 3)" );
  std::cout << parser;
  test( parser.AllTokensValid() );
  test( parser.GetToken( 0 ) == Token( TokenType::OpenParen, "(" ) );
  test( parser.GetToken( 3 ) == Token( TokenType::Number, "1" ) );
  ast = parser.GetAST();
  strStream = {};
  strStream << *ast;
  std::cout << strStream.str();
  test( strStream.str() ==
    "/ [Divide]\n"
    "  + [Plus]\n"
    "    42\n"
    "    1\n"
    "  * [Multiply]\n"
    "    2\n"
    "    3\n" );
  Interpreter interpreter;
  Value result = interpreter.Evaluate( ast->GetRoot() );
  std::cout << "Result: " << result << '\n';
  test( result == Value{ 7 } );
  std::cout << '\n';

  parser.Parse( "\"id\" + \"42\"" );
  std::cout << parser;
  test( parser.AllTokensValid() );
  test( parser.GetToken( 2 ) == Token( TokenType::String, "42" ) );
  ast = parser.GetAST();
  strStream = {};
  strStream << *ast;
  std::cout << strStream.str();
  test( strStream.str() ==
    "+ [Plus]\n"
    "  \"id\"\n"
    "  \"42\"\n" );
  result = interpreter.Evaluate( ast->GetRoot() );
  std::cout << "Result: " << result << '\n';
  test( result == Value{ std::string( "id42" ) } );
  std::cout << '\n';

  parser.Parse( "(42.42 + \"str\") / \"23\" * true" );
  std::cout << parser;
  test( parser.AllTokensValid() );
  ast = parser.GetAST();
  test( ast.has_value() );
  //result = interpreter.Evaluate( ast->GetRoot() );
  //test( !result.has_value() );
  //std::cout << "Result: " << result.error().GetErrorMessage() << '\n';
  //std::cout << '\n';

  parser.Parse( "(42 + \"0\") / \"23\" * true" );
  std::cout << parser;
  test( parser.AllTokensValid() );
  ast = parser.GetAST();
  test( ast.has_value() );
  result = interpreter.Evaluate( ast->GetRoot() );
  std::cout << "Result: " << result << '\n';
  test( result == Value{ 1 } );
  std::cout << '\n';

  std::string_view fib = "                \
  // fibonacci sequence                 \n\
  fun fib( int i )                      \n\
  {                                     \n\
    if( i <= 1 )                        \n\
      return i;                         \n\
    return fib( i - 2 ) + fib( i - 1 ); \n\
  }                                     \n\
  fun main()                            \n\
  {                                     \n\
    int start = clock();                \n\
    for( int i = 0; i < 10; i = i + 1 ) \n\
      print 'fib(' + i + ') -> ' + fib(i);\n\
    print 'fib time=' + (clock()-start) + ' ns';\n\
  }                                     \n\
  main();";

  std::string_view exp = "    \
  fun exp(int i)              \
  {                           \
    if( i == 0 )              \
      return 1;               \
    return i * exp( i - 1 );  \
  }                           \
  fun main()                  \
  {                           \
    print exp( 1 );           \
  }                           \
  main();";

  parser.Parse( fib );
  auto statements = parser.GetStatements();
  interpreter.Execute( *statements );

  std::string_view eightiesPop = "  \
  fun IsEightiesPop()               \
  {                                 \
    return ( genre == 'Rock' );     \
  }                                 \
  print IsEightiesPop();";

  // Handle common error conditions
  parser.Parse( "(" );
  std::cout << parser;
  ast = parser.GetAST();
  test( !ast );
  std::cout << ast.error().GetErrorMessage() << "\n\n";

  parser.Parse( ")" );
  std::cout << parser;
  ast = parser.GetAST();
  test( !ast );
  std::cout << ast.error().GetErrorMessage() << "\n\n";

  VirtualMachine vm;
  vm.Interpret( "print (42 + 1) / (2 * 3) - 4;" ); // 3

  //                   1   -   0   +     1    -        1            -    1   +     0     == 0
  vm.Reset();
  vm.Interpret( "print (5>2) - (6<4) + (-7>=-7) - (3 == (3*(1 <= 8))) - !!true + !!false;" ); // 0

  vm.Reset();
  vm.Interpret( "print 'foo' != 'goofball' ;" ); // true

  std::string_view bevvies =
    "str bev = 'tea';\n"
    "print 'beignets with ' + bev;\n"
    "bev = 'coffee';\n"
    "print 'beignets with ' + bev;";
  vm.Reset();
  vm.Interpret( bevvies ); // "beignets with tea/coffee"

  std::string_view locals =
    "{\n"
    "  int a = 1;\n"
    "  {\n"
    "    int a = 2;\n"
    "    int b = 3;\n"
    "    print a;\n"
    "    print b;\n"
    "  }\n"
    "  print a;\n"
    "}";
  vm.Reset();
  vm.Interpret( locals ); // 2, 3, 1

  std::string_view branch =
    "if ( 1 + 1 == 2)\n"
    "  print '1+1 == 2';\n"
    "else\n"
    "  print '1+1 != 2';\n"
    "if ( 2 == (1-1) )\n"
    "  print '2 == 1-1';\n"
    "else\n"
    "  print '2 != 1-1';\n"
    ;
  vm.Reset();
  vm.Interpret( branch ); // 1+1 == 2, 2 != 1-1

  std::string_view orCode =
    "bool t = true;  \n"
    "bool f = false; \n"
    "print t or t;   \n"
    "print t or f;   \n"
    "print f or t;   \n"
    "print f or f;   \n"
    ;
  vm.Reset();
  vm.Interpret( orCode ); // T T T F

  std::string_view andCode =
    "bool t = true;  \n"
    "bool f = false; \n"
    "print t and t;  \n"
    "print t and f;  \n"
    "print f and t;  \n"
    "print f and f;  \n"
    ;
  vm.Reset();
  vm.Interpret( andCode ); // T F F F

  std::string_view whileLoop =
    "int c = 0;     \n"
    "while( c < 3 ) \n"
    "{              \n"
    "  c = c + 1;   \n"
    "}              \n"
    "print c;       \n"
    ;
  vm.Reset();
  vm.Interpret( whileLoop ); // 3

  std::string_view forLoop =
    "for (int c = 0; c < 3; c = c + 1) \n"
    "  print c;                        \n"
    ;
  vm.Reset();
  vm.Interpret( forLoop ); // 0 1 2

  std::string_view emptyForLoop =
    "int c = 0;       \n"
    "for ( ; c < 3; ) \n"
    "{                \n"
    "  print c;       \n"
    "  c = c + 1;     \n"
    "}                \n"
    ;
  vm.Reset();
  vm.Interpret( emptyForLoop ); // 0 1 2

  std::string_view nativeFnCall1 =
    "int start = clock();        \n"
    "print 'start';              \n"
    "int end = clock();          \n"
    "print 'elapsed=' + (end-start); \n"
    ;
  vm.Reset();
  vm.Interpret( nativeFnCall1 ); // "end=[nanoseconds]"

  std::string_view nativeFnCall2 =
    "int start = clock();        \n"
    "print square(42);           \n"
    "int end = clock();          \n"
    "print 'elapsed=' + (end-start); \n"
    ;
  vm.Reset();
  vm.Interpret( nativeFnCall2 ); // "1764 end=[nanoseconds]"

  std::string_view simpleFnCall1 =
    "fun sum(int a, int b) { \n"
    "  return a + b;         \n"
    "}                       \n"
    "                        \n"
    "print 4 + sum(42, 3);   \n"
    ;
  vm.Reset();
  vm.Interpret( simpleFnCall1 ); // 49

  std::string_view simpleFnCall2 =
    "fun foo() {       \n"
    "  print 'fnCall'; \n"
    "}                 \n"
    "                  \n"
    "foo();            \n"
    ;
  vm.Reset();
  vm.Interpret( simpleFnCall2 ); // "fnCall"

  vm.Reset();
  // vm.Interpret( fib ); // 0,1,1,2,3,5,8,13,21,34 TODO re-enable later

  vm.Reset();
  std::string intToHex;
  File::ReadEntireFile( "IntToHex.c", intToHex );
  vm.Interpret( intToHex ); // "empty",50,65,51,48,51,57

  std::string_view captures1 =
    "fun outer() {\n"
    "  int a = 1;\n"
    "  int b = 2;\n"
    "  fun middle() {\n"
    "    int c = 3;\n"
    "    int d = 4;\n"
    "    fun inner() {\n"
    "      print a + c + b + d;\n"
    "    }\n"
    "    inner();\n"
    "  }\n"
    "  middle();\n"
    "}\n"
    "outer();\n"
    ;
  vm.Reset();
  vm.Interpret( captures1 ); // 10

  std::string_view captures2 =
    "fun outer() {\n"
    "  str x = 'outside';\n"
    "  fun inner() {\n"
    "    print x;\n"
    "  }\n"
    "  inner();\n"
    "}\n"
    "outer();\n"
    ;
  vm.Reset();
  vm.Interpret( captures2 ); // "outside"

  std::string_view funref1 =
    "fun printVal(str value) {\n"
    "  fun inner() {\n"
    "    print value;\n"
    "  }\n"
    "  return inner;\n"
    " }\n"
    "funref doughnut = printVal('doughnut');\n"
    "funref bagel = printVal('bagel');\n"
    "doughnut();\n"
    "bagel();\n"
    ;
  vm.Reset();
  vm.Interpret( funref1 ); // "doughnut bagel"

  /*
  std::string_view captures3 =
    "fun outer() {\n"
    "  str x = 'outside';\n"
    "  fun inner() {\n"
    "    print x;\n"
    "  }\n"
    "  return inner();\n"
    "}\n"
    "funref closure = outer();\n"
    "closure();\n"
    ;
  vm.Reset();
  vm.Interpret( captures3 ); // "outside"
  */
}

///////////////////////////////////////////////////////////////////////////////
