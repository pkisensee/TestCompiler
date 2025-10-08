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

#include "ByteCodeBlock.h"
#include "File.h"
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
  std::string_view fib = "                      \
  // fibonacci sequence                         \n\
  fun fib( int i )                              \n\
  {                                             \n\
    if( i <= 1 )                                \n\
      return i;                                 \n\
    return fib( i - 2 ) + fib( i - 1 );         \n\
  }                                             \n\
  fun main()                                    \n\
  {                                             \n\
    int start = clock();                        \n\
    for( int i = 0; i < 6; i = i + 1 )          \n\
      print 'fib(' + i + ') -> ' + fib(i);      \n\
    print 'fib time=' + (clock()-start) + ' ns';\n\
  }                                             \n\
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

  std::string_view eightiesPop = "  \
  fun IsEightiesPop()               \
  {                                 \
    return ( genre() == 'Rock' );     \
  }                                 \
  print IsEightiesPop();";

  VirtualMachine vm;
  vm.Interpret( "print (42 + 1) / (2 * 3) - 4;" );
  test( vm.GetOutput() == "3" );

  //                   1   -   0   +     1    -        1            -    1   +     0     == 0
  vm.Reset();
  vm.Interpret( "print (5>2) - (6<4) + (-7>=-7) - (3 == (3*(1 <= 8))) - !!true + !!false;" ); // 0
  test( vm.GetOutput() == "0" );

  vm.Reset();
  vm.Interpret( "print 'foo' != 'goofball' ;" );
  test( vm.GetOutput() == "true" );

  std::string_view bevvies =
    "str bev = 'tea';\n"
    "print 'beignets with ' + bev;\n"
    "bev = 'coffee';\n"
    "print 'beignets with ' + bev;";
  vm.Reset();
  vm.Interpret( bevvies );
  test( vm.GetOutput() == "beignets with tea\nbeignets with coffee" );

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
  vm.Interpret( locals );
  test( vm.GetOutput() == "2\n3\n1" );

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
  vm.Interpret( branch );
  test( vm.GetOutput() == "1+1 == 2\n2 != 1-1" );

  std::string_view orCode =
    "bool t = true;  \n"
    "bool f = false; \n"
    "print t or t;   \n"
    "print t or f;   \n"
    "print f or t;   \n"
    "print f or f;   \n"
    ;
  vm.Reset();
  vm.Interpret( orCode );
  test( vm.GetOutput() == "true\ntrue\ntrue\nfalse" );

  std::string_view andCode =
    "bool t = true;  \n"
    "bool f = false; \n"
    "print t and t;  \n"
    "print t and f;  \n"
    "print f and t;  \n"
    "print f and f;  \n"
    ;
  vm.Reset();
  vm.Interpret( andCode );
  test( vm.GetOutput() == "true\nfalse\nfalse\nfalse" );

  std::string_view whileLoop =
    "int c = 0;     \n"
    "while( c < 3 ) \n"
    "{              \n"
    "  c = c + 1;   \n"
    "}              \n"
    "print c;       \n"
    ;
  vm.Reset();
  vm.Interpret( whileLoop );
  test( vm.GetOutput() == "3" );

  std::string_view forLoop =
    "for (int c = 0; c < 3; c = c + 1) \n"
    "  print c;                        \n"
    ;
  vm.Reset();
  vm.Interpret( forLoop );
  test( vm.GetOutput() == "0\n1\n2" );

  std::string_view emptyForLoop =
    "int c = 0;       \n"
    "for ( ; c < 3; ) \n"
    "{                \n"
    "  print c;       \n"
    "  c = c + 1;     \n"
    "}                \n"
    ;
  vm.Reset();
  vm.Interpret( emptyForLoop );
  test( vm.GetOutput() == "0\n1\n2" );

  std::string_view nativeFnCall1 =
    "int start = clock();        \n"
    "print 'start';              \n"
    "int end = clock();          \n"
    "print 'elapsed=' + (end-start); \n"
    ;
  vm.Reset();
  vm.Interpret( nativeFnCall1 );
  test( vm.GetOutput().starts_with( "start\nelapsed=" ) );
  test( std::isdigit( vm.GetOutput()[ 14 ] ) );

  std::string_view nativeFnCall2 =
    "int start = clock();        \n"
    "print square(42);           \n"
    "int end = clock();          \n"
    "print 'elapsed=' + (end-start); \n"
    ;
  vm.Reset();
  vm.Interpret( nativeFnCall2 );
  test( vm.GetOutput().starts_with( "1764\nelapsed=" ) );
  test( std::isdigit( vm.GetOutput()[ 14 ] ) );

  std::string_view simpleFnCall1 =
    "fun sum(int a, int b) { \n"
    "  return a + b;         \n"
    "}                       \n"
    "                        \n"
    "print 4 + sum(42, 3);   \n"
    ;
  vm.Reset();
  vm.Interpret( simpleFnCall1 );
  test( vm.GetOutput() == "49" );

  std::string_view simpleFnCall2 =
    "fun foo() {       \n"
    "  print 'fnCall'; \n"
    "}                 \n"
    "                  \n"
    "foo();            \n"
    ;
  vm.Reset();
  vm.Interpret( simpleFnCall2 );
  test( vm.GetOutput() == "fnCall" );

  vm.Reset();
  vm.Interpret( fib );
  test( vm.GetOutput().starts_with( "fib(0) -> 0\n"
                                    "fib(1) -> 1\n"
                                    "fib(2) -> 1\n"
                                    "fib(3) -> 2\n"
                                    "fib(4) -> 3\n"
                                    "fib(5) -> 5\n"
                                    "fib time=") );

  vm.Reset();
  std::string intToHex;
  File::ReadEntireFile( "IntToHex.c", intToHex );
  vm.Interpret( intToHex );
  test( vm.GetOutput() == "empty\n50\n65\n51\n48\n51\n57" );

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
  vm.Interpret( captures1 );
  test( vm.GetOutput() == "10" );

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
  vm.Interpret( captures2 );
  test( vm.GetOutput() == "outside" );

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
  vm.Interpret( funref1 );
  test( vm.GetOutput() == "doughnut\nbagel" );

  // Stopped implementation of https://creaftinginterpreters.com around 25.2 in Closures chapter

  vm.Reset();
  //auto genre = []( uint32_t/*, Value* */) { return Value{"Rock"}; };
  //vm.AddNativeFunction( "genre", genre );
  vm.Interpret( eightiesPop );
  test( vm.GetOutput() == "true" );

  std::string_view bestOfSarahAndLaufey = "  \
  fun bestOfSarahAndLaufey()               \
  {                                 \
    return ( ( artist == 'Sarah McLachlan' or artist == 'Laufey' ) \
             and rating >= 4 );     \
  }                                 \
  print bestOfSarahAndLaufey();";
  vm.Reset();
  //vm.Interpret( bestOfSarahAndLaufey );
  //test( vm.GetOutput() == "true" );
}

///////////////////////////////////////////////////////////////////////////////
