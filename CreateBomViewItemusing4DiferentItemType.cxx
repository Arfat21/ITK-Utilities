#include "Header.hxx"
#include <string>
#include <iostream>

using namespace std;

// Function to create an item and its revision
void createBVR(const tag_t item, const tag_t item_revision);
int CreateItem(const char* item_type, const char* object_name, tag_t* item, tag_t* revision) {
	int status = ITK_ok;
	char* message = NULL;

	// Attributes to set during item creation
	char** attNames = (char**)MEM_alloc(sizeof(char*));
	attNames[0] = (char*)MEM_alloc(sizeof(char) * (strlen("object_name") + 1));
	tc_strcpy(attNames[0], "object_name");

	char** attValues = (char**)MEM_alloc(sizeof(char*));
	attValues[0] = (char*)MEM_alloc(sizeof(char) * (strlen(object_name) + 1));
	tc_strcpy(attValues[0], object_name);

	// Create the item and its revision
	status = ITEM_create_item2(item_type, 1, attNames, attValues, "A", item, revision);
	if (status != ITK_ok) {
		EMH_ask_error_text(status, &message);
		printf("Error ITEM_create_item2 : %s\n", message);
		MEM_free(message);
		return status;
	}

	// Save and refresh the item
	status = AOM_save_with_extensions(*item);
	if (status != ITK_ok) {
		EMH_ask_error_text(status, &message);
		printf("Error saving item : %s\n", message);
		MEM_free(message);
		return status;
	}

	status = AOM_refresh(*item, FALSE);
	if (status != ITK_ok) {
		EMH_ask_error_text(status, &message);
		printf("Error refreshing item : %s\n", message);
		MEM_free(message);
		return status;
	}

	// Clean up allocated memory
	MEM_free(attNames[0]);
	MEM_free(attNames);
	MEM_free(attValues[0]);
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
		printf("\n\tMissing Arguments: Please provide mandatory arguments.");
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

	printf("\n Login Successful \n");

	tag_t parent_item, parent_revision;
	tag_t child_items[6], child_revisions[6];
	const char* child_types[6] = { "Document", "Design", "A7ArfatDemo", "Document", "Design", "A7ArfatDemo" };
	const char* child_names[6] = { "Child1", "Child2", "Child3", "Child4", "Child5", "Child6" };

	// Create the parent item revision
	status = CreateItem("Item", "Parent Item", &parent_item, &parent_revision);
	if (status != ITK_ok) {
		printf("Error creating parent item revision.\n");
		return status;
	}

	// Print parent item ID and revision ID
	char* parent_item_id = NULL;
	status = AOM_ask_value_string(parent_item, "item_id", &parent_item_id);
	if (status != ITK_ok) {
		char* message = NULL;
		EMH_ask_error_text(status, &message);
		printf("Error getting parent item ID : %s\n", message);
		MEM_free(message);
		return status;
	}
	printf("\nParent Item ID: %s, Parent Revision ID: %u\n", parent_item_id, parent_revision);
	MEM_free(parent_item_id);

	// Create the child items and their revisions
	for (int i = 0; i < 6; i++) {
		status = CreateItem(child_types[i], child_names[i], &child_items[i], &child_revisions[i]);
		if (status != ITK_ok) {
			printf("Error creating child item revision %d.\n", i + 1);
			return status;
		}

		// Print child item ID and revision ID
		char* child_item_id = NULL;
		status = AOM_ask_value_string(child_items[i], "item_id", &child_item_id);
		if (status != ITK_ok) {
			char* message = NULL;
			EMH_ask_error_text(status, &message);
			printf("Error getting child item ID %d : %s\n", i + 1, message);
			MEM_free(message);
			return status;
		}
		printf("Child %d Item ID: %s, Child %d Revision ID: %u\n", i + 1, child_item_id, i + 1, child_revisions[i]);
		MEM_free(child_item_id);
	}

	// Create BOM Window and add children to the parent
	tag_t bomWindow, topBOMLine, newBOMLine;

	status = BOM_create_window(&bomWindow);
	if (status != ITK_ok) {
		printf("Error creating BOM window.\n");
		return status;
	}

	status = BOM_set_window_top_line(bomWindow, NULLTAG, parent_revision, NULLTAG, &topBOMLine);
	if (status != ITK_ok) {
		printf("Error setting top line of BOM window.\n");
		return status;
	}

	createBVR(parent_item, parent_revision);

	// Add children to the BOM structure
	for (int i = 0; i < 6; i++) {
		status = BOM_line_add(topBOMLine, NULLTAG, child_revisions[i], NULLTAG, &newBOMLine);
		if (status != ITK_ok) {
			char* message = NULL;
			EMH_ask_error_text(status, &message);
			printf("Error adding Child %d to BOM: %s\n", i + 1, message);
			MEM_free(message);
			printf("topBOMLine tag: %u\n", topBOMLine);  // Print out the tag of topBOMLine for debugging
			printf("Child revision tag: %u\n", child_revisions[i]);  // Print out the tag of the child revision
		}
		else {
			printf("Child %d added successfully to BOM.\n", i + 1);
		}
	}

	// Save and refresh the BOM window
	status = BOM_save_window(bomWindow);
	if (status != ITK_ok) {
		printf("Error saving BOM window.\n");
		return status;
	}

	status = BOM_refresh_window(bomWindow);
	if (status != ITK_ok) {
		printf("Error refreshing BOM window.\n");
		return status;
	}

	status = BOM_close_window(bomWindow);
	if (status != ITK_ok) {
		printf("Error closing BOM window.\n");
		return status;
	}

	printf("BOM structure created successfully.\n");

	return status;
}

/*******************************************************************************************/
void createBVR(const tag_t item, const tag_t item_revision) {
	int status = ITK_ok;
	char* message = NULL;

	tag_t item_bom_view = NULLTAG;
	status = PS_create_bom_view(NULLTAG, NULL, NULL, item, &item_bom_view);
	if (status != ITK_ok) {
		EMH_ask_error_text(status, &message);
		printf("Error while PS_create_bom_view : %s\n", message);
		MEM_free(message);
		return;
	}
	printf("Item Bom view Tag %u \n", item);
	AOM_save_without_extensions(item_bom_view);
	AOM_save_without_extensions(item);

	tag_t item_rev_bvr = NULLTAG;
	status = PS_create_bvr(item_bom_view, NULL, NULL, false, item_revision, &item_rev_bvr);
	if (status != ITK_ok) {
		EMH_ask_error_text(status, &message);
		printf("Error while PS_create_bvr : %s\n", message);
		MEM_free(message);
		return;
	}
	printf("ItemRev Bom view Tag %u \n", item_revision);

	AOM_save_without_extensions(item_rev_bvr);
	AOM_save_without_extensions(item_revision);
}
