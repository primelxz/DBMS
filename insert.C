#include "catalog.h"
#include "query.h"
#include <stdlib.h>
#include <unistd.h>

/*
 * Inserts a record into the specified relation.
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */

const Status QU_Insert(const string & relation, const int attrCnt, const attrInfo attrList[]) {
	// part 6
	AttrDesc *attrs;
	int relAttrs;
	Status status;
	int size = 0;

	InsertFileScan rel(relation, status);

	if(status != OK) {
		return RELNOTFOUND;
	}

	status = attrCat->getRelInfo(relation, (int&)relAttrs, attrs);

	if(status != OK)
		return status;

	if(attrCnt != relAttrs) {
		return BADCATPARM;
	}
	
	// find total size via AttrDesc
	for(int i = 0; i < relAttrs; i++) {
		size += attrs[i].attrLen;
	}

	char data[size];
	


	for(int i = 0; i < relAttrs; i++) {
		attrInfo attr = attrList[i];

		if (attr.attrValue == NULL) {
			return ATTRNOTFOUND;
		}
	}

	for(int i = 0; i < relAttrs; i++) {
		int index = i;
		AttrDesc desc = attrs[i];

		// find correct index
		for(int j = 0; j < relAttrs; j++) {
			attrInfo attr = attrList[j];

			if(strcmp(attr.attrName, desc.attrName) == 0) {
				index = j;
				break;
			}
		}
		
		attrInfo insert = attrList[index];
		char *val;
		int tempI;
		int tempF;
		 
		switch (insert.attrType) {
			case STRING:
				val = (char *)insert.attrValue;
				break;
			case INTEGER:
				tempI = atoi((char*)insert.attrValue);
				val = (char*)&tempI;
				break;
			case FLOAT:
				tempF = atof((char*)insert.attrValue);
				val = (char*)&tempF;
				break;
		}

		memcpy(data + desc.attrOffset, val, desc.attrLen);
	}


	RID rid;
	Record record;
	record.length = size;

	record.data = (void *)data;

	status = rel.insertRecord(record, rid);

	if(status != OK)
		return status;

	return OK;
}