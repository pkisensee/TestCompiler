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
    for( int i = 0; i < 10; i = i + 1 ) \n\
      print 'fib(' + i + ') -> ' + fib(i);\
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
  Chunk chunk;
  Chunk::LineCount line = 123;
  auto constant = chunk.AddConstant( Value{ 12 } );
  chunk.Append( OpCode::Constant, line );
  chunk.Append( constant, line );

  constant = chunk.AddConstant( Value{ 34 } );
  chunk.Append( OpCode::Constant, line );
  chunk.Append( constant, line );
  chunk.Append( OpCode::Add, line );

  constant = chunk.AddConstant( Value{ 6 } );
  chunk.Append( OpCode::Constant, line );
  chunk.Append( constant, line );

  chunk.Append( OpCode::Divide, line );
  chunk.Append( OpCode::Negate, line );
  chunk.Append( OpCode::Return, line );
  chunk.Disassemble( "Test" );
  vm.Interpret( &chunk ); // -7

  chunk.Free();
  chunk.Disassemble( "Empty" );

  Compiler compiler;
  compiler.Compile( "print (42 + 1) / (2 * 3) - 4;", &chunk ); // 43/6 - 4 == 3
  chunk.Disassemble( "Math ops" );
  vm.Reset();
  vm.Interpret( &chunk ); // 3

  chunk.Free();
  //                   1   -   0   +     1    -        1            -    1   +     0     == 0
  compiler.Compile( "print (5>2) - (6<4) + (-7>=-7) - (3 == (3*(1 <= 8))) - !!true + !!false;", &chunk ); 
  chunk.Disassemble( "Logical ops" );
  vm.Reset();
  vm.Interpret( &chunk ); // 0

  chunk.Free();
  compiler.Compile( "print 'foo' != 'goofball' ;", &chunk ); // true
  chunk.Disassemble( "String comparison" );
  vm.Reset();
  vm.Interpret( &chunk ); // true

  chunk.Free();
  std::string_view bevvies =
    "str bev = 'tea';\n"
    "print 'beignets with ' + bev;\n"
    "bev = 'coffee';\n"
    "print 'beignets with ' + bev;";
  compiler.Compile( bevvies, &chunk );
  chunk.Disassemble( "String addition" );
  vm.Reset();
  vm.Interpret( &chunk ); // "beignets with tea/coffee"

  chunk.Free();
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
  compiler.Compile( locals, &chunk );
  chunk.Disassemble( "Locals" );
  vm.Reset();
  vm.Interpret( &chunk ); // 2, 3, 1

  chunk.Free();
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
  compiler.Compile( branch, &chunk );
  chunk.Disassemble( "Branch" );
  vm.Reset();
  vm.Interpret( &chunk ); // 1+1 == 2, 2 != 1-1

  chunk.Free();
  std::string_view orCode =
    "bool t = true;  \n"
    "bool f = false; \n"
    "print t or t;   \n"
    "print t or f;   \n"
    "print f or t;   \n"
    "print f or f;   \n"
    ;
  compiler.Compile( orCode, &chunk );
  chunk.Disassemble( "Or" );
  vm.Reset();
  vm.Interpret( &chunk ); // T T T F

  chunk.Free();
  std::string_view andCode =
    "bool t = true;  \n"
    "bool f = false; \n"
    "print t and t;  \n"
    "print t and f;  \n"
    "print f and t;  \n"
    "print f and f;  \n"
    ;
  compiler.Compile( andCode, &chunk );
  chunk.Disassemble( "And" );
  vm.Reset();
  vm.Interpret( &chunk ); // T F F F

  chunk.Free();
  std::string_view whileLoop =
    "int c = 0;     \n"
    "while( c < 3 ) \n"
    "{              \n"
    "  c = c + 1;   \n"
    "}              \n"
    "print c;       \n"
    ;
  compiler.Compile( whileLoop, &chunk );
  chunk.Disassemble( "While" );
  vm.Reset();
  vm.Interpret( &chunk ); // 3

  chunk.Free();
  std::string_view forLoop =
    "for (int c = 0; c < 3; c = c + 1) \n"
    "  print c;                        \n"
    ;
  compiler.Compile( forLoop, &chunk );
  chunk.Disassemble( "For" );
  vm.Reset();
  vm.Interpret( &chunk ); // 0 1 2

  chunk.Free();
  std::string_view emptyForLoop =
    "int c = 0;       \n"
    "for ( ; c < 3; ) \n"
    "{                \n"
    "  print c;       \n"
    "  c = c + 1;     \n"
    "}                \n"
    ;
  compiler.Compile( emptyForLoop, &chunk );
  chunk.Disassemble( "Empty For" );
  vm.Reset();
  vm.Interpret( &chunk ); // 0 1 2
}

///////////////////////////////////////////////////////////////////////////////
