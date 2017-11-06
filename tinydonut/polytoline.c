#include <stdio.h>
#include <string.h>

int main()
{
	int c;
	int cbb = 0;
	char stbuff[100];
	char last[100];
	int stplace;
	while( (c=getchar()) != EOF )
	{
		if( c == '(' && cbb == 0 )
		{
			cbb = 1;
		}
		if( cbb == 1  && c == ' ' )
		{
			cbb = 2;
			stplace = 0;
		}
		if( cbb == 2 )
		{
			if( c == ')' )
			{
				stbuff[stplace] = 0;
				if( last[0] )
					printf( "      (fp_line (start %s) (end %s) (layer Edge.Cuts) (width 0.2))\n",  last, stbuff );
				else
					printf( "\n\n\n" );
				strcpy( last, stbuff );
				cbb = 0;
			}
			stbuff[stplace++] = c;
		}
	}
}

