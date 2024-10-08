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

  parser.Parse( "bar = 42 + 1 / 24 * 16;" );
  std::cout << parser << '\n';
  test( parser.AllTokensValid() );
  test( parser.GetToken( 3 ) == Token( TokenType::Plus, "+" ) );
  test( parser.GetToken( 6 ) == Token( TokenType::Number, "24" ) );

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
    "  1 [Number]\n"
    "  * [Multiply]\n"
    "    2 [Number]\n"
    "    3 [Number]\n" );

  parser.Parse( "42 + 1 / (24 * 16)" );
  std::cout << parser;
  test( parser.AllTokensValid() );
  test( parser.GetToken( 0 ) == Token( TokenType::Number, "42" ) );
  test( parser.GetToken( 4 ) == Token( TokenType::OpenParen, "(" ) );
  ast = parser.GetAST();
  strStream = {};
  strStream << *ast;
  std::cout << strStream.str();
  test( strStream.str() ==
        "+ [Plus]\n"
        "  42 [Number]\n"
        "  / [Divide]\n"
        "    1 [Number]\n"
        "    * [Multiply]\n"
        "      24 [Number]\n"
        "      16 [Number]\n" );
  std::cout << '\n';

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
