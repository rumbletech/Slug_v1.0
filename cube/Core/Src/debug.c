/*
 * debug.c
 *
 *  Created on: Oct 5, 2024
 *      Author: Mohamed-Rashad
 */

#include "debug.h"

#if defined(_OPTS_DEBUG_EN) && _OPTS_DEBUG_EN == true

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

static lw_uart* debugh;

extern void Debug_Init(lw_uart * huart ){
	debugh = huart;
}

extern int Debug_Printf (char * str, ...){

	va_list varlist;
	int istr_i = 0, ostr_i=0;
	char strbuff[_DEBUG_PRINTF_BUFFLEN_]= {0u};
	char tmpbuff[20] = {0u};

	va_start( varlist, str );

	while (str && str[istr_i]){
		int l;
		char * p;
		if(str[istr_i] == '%'){
 		    istr_i++;
 		    switch (str[istr_i])
 		    {
	 		    case 'c':
	 		    {
	 		    	tmpbuff[0u] = (char)va_arg( varlist, int );
	 		        p = &tmpbuff[0];
	 		        l = 1u;
	 		        break;
	 		    }
	 		    case 'd':
	 		    {
	 		        itoa(va_arg( varlist, int ), tmpbuff, 10);
	 		        p = tmpbuff;
	 		        l = strlen(tmpbuff);
		           break;
		        }
		        case 'x':
		        {
		           itoa(va_arg( varlist, int ), tmpbuff, 16);
		           p = tmpbuff;
		           l = strlen(tmpbuff);
		           break;
		        }
		        case 's':
		        {
			       p = va_arg( varlist, char* );
		           l = strlen(p);
		           break;
		        }
        	}
     	}
		else{
			l = 1u;
			p = &str[istr_i];
	    }

	    if ( (ostr_i +l) >= _DEBUG_PRINTF_BUFFLEN_ ) {
	        break;
	    }

	    strncpy(&strbuff[ostr_i],p,l);
	    ostr_i += l;
	    istr_i++;
	}

    va_end(varlist);

	lw_UART_Transmit(debugh, (const uint8_t*)&strbuff[0], ostr_i);

    return ostr_i;
 }

#endif
