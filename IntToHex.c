int count = 0;
char r0;
char r1;
char r2;
char r3;
char r4;

fun DigitToChar( int digit )
{
  char zero = 48;
  char letterA = 65;
  if ( digit < 10 )
    return zero + digit;
  return letterA + digit - 10;
}

fun StoreDigit( char c )
{
  if( count == 0 )
    r0 = c;
  else if( count == 1 )
    r1 = c;
  else if( count == 2 )
    r2 = c;
  else if( count == 3 )
    r3 = c;
  else if( count == 4 )
    r4 = c;
  count = count + 1;
}

fun PrintResult()
{
  if( count == 0 )
    print 'empty';
  else if( count == 1 )
    print r0;
  else if( count == 2 )
  {
    print r1;
    print r0;
  }
  else if( count == 3 )
  {
    print r2;
    print r1;
    print r0;
  }
  else if( count == 4 )
  {
    print r3;
    print r2;
    print r1;
    print r0;
  }
  else if( count == 5 )
  {
    print r4;
    print r3;
    print r2;
    print r1;
    print r0;
  }
  else
    print 'Too big!';
}

fun IntToHex( int i )
{
  count = 0;
  if( i < 0 )
    return '';

  while( i != 0 )
  {
    int digit = i % 16;
    char c = DigitToChar(digit);
    i = i / 16;
    StoreDigit( c );
  }

  PrintResult();
  return '';
}

fun main()
{
  IntToHex( -1 );
  IntToHex( 0 );
  IntToHex( 42 );
  IntToHex( 12345 );
}

main();
