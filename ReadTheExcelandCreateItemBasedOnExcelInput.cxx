#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include <vector>
#include "libxl.h"

#pragma once
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

using namespace libxl;
using namespace std;

struct ItemData {
	wstring item_id;
	wstring object_type;
	wstring object_name;
};

vector<ItemData> readExcel(const wchar_t* filename) {
	vector<ItemData> items;  // Vector to store ItemData objects

	Book* book = xlCreateBook();
	if (book) {
		if (book->load(filename)) {
			Sheet* sheet = book->getSheet(0);
			if (sheet) {
				for (int row = 1; row <= sheet->lastRow(); ++row) {
					ItemData item;
					item.item_id = sheet->readStr(row, 0) ? sheet->readStr(row, 0) : L"";
					item.object_type = sheet->readStr(row, 1) ? sheet->readStr(row, 1) : L"";
					item.object_name = sheet->readStr(row, 2) ? sheet->readStr(row, 2) : L"";
					items.push_back(item);
				}
			}
			else {
				wprintf(L"Failed to get the first sheet in the Excel file: %ls\n", filename);
			}
		}
		else {
			wprintf(L"Failed to load Excel file: %ls\n", filename);
		}
		book->release();
	}
	else {
		wprintf(L"Failed to create Excel book object\n");
	}

	return items;
}

int CreateItem(const char* object_type, const wchar_t* object_name, const wchar_t* item_id, tag_t* item, tag_t* revision) {
	int status = ITK_ok;
	char* message = NULL;

	// Print debug information
	printf("Creating item with type: %s and name: %ls\n", object_type, object_name);

	char** attNames = (char**)MEM_alloc(2 * sizeof(char*));
	attNames[0] = (char*)MEM_alloc(sizeof(char) * (strlen("object_name") + 1));
	tc_strcpy(attNames[0], "object_name");
	attNames[1] = (char*)MEM_alloc(sizeof(char) * (strlen("item_id") + 1));
	tc_strcpy(attNames[1], "item_id");

	char** attValues = (char**)MEM_alloc(2 * sizeof(char*));
	size_t convertedChars = 0;
	attValues[0] = (char*)MEM_alloc((wcslen(object_name) + 1) * sizeof(char));
	wcstombs_s(&convertedChars, attValues[0], (wcslen(object_name) + 1) * sizeof(char), object_name, _TRUNCATE);
	attValues[1] = (char*)MEM_alloc((wcslen(item_id) + 1) * sizeof(char));
	wcstombs_s(&convertedChars, attValues[1], (wcslen(item_id) + 1) * sizeof(char), item_id, _TRUNCATE);

	status = ITEM_create_item2(object_type, 2, attNames, attValues, "A", item, revision);
	if (status != ITK_ok) {
		EMH_ask_error_text(status, &message);
		printf("Error ITEM_create_item2: %s\n", message);
		MEM_free(message);
		return status;
	}

	// Save item and refresh
	status = AOM_save_with_extensions(*item);
	if (status != ITK_ok) {
		EMH_ask_error_text(status, &message);
		printf("Error saving item: %s\n", message);
		MEM_free(message);
		return status;
	}

	status = AOM_refresh(*item, FALSE);
	if (status != ITK_ok) {
		EMH_ask_error_text(status, &message);
		printf("Error refreshing item: %s\n", message);
		MEM_free(message);
		return status;
	}

	MEM_free(attNames[0]);
	MEM_free(attNames[1]);
	MEM_free(attNames);
	MEM_free(attValues[0]);
	MEM_free(attValues[1]);
	MEM_free(attValues);

	return status;
}

int main(int argc, char* argv[]) {
	TC__initialise_tc(argc, argv);
	int status = ITK_ok;
	char* user = NULL;
	char* password = NULL;
	char* group = NULL;

	user = ITK_ask_cli_argument("-u=");
	password = ITK_ask_cli_argument("-p=");
	group = ITK_ask_cli_argument("-g=");

	if (user == NULL || password == NULL || group == NULL) {
		printf("\n\tMissing Arguments: Please provide mandatory arguments.\n");
		exit(0);
	}

	status = ITK_init_module(user, password, group);
	if (status != ITK_ok) {
		printf("\n\tError occurred while logging into Teamcenter.\n");
		char* err_string = NULL;
		EMH_ask_error_text(status, &err_string);
		printf("\n\tError while logging into Teamcenter; Reason: %s\n", err_string);
		MEM_free(err_string);
		return status;
	}

	printf("\nLogin Successful\n");

	vector<ItemData> items = readExcel(L"D:\\Book5.xlsx");

	// Process items
	for (const auto& item : items) {
		if (item.object_type.empty() || item.object_name.empty()) {
			wprintf(L"Skipping item creation due to empty object_type or object_name for item_id=%ls\n", item.item_id.c_str());
			wprintf(L"Skipping item creation due to empty object_type or object_name for item_id=%ls\n", item.object_name.c_str());
			wprintf(L"Skipping item creation due to empty object_type or object_name for item_id=%ls\n", item.object_type.c_str());

			continue; // Skip processing this item
		}

		// Proceed with item creation
		tag_t itemTag, itemRevision;
		string itemType(item.object_type.begin(), item.object_type.end());
		status = CreateItem(itemType.c_str(), item.object_name.c_str(), item.item_id.c_str(), &itemTag, &itemRevision);
		if (status != ITK_ok) {
			wprintf(L"Error creating item: %ls\n", item.item_id.c_str());
			continue;
		}
		wprintf(L"Successfully created item: %ls\n", item.item_id.c_str());

		// Print tag values for verification
		printf("Tag for item %ls: %d, Revision tag: %d\n", item.item_id.c_str(), itemTag, itemRevision);
	}

	return status;
}



