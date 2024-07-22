#pragma once
#include <CreateClassificationObjectByReadingExcel.hxx>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


using namespace libxl;
using namespace std;

struct ItemData {
	wstring item_id;
	wstring class_hierarchy;
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
					item.class_hierarchy = sheet->readStr(row, 1) ? sheet->readStr(row, 1) : L"";
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

string findClassID(const vector<string>& classHierarchy) {
	if (classHierarchy.empty()) return "";

	// Start with the root class ID (assuming you have a known root)
	string currentClassID = classHierarchy[0];

	for (size_t i = 1; i < classHierarchy.size(); ++i) {
		int childCount = 0;
		char** children = nullptr;
		int status = ICS_class_ask_children(currentClassID.c_str(), ICS_CHILD_type_class, &childCount, &children);

		if (status != ITK_ok || childCount == 0) {
			char* message = NULL;
			EMH_ask_error_text(status, &message);
			printf("Error finding children for class ID: %s\n", currentClassID.c_str());
			MEM_free(message);
			return "";
		}

		bool found = false;
		for (int j = 0; j < childCount; ++j) {
			if (classHierarchy[i] == children[j]) {
				currentClassID = children[j];
				found = true;
				break;
			}
		}
		MEM_free(children);
		if (!found) {
			printf("Error finding class ID for hierarchy: %s\n", classHierarchy[i].c_str());
			return "";
		}
	}

	return currentClassID;
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

	vector<ItemData> items = readExcel(L"D:\\Book8.xlsx");

	// Process items
	for (const auto& item : items) {
		if (item.item_id.empty() || item.class_hierarchy.empty()) {
			wprintf(L"Skipping item classification due to empty item_id or class_hierarchy\n");
			continue; // Skip processing this item
		}

		// Retrieve item tag
		tag_t itemTag = getItemTag(ws2s(item.item_id));
		if (itemTag == NULLTAG) {
			wprintf(L"Error retrieving item tag for item: %ls\n", item.item_id.c_str());
			continue;
		}

		// Parse class hierarchy
		std::vector<std::string> classHierarchy = split(ws2s(item.class_hierarchy), ',');
		std::string classId = findClassID(classHierarchy);
		if (classId.empty()) {
			wprintf(L"Error finding class ID for hierarchy: %ls\n", item.class_hierarchy.c_str());
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
		status = createICO("NewICO", itemTag, classId, attributeCount, attributeIds, attributeValues);
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
