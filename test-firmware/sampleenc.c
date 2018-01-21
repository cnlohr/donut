#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main()
{
	FILE * f = fopen( "sample0.raw", "rb" );
	fseek( f, 0, SEEK_END );
	int len = ftell( f );
	fseek( f, 0, SEEK_SET );

	int8_t * dat = malloc( len );
	fread( dat, 1, len, f );
	fclose( f );
	printf( "#include <stdint.h>\n#include <avr/pgmspace.h>\n#define NUM_SAMPLES0 %d\nconst int8_t PROGMEM aud0dat[%d] = {", len, len );
	int i;
	for( i = 0; i < len; i++ )
	{
		if( (!(i & 0x0f)) ) printf( "\n\t" );
		printf( "%d, ", dat[i] );
	}
	printf( "};\n" );


	FILE * g = fopen( "sample1.raw", "rb" );
	fseek( g, 0, SEEK_END );
	len = ftell( g );
	fseek( g, 0, SEEK_SET );

	dat = malloc( len );
	fread( dat, 1, len, g );
	fclose( g );
	printf( "#define NUM_SAMPLES1 %d\nconst int8_t PROGMEM aud1dat[%d] = {", len, len );
	for( i = 0; i < len; i++ )
	{
		if( (!(i & 0x0f)) ) printf( "\n\t" );
		printf( "%d, ", dat[i] );
	}
	printf( "};\n" );
	return 0;
}

