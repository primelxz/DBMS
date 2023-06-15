#include "catalog.h"
#include "query.h"
#include <unistd.h>
/**
 * Implemented by Tianyi Xu
*/


// forward declaration
const Status ScanSelect(const string & result, 
			const int projCnt, 
			const AttrDesc projNames[],
			const AttrDesc *attrDesc, 
			const Operator op, 
			const char *filter,
			const int reclen);

/*
 * Selects records from the specified relation.
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */

const Status QU_Select(const string & result, 
		       const int projCnt, 
		       const attrInfo projNames[],
		       const attrInfo *attr, 
		       const Operator op, 
		       const char *attrValue)
{
   // Qu_Select sets up things and then calls ScanSelect to do the actual work
    cout << "Doing QU_Select " << endl;

	// define variables we need to use
	Status status;
	AttrDesc projDescNames[projCnt];
	AttrDesc *attrDesc;

	// transform attrInfo into attrDesc
	for(int i = 0; i < projCnt; i++){
		status = attrCat->getInfo(projNames[i].relName, projNames[i].attrName, projDescNames[i]);
		if(status != OK){
			return status;
		}
	}

	int reclen = 0;
	// update attrDesc and get rectlen
	AttrDesc tempDesc;
	if(attr != NULL){
		status = attrCat->getInfo(attr->relName, attr->attrName, tempDesc);
		if(status != OK){
			return status;
		}
		attrDesc = &tempDesc;
	}

	// get reclen
	for(int i = 0; i < projCnt; i++){
		reclen += projDescNames[i].attrLen;
	}

	// call scanSelect
	// if attr is null, then we do unconditioned scan
	if(attr == NULL){
		status = ScanSelect(result, projCnt, projDescNames, attrDesc, EQ, NULL, reclen);
		if(status != OK){
			return status;
		}
	} else {
		status = ScanSelect(result, projCnt, projDescNames, attrDesc, op, attrValue, reclen);
		if(status != OK){
			return status;
		}
	}
	return status;
}


const Status ScanSelect(const string & result, 
#include "stdio.h"
#include "stdlib.h"
			const int projCnt, 
			const AttrDesc projNames[],
			const AttrDesc *attrDesc, 
			const Operator op, 
			const char *filter,
			const int reclen)
{
    cout << "Doing HeapFileScan Selection using ScanSelect()" << endl;
	Status status;

	//hfs scan
	HeapFileScan *hfs = new HeapFileScan(projNames[0].relName, status);
	//printf("Back from scan\n");
	//sleep(2);
	if(status != OK){
		return status;
	}

	//find record, start scan
	printf("Start scan\n");
	if(attrDesc == NULL){
		// This is the case when attr is null, so unconditioned scan
		status = hfs->startScan(0, 0, STRING, NULL, EQ);
		if(status != OK){
			return status;
		}
	} else {
		// we check the attribute type and then do the corresponding start scan and type casting
		
		float search_floatVal;
		switch(attrDesc->attrType){
			// string, no need conversion
			case 0:{
				status = hfs->startScan(attrDesc->attrOffset, attrDesc->attrLen, STRING, filter, op);
				if(status != OK){
					return status;
				}
				break;
			}
			// int and float, need conversion
			case 1:{
				int search_val = atoi(filter);
				status = hfs->startScan(attrDesc->attrOffset, attrDesc->attrLen, INTEGER, (char *)&search_val, op);
				if(status != OK){
					return status;
				}
				break;
			}
			case 2:{
				float search_val = atof(filter);
				status = hfs->startScan(attrDesc->attrOffset, attrDesc->attrLen, FLOAT, (char *)&search_val, op);
				if(status != OK){
					return status;
				}
				break;
			}
		}
	}
	// printf("Done start scan\n");
	// sleep(2);

	// record attrInfo list
	// find out how long the total object you want to copy is
	// create new array of that size
	// memcpy(dest, src, size)
	
	Status nextStatus = OK;
	// scan next record
	while(nextStatus == OK){
		RID rid;
		Record rec;
		nextStatus = hfs->scanNext(rid);
		if(nextStatus != OK){
			break;
		}

		// get the record
		status = hfs->getRecord(rec);
		if(status != OK){
			return status;
		}

		// Use memcpy to copy each attribute in the record into a list of attrInfo objects. 
		attrInfo attributeList[projCnt];

		// copy each attribute in the record into a list of attrInfo objects
		// int offset = 0;
		for(int i = 0; i < projCnt; i++){
			// copy the relation name
			strcpy(attributeList[i].relName, projNames[i].relName);
			
			// copy the attribute name
			strcpy(attributeList[i].attrName, projNames[i].attrName);

			// copy the attribute type
			attributeList[i].attrType = projNames[i].attrType;

			// copy the attribute length
			attributeList[i].attrLen = projNames[i].attrLen;

			// printf("segfault here\n");
			// sleep(2);
			
			attributeList[i].attrValue = (void*)malloc(projNames[i].attrLen);

			// copy the attribute value
			switch(projNames[i].attrType){
				case 0:{
					memcpy(attributeList[i].attrValue, rec.data + projNames[i].attrOffset, projNames[i].attrLen);
					break;
				}
				case 1:{
					int int_attr;
					// attributeList[i].attrValue = malloc(projNames[i].attrLen);
					memcpy(&int_attr, (int*)(rec.data + projNames[i].attrOffset), projNames[i].attrLen);
					//printf("int value %d\n", int_attr); 
					//sleep(2);
					//int* tmp = (int*)attributeList[i].attrValue; 
					memcpy(attributeList[i].attrValue, &int_attr, projNames[i].attrLen);
					//*((int*)attributeList[i].attrValue) = int_attr;
					//printf("pointer value int %d\n", &attributeList[i].attrValue); 
					break;
				}
				case 2:{
					float float_attr;
					// attributeList[i].attrValue = malloc(projNames[i].attrLen);
					memcpy(&float_attr, (float*)(rec.data + projNames[i].attrOffset), projNames[i].attrLen);
					//printf("float value %.2f\n", float_attr); 
					//sleep(2);
					//float* tmp = (float*)attributeList[i].attrValue; 
					memcpy(attributeList[i].attrValue, &float_attr, projNames[i].attrLen);
					//*((float*)attributeList[i].attrValue) = float_attr;
					//printf("float value pointer %.2f\n", &attributeList[i].attrValue); 
					break;
				}
			}
			//free(attributeList[i].attrValue);
		}
		// insert the record
		// printf("Inserting\n");
		// sleep(2);
		status = QU_Insert(result, projCnt, attributeList);
		if(status != OK){
			return status;
		}
		// printf("Done inserting\n");
		// sleep(2);
	}

	hfs->endScan();
	delete hfs;

	return status;
}
