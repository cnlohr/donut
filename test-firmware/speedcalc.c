#include <stdio.h>
#include <math.h>
#include <stdint.h>

int main()
{
	float f;
	int note;

#define NUMNOTES 72 //6 octaves
	uint8_t freq_s[NUMNOTES];
	uint8_t freq_rs[NUMNOTES];
	for( note = 0; note < NUMNOTES; note++ )
	{
		f = 130.81 * powf( 2.0f, ((float)note)/12.0f ); //C3 base note.

		int target_rs, target_speed;
		int best_rs;
		int best_speed;
		float best_err = 1e20;

		for( target_rs = 64; target_rs < 126; target_rs++ )
		{
			for( target_speed = 1; target_speed < target_rs - 1;  target_speed++ )
			{
				//int calc_speed = ( (f * target_rs) / ( 31250.0 / 2.0 ) + 0.5 );
				float calcfreq = (31250/2.0) / target_rs * target_speed;
				//printf( "   %f == %f [%d]\n", f, calcfreq, target_speed );
				float err = (f<calcfreq)?(calcfreq-f):(f-calcfreq);
				if( err < best_err )
				{
					best_rs = target_rs;
					best_speed = target_speed;
					best_err = err;
				}
			}
		}
		float calcfreq = (31250/2.0) / best_rs * best_speed;
		freq_s[note] = best_speed;
		freq_rs[note] = best_rs;
		//printf( "CHOSEN: %d %d = %f %f  // %f\n", best_rs, best_speed, calcfreq, f, (calcfreq - f)/f*100.0 );
	}

	printf( "#include <stdint.h>\n" );
	printf( "uint8_t freq_s[%d] = { ", NUMNOTES );
	for( note = 0; note < NUMNOTES; note++ )
	{
		printf( "%d, ", freq_s[note] );
	}
	printf( "};\nuint8_t freq_rs[%d] = { ", NUMNOTES );
	for( note = 0; note < NUMNOTES; note++ )
	{
		printf( "%d, ", freq_rs[note] );
	}
	printf( "};\n" );

	return 0;
}

