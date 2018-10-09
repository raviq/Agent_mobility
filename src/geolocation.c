/***
 *
 *  json
 *  
 *  http://www.quora.com/Is-there-a-C-struct-to-JSON-generator-library
 *
 *  json api used in yamux ?
 *
 *  use this for th moment: http://ccodearchive.net/info/json.html
 *
 */

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "geolocation.h"


float random_float(float max)
{
    return (float)rand()/(float)(RAND_MAX/max);
}

// test
void geo()
{
    
    int i;
    for (i = 0 ; i < 5 ; i++)
    {
        printf("    %f, %f\n", random_float(100.0), random_float(100.0));
    }
   
}
