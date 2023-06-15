#include "catalog.h"
#include "query.h"

/*
 * Deletes records from a specified relation.
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */

const Status QU_Delete(const string &relation,
					   const string &attrName,
					   const Operator op,
					   const Datatype type,
					   const char *attrValue)
{

	Status status;

	// Construct a heapFileScan object using the relation name.
	HeapFileScan *hfs = new HeapFileScan(relation, status);
	if (status != OK)
	{
		return status;
	}

	// To call startScan, we need more information such as length and offset.
	// Thus, we need to get these information by calling getInfo and get a AttrDesc object.
	AttrDesc attrDesc;
	attrCat->getInfo(relation, attrName, attrDesc);

	// Next, we call startScan() with different parameters for different value types.
	// Before that, we first handle the case when attrName is NULL.
	if (attrName == "")
	{
		status = hfs->startScan(0, 0, STRING, NULL, EQ);
	}
	else
	{
		int intValue;
		float floatValue;

		switch (type)
		{
		case STRING:
			status = hfs->startScan(attrDesc.attrOffset, attrDesc.attrLen, type, attrValue, op);
			break;

		case INTEGER:
			intValue = atoi(attrValue);
			status = hfs->startScan(attrDesc.attrOffset, attrDesc.attrLen, type, (char *)&intValue, op);
			break;

		case FLOAT:
			floatValue = atof(attrValue);
			status = hfs->startScan(attrDesc.attrOffset, attrDesc.attrLen, type, (char *)&floatValue, op);
			break;
		}
	}

	// If status of startScan() is not OK, delete the heapFileScan object.
	if (status != OK)
	{
		delete hfs;
		return status;
	}

	// Start the scan successcully, then we can try to find and delete the target record.
	while (status == OK)
	{
		RID rid;
		status = hfs->scanNext(rid);
		if (status == FILEEOF)
			break;
		
		status = hfs->deleteRecord();
		if (status != OK)
			return status;
	}

	// Finally, we end the scan and cleanup the heapFileScan object.
	hfs->endScan();
	delete hfs;

	return OK;
}