using namespace libxl;
using namespace std;

struct ItemData {
	wstring item_id;
	wstring class_id;
	wstring attribute_values; // Comma-separated attribute values
};

vector<ItemData> readExcel(const wchar_t* filename) {
	vector<ItemData> items;

	Book* book = xlCreateBook();
	if (book) {
		if (book->load(filename)) {
			Sheet* sheet = book->getSheet(0);
			if (sheet) {
				for (int row = 1; row < sheet->lastRow(); ++row) {
					ItemData item;
					item.item_id = sheet->readStr(row, 0) ? sheet->readStr(row, 0) : L"";
					item.class_id = sheet->readStr(row, 1) ? sheet->readStr(row, 1) : L"";
					item.attribute_values = sheet->readStr(row, 2) ? sheet->readStr(row, 2) : L"";
					items.push_back(item);
				}
			}
			else {
				wprintf(L"Failed to get the first sheet in the Excel file: %ls\n", filename);
			}
		}
		else {
			wprintf(L"Failed to load Excel file: %ls\n", filename);
		}
		book->release();
	}
	else {
		wprintf(L"Failed to create Excel book object\n");
	}

	return items;
}

tag_t getItemTag(const std::string& itemId) {
	tag_t itemTag = NULLTAG;
	int status = ITEM_find_item(itemId.c_str(), &itemTag);
	if (status != ITK_ok || itemTag == NULLTAG) {
		char* message = NULL;
		EMH_ask_error_text(status, &message);
		printf("Error finding item: %s\n", message);
		MEM_free(message);
	}
	return itemTag;
}

int createICO(const std::string& icoId, tag_t itemTag, const std::string& classId, int attributeCount, int* attributeIds, const char** attributeValues) {
	tag_t icoTag = NULLTAG;
	int status = ICS_ico_create(icoId.c_str(), itemTag, classId.c_str(), attributeCount, attributeIds, attributeValues, &icoTag);
	if (status != ITK_ok) {
		char* message = NULL;
		EMH_ask_error_text(status, &message);
		printf("Error creating ICO: %s\n", message);
		MEM_free(message);
	}
	return status;
}

std::string ws2s(const std::wstring& wstr) {
	std::string str(wstr.begin(), wstr.end());
	return str;
}

std::vector<std::string> split(const std::string& str, char delimiter) {
	std::vector<std::string> tokens;
	std::stringstream ss(str);
	std::string token;
	while (std::getline(ss, token, delimiter)) {
		tokens.push_back(token);
	}
	return tokens;
}

int main(int argc, char* argv[]) {
	TC__initialise_tc(argc, argv);
	int status = ITK_ok;
	char* user = NULL;
	char* password = NULL;
	char* group = NULL;

	user = ITK_ask_cli_argument("-u=");
	password = ITK_ask_cli_argument("-p=");
	group = ITK_ask_cli_argument("-g=");

	if (user == NULL || password == NULL || group == NULL) {
		printf("\n\tMissing Arguments: Please provide mandatory arguments.\n");
		exit(0);
	}

	status = ITK_init_module(user, password, group);
	if (status != ITK_ok) {
		printf("\n\tError occurred while logging into Teamcenter.\n");
		char* err_string = NULL;
		EMH_ask_error_text(status, &err_string);
		printf("\n\tError while logging into Teamcenter; Reason: %s\n", err_string);
		MEM_free(err_string);
		return status;
	}

	printf("\nLogin Successful\n");

	vector<ItemData> items = readExcel(L"D:\\Book6.xlsx");

	// Process items
	for (const auto& item : items) {
		if (item.item_id.empty() || item.class_id.empty()) {
			wprintf(L"Skipping item classification due to empty item_id or class_id\n");
			continue; // Skip processing this item
		}

		// Retrieve item tag
		tag_t itemTag = getItemTag(ws2s(item.item_id));
		if (itemTag == NULLTAG) {
			wprintf(L"Error retrieving item tag for item: %ls\n", item.item_id.c_str());
			continue;
		}

		// Parse attribute values
		std::vector<std::string> attributeTokens = split(ws2s(item.attribute_values), ',');
		int attributeCount = attributeTokens.size();
		int* attributeIds = new int[attributeCount];
		const char** attributeValues = new const char*[attributeCount];

		for (int i = 0; i < attributeCount; ++i) {
			attributeIds[i] = i + 1; // Assuming attribute IDs start from 1 and are sequential
			attributeValues[i] = attributeTokens[i].c_str();
		}

		// Create ICO
		status = createICO("NewICO", itemTag, ws2s(item.class_id), attributeCount, attributeIds, attributeValues);
		if (status != ITK_ok) {
			wprintf(L"Error creating ICO for item: %ls\n", item.item_id.c_str());
			continue;
		}

		wprintf(L"Successfully classified item: %ls\n", item.item_id.c_str());

		delete[] attributeIds;
		delete[] attributeValues;
	}

	return status;
}
