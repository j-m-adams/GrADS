#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define MAX_BUF 1024

main (int argc, char *argv[])
{
    
    int   line, i, iflag;
    char  buf[MAX_BUF];
    char  tokbuf[MAX_BUF];  /* copy of buf[] for strtok() to work on */
    int   f, x, y, scale, ref, dw;
    char  *valp;

    char* units;
    char* description;
    char file_in[70], file_out[70];
    FILE *fp_in, *fp_out;

    if ( argc != 2 )
    {
      printf(" ERROR:  no arguments\n");
      printf(" convert <filename:\n");
      exit(-1);
    }
    sprintf(file_in,"%s.old",argv[1]);
    printf(" full input file name =%s\n", file_in);
    sprintf(file_out,"%s",argv[1]);
    printf(" full output file name =%s\n", file_out);
    sprintf(buf, "mv %s %s.old\n", argv[1], argv[1]);
    system(buf);
    
    if( (fp_in = fopen( file_in, "r" )) == NULL )
    {
        sprintf( buf, "Can't open input file \"%s\"\n", file_in );
        exit(1);
    }
    if( (fp_out = fopen( file_out, "w" )) == NULL )
    {
        sprintf( buf, "Can't open output file \"%s\"\n", file_in );
        exit(1);
    }
    
    for( line=1; fgets( buf, MAX_BUF, fp_in ) != NULL; line++ )
    {
        iflag = 0;
        valp = buf;
        if( *valp == '#')
        {
          fprintf(fp_out,"%s", buf);
        } else if ( strlen(buf) == 1 ) {
          fprintf(fp_out,"\n");
        }else{
          /*
          * Since strtok() mangles the string it works on, let strtok() use
          * a copy of buf.
          */

          strcpy( tokbuf, buf );

          /*
          * Trim trailing whitespace in 'tokbuf' (this makes it easier
          * to get the description later on).
          */

          /*  092997  LAH: Added char cast to correct Linux warning */
          /* 100997 LAH: Added ulong_t cast */
          for( valp=&tokbuf[strlen(tokbuf)-1]; isspace( *valp ); valp-- )
             *valp = (char) NULL;

          /* Get F, X, and Y portion and compute table location */

          if( (valp = strtok( tokbuf, " -\t" )) == NULL )
          {
            printf("Line %d: Missing F descriptor\n", line);
            iflag = 1;
          }
          else
            f = atoi( valp );

          if (iflag == 0)
          {
            if( (valp = strtok( NULL, " -\t" )) == NULL )
            {
              printf("Line %d: Missing X descriptor\n", line);
              iflag =1;
           
            }
            else
              x = atoi( valp );
          }

          if ( iflag == 0)
          {
            if( (valp = strtok( NULL, " \t" )) == NULL )
            {
              printf("%Line %d: Missing Y descriptor\n", line);
              iflag = 1;
            }
            else
             y = atoi( valp );
          }

          
/* Check for descriptor duplication. */
          if (iflag == 0 )
          {
      	    if( (valp = strtok( NULL, " \t" )) == NULL )
            {
              printf("Line %d: Missing Scale value\n", line);
            } else
              scale = atoi( valp );
          }

          if ( iflag == 0 )
          {
            if( (valp = strtok( NULL, " \t" )) == NULL )
            {
              printf("Line %d: Missing Reference Value\n", line );
              iflag =1;
            } else
              ref = atoi( valp );
          }

          if ( iflag == 0 )
          {
            if( (valp = strtok( NULL, " \t" )) == NULL )
            {
              printf(" Line %d: Missing Data Width value\n", line );
              iflag = 1;
            } else
              dw = atoi( valp );
          }


          if ( iflag == 0)
          {
            if( (units = strtok( NULL, " \t\n" )) == NULL )
            {
              printf("Line %d: Missing Units description\n", line );
              iflag == 1;
            }
          }

          /* Ignore leading whitespace. */

          /*  092997  LAH: Added cast to correct Linux warning */
          for( valp=units+strlen(units)+1; *valp != (char) NULL; valp++ )
          {
            /* 100997 LAH: Added ulong_t cast */
            if( !isspace( *valp ) )
                break;
          }

          /*  092997  LAH: Added cast to correct Linux warning */
          if( *valp == '\n' || *valp == (char) NULL )
            description = strdup( "???" );
          else
            description = strdup( valp );

          if ( iflag == 0)
          {
            fprintf(fp_out, "%3d;%5d;%7d;%7d;%12d;%5d; %12s; %s\n",
	            f,x,y,scale,ref,dw,units,description);
          }
        }
     }
     return 0;
}
