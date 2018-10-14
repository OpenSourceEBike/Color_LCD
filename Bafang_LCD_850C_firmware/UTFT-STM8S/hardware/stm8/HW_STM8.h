#include "stdio.h"
#include "math.h"

// mixture of function Print_print_fd from sduino and function printNumI of UTFT

int UTFT__convert_integer(char * st, long num, int length, char filler);

void UTFT__convert_float(char *buf, double num, int width, byte prec)
{
  size_t n = 0;
  uint8_t i;
  unsigned long int_part;
  double remainder, rounding;
  boolean neg=false;
  
  // Handle negative numbers
  if (num < 0.0)
  {
    num = -num;
    neg = true;
  }

  // Round correctly so that print(1.999, 2) prints as "2.00"
  rounding = 0.5;
  for (i=0; i<prec; ++i)
    rounding /= 10.0;

  num += rounding;

  // Extract the integer part of the number and print it
  int_part = (unsigned long)num;
  n+=UTFT__convert_integer(buf+n,(neg?-1:1)*num,width,' ');
  remainder = num - (double)int_part;
  if(prec>0)
  {
    buf[n++]='.';
    remainder=remainder*powf(10.0,(float)prec);
    n+=UTFT__convert_integer(buf+n,(unsigned long)remainder,prec,'0');
  }
  buf[n]=0;
}


int UTFT__convert_integer(char * st, long num, int length, char filler)
{
	char buf[25];
	boolean neg=false;
	int c=0, f=0;
  
	if (num==0)
	{
		if (length!=0)
		{
			for (c=0; c<(length-1); c++)
				st[c]=filler;
			st[c++]=48;
			st[c]=0;
		}
		else
		{
			st[c++]=48;
			st[c]=0;
		}
	}
	else
	{
		if (num<0)
		{
			neg=true;
			num=-num;
		}
	  
		while (num>0)
		{
			buf[c]=48+(num % 10);
			c++;
			num=(num-(num % 10))/10;
		}
		buf[c]=0;
	  
		if (neg)
		{
			st[0]=45;
		}
	  
		if (length>(c+neg))
		{
			for (int i=0; i<(length-c-neg); i++)
			{
				st[i+neg]=filler;
				f++;
			}
		}

		for (int i=0; i<c; i++)
		{
			st[i+neg+f]=buf[c-i-1];
		}
		st[c+neg+f]=0;
	}
	return c+neg+f;
}
