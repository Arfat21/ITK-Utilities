#pragma
#include<cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>
#include <tcinit/tcinit.h>		// ITK_init_module, ITK_exit_module
#include <tc/tc_startup.h>		// ITK_user_main, ITK_initialize_text_services.
#include <tccore/item.h>		// ITEM Module.
#include <tc/emh.h>				// Error Module.
#include <tccore/aom.h>			// AOM Module.
#include <tccore/aom_prop.h>	// Property accessor methods (getters and setters).
#include <tccore/tctype.h>		// TCTYPE APIs. tc_strcpy

#include <ae/dataset.h>
#include <ae/datasettype.h>
#include <tccore/grm.h>
#include<bom/bom.h>
#include <sa/tcfile.h>
#include <tc/folder.h>
#include <tccore/workspaceobject.h>
#include <ae/ae.h>
#include<tccore/grm.h>
#include<form/form.h>
#include<tccore/releasestatus.h>
#include<sa/user.h>
#include<bom/bom.h>
#include<property/prop.h>

#define GRM_class_name_c   "ImanRelation"
#define GRM_relationtype_name "AttachedDataset"


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


