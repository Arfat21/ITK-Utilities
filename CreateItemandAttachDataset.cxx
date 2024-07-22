#include "Header.hxx"
#include <string>
#include <iostream>


using namespace std;

//Function to create an item and its revision

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

	 //Save and refresh the item
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

	 //Clean up allocated memory
	MEM_free(attNames[0]);
	MEM_free(attNames);
	MEM_free(attValues[0]);
	MEM_free(attValues);

	return status;
}

//Function to create a relation between two objects
int CreateRelation(tag_t primary_object, tag_t secondary_object, const char* relation_type, tag_t* relation) {
	int status = ITK_ok;
	tag_t relation_tag = NULLTAG;
	tag_t rel_type_tag = NULLTAG;

	// Find the relation type tag
	status = GRM_find_relation_type(relation_type, &rel_type_tag);
	if (status != ITK_ok) {
		char* message = NULL;
		EMH_ask_error_text(status, &message);
		printf("Error finding relation type : %s\n", message);
		MEM_free(message);
		return status;
	}

	// Create the relation between primary and secondary objects
	status = GRM_create_relation(primary_object, secondary_object, rel_type_tag, NULLTAG, &relation_tag);
	if (status != ITK_ok) {
		char* message = NULL;
		EMH_ask_error_text(status, &message);
		printf("Error creating relation : %s\n", message);
		MEM_free(message);
		return status;
	}
	else {
		printf("Relation created successfully between primary object %u and secondary object %u.\n", primary_object, secondary_object);
	}

	// Save the relation
	status = GRM_save_relation(relation_tag);
	if (status != ITK_ok) {
		char* message = NULL;
		EMH_ask_error_text(status, &message);
		printf("Error saving relation : %s\n", message);
		MEM_free(message);
		return status;
	}
	else {
		printf("Relation saved successfully.\n");
	}

	 //Pass back the relation tag if needed
	if (relation != NULL) {
		*relation = relation_tag;
	}

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
	tag_t secondary_item, secondary_revision;
	tag_t tertiary_item, tertiary_revision;

	// Create the parent item revision
	status = CreateItem("Item", "Parent Item", &parent_item, &parent_revision);
	if (status != ITK_ok) {
		printf("Error creating parent item revision.\n");
		return status;
	}

	 //Print parent item ID and revision ID
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

	 //Create the secondary custom item revision (A7FinalTask)
	status = CreateItem("A7FinalTask", "Secondary Item", &secondary_item, &secondary_revision);
	if (status != ITK_ok) {
		printf("Error creating secondary item revision.\n");
		return status;
	}

	 //Print secondary item ID and revision ID
	char* secondary_item_id = NULL;
	status = AOM_ask_value_string(secondary_item, "item_id", &secondary_item_id);
	if (status != ITK_ok) {
		char* message = NULL;
		EMH_ask_error_text(status, &message);
		printf("Error getting secondary item ID : %s\n", message);
		MEM_free(message);
		return status;
	}
	printf("Secondary Item ID: %s, Secondary Revision ID: %u\n", secondary_item_id, secondary_revision);
	MEM_free(secondary_item_id);

	 //Create the tertiary custom item revision (A7ArfatDemo)
	status = CreateItem("A7ArfatDemo", "Tertiary Item", &tertiary_item, &tertiary_revision);
	if (status != ITK_ok) {
		printf("Error creating tertiary item revision.\n");
		return status;
	}

	 //Print tertiary item ID and revision ID
	char* tertiary_item_id = NULL;
	status = AOM_ask_value_string(tertiary_item, "item_id", &tertiary_item_id);
	if (status != ITK_ok) {
		char* message = NULL;
		EMH_ask_error_text(status, &message);
		printf("Error getting tertiary item ID : %s\n", message);
		MEM_free(message);
		return status;
	}
	printf("Tertiary Item ID: %s, Tertiary Revision ID: %u\n", tertiary_item_id, tertiary_revision);
	MEM_free(tertiary_item_id);

	// Establish relations: parent -> secondary -> tertiary
	status = CreateRelation(parent_revision, secondary_revision, "IMAN_reference", NULL);
	if (status != ITK_ok) {
		printf("Error creating relation between parent and secondary item.\n");
		return status;
	}

	status = CreateRelation(secondary_revision, tertiary_revision, "IMAN_reference", NULL);
	if (status != ITK_ok) {
		printf("Error creating relation between secondary and tertiary item.\n");
//		return status;
	}

	// Print structure created successfully
	printf("\nStructure created successfully:\n");
	printf("Parent Item Revision: %u\n", parent_revision);
	printf("Secondary Custom Item Revision (A7FinalTask): %u\n", secondary_revision);
	printf("Tertiary Custom Item Revision (A7ArfatDemo): %u\n", tertiary_revision);

	return status;
}