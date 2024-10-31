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

#include "Compiler.h"
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

  parser.Parse( "'String Literal A' \"String Literal B\"" );
  std::cout << parser << '\n';
  test( parser.AllTokensValid() );

  parser.Parse( "orchid == nottingham" );
  std::cout << parser << '\n';
  test( parser.AllTokensValid() );
  i = 0;
  test( parser.GetToken( i++ ) == Token( TokenType::Identifier, "orchid" ) );
  test( parser.GetToken( i++ ) == Token( TokenType::IsEqual, "==" ) );
  test( parser.GetToken( i++ ) == Token( TokenType::Identifier, "nottingham" ) );

  parser.Parse( "facts == true" );
  std::cout << parser << '\n';
  test( parser.AllTokensValid() );
  i = 0;
  test( parser.GetToken( i++ ) == Token( TokenType::Identifier, "facts" ) );
  test( parser.GetToken( i++ ) == Token( TokenType::IsEqual, "==" ) );
  test( parser.GetToken( i++ ) == Token( TokenType::True, "true" ) );

  parser.Parse( "bar = 42 + 1 / 24 * 16 + false;" );
  std::cout << parser << '\n';
  test( parser.AllTokensValid() );
  test( parser.GetToken( 3 ) == Token( TokenType::Plus, "+" ) );
  test( parser.GetToken( 6 ) == Token( TokenType::Number, "24" ) );
  test( parser.GetToken( 10 ) == Token( TokenType::False, "false" ) );

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

  parser.Parse( "(42 + \"str\") / \"23\" * true" );
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
  fun fib( int i )                        \
  {                                       \
    if( i <= 1 )                          \
      return i;                           \
    return fib( i - 2 ) + fib( i - 1 );   \
  }                                       \
  fun main()                              \
  {                                       \
    for( int i = 0; i < 10; i = i + 1 )   \
      print 'fib(' + i + ') -> ' + fib(i);\
  }                                       \
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
}

///////////////////////////////////////////////////////////////////////////////
