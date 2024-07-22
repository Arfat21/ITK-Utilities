#pragma
#include <tcinit/tcinit.h>		
#include <tccore/item.h>	
#include <tc/emh.h>		
#include <string.h> 
#include <tcinit/tcinit.h>		// ITK_init_module, ITK_exit_module 
//#include <tc/tc_startup.h>		// ITK_user_main, ITK_initialize_text_services. 
#include <tccore/item.h>		// ITEM Module. 
#include <tc/emh.h>				// Error Module. 
#include <tccore/aom.h>			// AOM Module. 
#include <tccore/aom_prop.h> 
#include <tccore/tctype.h>// TCTYPE APIs. tc_strcpy
#include <iostream>
#include <vector>"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tcinit/tcinit.h>
#include <tccore/item.h>
#include <tc/emh.h>
#include <tccore/aom.h>
#include <tccore/aom_prop.h>
#include <tccore/tctype.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <ics/ics2.h>
#include "libxl.h"


#define EXIT_FAILURE 1

#define ITK_CALL(x)											\
{																\
	int retcode;												\
	char *err_string = NULL;									\
	if( (retcode = (x)) != ITK_ok)								\
	{															\
		EMH_ask_error_text (retcode, &err_string);              \
		if(err_string != NULL)									\
		{														\
			printf ("ERROR: %d ERROR MSG: %s \n", retcode, err_string);        \
			printf ("Function: %s FILE: %s LINE: %d \n",#x, __FILE__, __LINE__);             \
			TC_write_syslog("ERROR[%d]: %s\t(FILE: %s, LINE:%d)\n", retcode, err_string, __FILE__, __LINE__);\
			MEM_free(err_string);								\
		}		\
		return retcode;\
	}                         									\
}


