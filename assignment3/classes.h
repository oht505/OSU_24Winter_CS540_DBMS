#include <string>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <bitset>
using namespace std;

class Record {
public:
    int id, manager_id;
    std::string bio, name;

    Record() :id(-1) {}

    Record(vector<std::string> fields) {
        id = stoi(fields[0]);
        name = fields[1];
        bio = fields[2];
        manager_id = stoi(fields[3]);
    }

    void print() {
        cout << "\tID: " << id << "\n";
        cout << "\tNAME: " << name << "\n";
        cout << "\tBIO: " << bio << "\n";
        cout << "\tMANAGER_ID: " << manager_id << "\n";
    }
    
};


class LinearHashIndex {

private:
    static const int BLOCK_SIZE = 4096;
    const int POINTER_ADDRESS_SIZE = sizeof(int);
    const int RECORD_LENGTH_SIZE = sizeof(int);
    const int SLOT_RECORD_NUMBER_SIZE = sizeof(int); 
    const int SLOT_OVERFLOW_SIZE = sizeof(int);
    // Size of 2 pointers for (offset, length) of a record in the slot directory
    const int SLOT_DIRECTORY_SIZE = POINTER_ADDRESS_SIZE * 2;
    // Size of offset address for 4 fields + 1 offset end address of a record
    const int FIELD_OFFSET_POINTER_SIZE = POINTER_ADDRESS_SIZE * 5; 
    // Size of integer fields
    const int ID_FIELD_SIZE = 8;
    const int MAX_RECORD_SIZE = ID_FIELD_SIZE * 2 + 500 + 200;
    const int negativeOne = -1;
    vector<int> blockDirectory; // Map the least-significant-bits of h(id) to a bucket location in EmployeeIndex (e.g., the jth bucket)
                                // can scan to correct bucket using j*BLOCK_SIZE as offset (using seek function)
								// can initialize to a size of 256 (assume that we will never have more than 256 regular (i.e., non-overflow) buckets)
    int n;  // The number of indexes in blockDirectory currently being used
    int i;	// The number of least-significant-bits of h(id) to check. Will need to increase i once n > 2^i
    int numRecords;    // Records currently in index. Used to test whether to increase n
    int nextFreeBlock; // Next place to write a bucket. Should increment it by BLOCK_SIZE whenever a bucket is written to EmployeeIndex
    string fName;      // Name of index file
    int totalNumberOfPage;    // To count the total number of pages 
    float totalNumRecords;  // To count the total number of records
    int addressOfOverflowPage = -1;   // -1: no overflow page, >0: address of overflow page  
    char overflowPage[BLOCK_SIZE] = {};
    int freeSpacePointer = 0;
    int addressOfCurrentField = FIELD_OFFSET_POINTER_SIZE;      // The address of the current field relative to the variable length record
    int addressOfCurrentOffsetField = 0;    // The address of the current offset field relative to the variable length record
    int addressOfPrevPage = -1;
    int pageRemaining = 0;
    int idBits;
    float reHashCondition = 0.0;
    int j = 0;
    char copyPage[BLOCK_SIZE] = {};
    char page[BLOCK_SIZE] = {};
    char emptyPage[BLOCK_SIZE] = {};
    float avgLength = 0.0;

    // Insert new record into index
    void insertRecord(Record record, ofstream *dataFile) {
        
        addressOfPrevPage = -1;

        // [Read a page from the Employee Index file]
        ifstream readFile("EmployeeIndex", ios::in|ios::binary);
        if(!readFile.is_open()){
            throw "Error Opening";
        }

        // Use bitHash function to find bucket and copy the page
        idBits = bitHash(record.id);
        readFile.seekg(blockDirectory[idBits]);
        readFile.read(page,BLOCK_SIZE);

        // Get the address of overflow page
        memcpy(&addressOfOverflowPage, &page[0] + BLOCK_SIZE - 3 * POINTER_ADDRESS_SIZE, POINTER_ADDRESS_SIZE);

        // Get numRecords 
        memcpy(&numRecords, &page[0] + BLOCK_SIZE - 2 * POINTER_ADDRESS_SIZE, POINTER_ADDRESS_SIZE);
        //cout<<"offset:"<<blockDirectory[bitHash(record.id)]<<", numRecords:"<<numRecords<<endl;

        // Get freeSpacePointer
        memcpy(&freeSpacePointer, &page[0] + BLOCK_SIZE-POINTER_ADDRESS_SIZE, POINTER_ADDRESS_SIZE);
        

        // No records written to index yet
        // if (numRecords == 0) {
        //     // Initialize index with first blocks (start with 4)       
        // }

        // Size of string (Name, Bio)
        int nameLength = record.name.size() + 1;
        int bioLength = record.bio.size() + 1;

        // Size of a variable record(its length and its pointer)
        int recordLength = FIELD_OFFSET_POINTER_SIZE + (ID_FIELD_SIZE*2) + nameLength + bioLength;
        int recordRealSize = SLOT_DIRECTORY_SIZE + recordLength;

        // Calculate empty space in the page
        pageRemaining = BLOCK_SIZE - SLOT_DIRECTORY_SIZE - SLOT_OVERFLOW_SIZE - freeSpacePointer - (numRecords*SLOT_DIRECTORY_SIZE);
        //cout<<"pageRemaining:"<<pageRemaining<<endl;

        // [Overflow page] Take neccessary steps if capacity is reached:
        if (pageRemaining < 716){
            // Search the address of empty overflow page 
            while (addressOfOverflowPage>0){
                // Remember the address of current page in the file
                addressOfPrevPage = addressOfOverflowPage;
                //cout<<"previous page address:"<<addressOfPrevPage<<endl;
                overflowPage[BLOCK_SIZE]={};
                readFile.seekg(addressOfOverflowPage);
                readFile.read(overflowPage, BLOCK_SIZE);

                // Update
                memcpy(&addressOfOverflowPage, &overflowPage[0] + BLOCK_SIZE - 3 * POINTER_ADDRESS_SIZE, POINTER_ADDRESS_SIZE);
            }
            // Arrive at the last empty overflow page
            memcpy(&numRecords, &overflowPage[0] + BLOCK_SIZE - 2 * POINTER_ADDRESS_SIZE, POINTER_ADDRESS_SIZE);
            memcpy(&freeSpacePointer, &overflowPage[0] + BLOCK_SIZE - POINTER_ADDRESS_SIZE, POINTER_ADDRESS_SIZE);

            // Initialize the address of overflow page
            addressOfOverflowPage = -1;
         
            // Initialize a page to store the overflow page
            page[BLOCK_SIZE]={};
            memcpy(page, &overflowPage, BLOCK_SIZE);
            
        }

        // Add record to the index in the correct block, creating a overflow block if necessary
        // [Create a variable record for storing record]
        char variableRecord[recordLength];
        
        // The address of the current field relative to the variable length record
        int addressOfCurrentField = FIELD_OFFSET_POINTER_SIZE;
        // The address of the current offset field relative to the variable length record
        int addressOfCurrentOffsetField = 0;

        // Write field offset pointers and fields
        // record.id
        memcpy(variableRecord + addressOfCurrentField, &record.id, sizeof(int));
        //addressOfCurrentField += freeSpacePointer;
        memcpy(variableRecord + addressOfCurrentOffsetField, &addressOfCurrentField, POINTER_ADDRESS_SIZE);
        //addressOfCurrentField -= freeSpacePointer;
        addressOfCurrentField += ID_FIELD_SIZE;
        addressOfCurrentOffsetField += POINTER_ADDRESS_SIZE;

        // record.name
        memcpy(variableRecord + addressOfCurrentField, &record.name[0], nameLength);
        //addressOfCurrentField += freeSpacePointer;
        memcpy(variableRecord + addressOfCurrentOffsetField, &addressOfCurrentField, POINTER_ADDRESS_SIZE);
        //addressOfCurrentField -= freeSpacePointer;
        addressOfCurrentField += nameLength;
        addressOfCurrentOffsetField += POINTER_ADDRESS_SIZE;

        // record.bio
        memcpy(variableRecord + addressOfCurrentField, &record.bio[0], bioLength);
        //addressOfCurrentField += freeSpacePointer;
        memcpy(variableRecord + addressOfCurrentOffsetField, &addressOfCurrentField, POINTER_ADDRESS_SIZE);
        //addressOfCurrentField -= freeSpacePointer;
        addressOfCurrentField += bioLength;
        addressOfCurrentOffsetField += POINTER_ADDRESS_SIZE;

        // record.manager_id
        memcpy(variableRecord + addressOfCurrentField, &record.manager_id, sizeof(int));
        //addressOfCurrentField += freeSpacePointer;
        memcpy(variableRecord + addressOfCurrentOffsetField, &addressOfCurrentField, POINTER_ADDRESS_SIZE);
        //addressOfCurrentField -= freeSpacePointer;
        addressOfCurrentField += ID_FIELD_SIZE;
        addressOfCurrentOffsetField += POINTER_ADDRESS_SIZE;

        // Offset of end of record
        //addressOfCurrentField += freeSpacePointer;
        memcpy(variableRecord + addressOfCurrentOffsetField, &addressOfCurrentField, POINTER_ADDRESS_SIZE);
        //addressOfCurrentField -= freeSpacePointer;

        // Copy the variable record to the page
        memcpy(page + freeSpacePointer, &variableRecord, recordLength);
        int recordAddress = freeSpacePointer;
        freeSpacePointer += recordLength;

        // [Update Slot Directory]
        //  Add (offset, recordLength) to the slot directory in the page 
        memcpy(page + BLOCK_SIZE - POINTER_ADDRESS_SIZE - SLOT_RECORD_NUMBER_SIZE - SLOT_OVERFLOW_SIZE - ((numRecords+1)*8), &recordAddress, POINTER_ADDRESS_SIZE);
        memcpy(page + BLOCK_SIZE - POINTER_ADDRESS_SIZE - SLOT_RECORD_NUMBER_SIZE - SLOT_OVERFLOW_SIZE - ((numRecords+1)*8 - 4), &recordLength, RECORD_LENGTH_SIZE);
        
        //  Update free space pointer and the number of variable length records in the page
        numRecords += 1;
        totalNumRecords += 1;
        memcpy(page + BLOCK_SIZE - POINTER_ADDRESS_SIZE, &freeSpacePointer, POINTER_ADDRESS_SIZE);
        memcpy(page + BLOCK_SIZE - POINTER_ADDRESS_SIZE - SLOT_RECORD_NUMBER_SIZE, &numRecords, SLOT_RECORD_NUMBER_SIZE);
        
        // Calculate empty space in the current page
        pageRemaining = BLOCK_SIZE - SLOT_DIRECTORY_SIZE - SLOT_OVERFLOW_SIZE - freeSpacePointer - (numRecords*SLOT_DIRECTORY_SIZE);
        
        
        //Add overflow page at the end of the existing pages in the file
        if (pageRemaining < MAX_RECORD_SIZE){
            // next overflow page would be (+ # of bucket and overflow pages)
            addressOfOverflowPage = totalNumberOfPage * BLOCK_SIZE;
            
            totalNumberOfPage += 1;
            //cout<<"total pages:"<<totalNumberOfPage<<endl;
        } 

        // Update address of overflow page in the page
        memcpy(page + BLOCK_SIZE - 2 * POINTER_ADDRESS_SIZE - SLOT_RECORD_NUMBER_SIZE, &addressOfOverflowPage, POINTER_ADDRESS_SIZE);

        // Write the page to the appropriate offset of the file
        if (addressOfPrevPage==-1){
            (*dataFile).seekp(blockDirectory[idBits]);
        } else{
            (*dataFile).seekp(addressOfPrevPage);
        }

        (*dataFile).write(page, BLOCK_SIZE);

        // [Re-hashing] increase n; increase i (if necessary); place records in the new bucket that may have been originally misplaced due to a bit flip
        // Calculate Re-Hash condition
        reHashCondition = totalNumRecords/(n*7);

        if(reHashCondition > 0.7){
            // Increase the number of bucket and Add a new ponter address
            n += 1;
            blockDirectory.push_back(n*BLOCK_SIZE);

            // If n is larger than 2^i, read one more LSB bit
            if (n > pow(2,i)){
                i+=1;
            }

        } 

        readFile.close();
    }

public:
    LinearHashIndex(string indexFileName) {
        n = 4; // Start with 4 buckets in index
        i = 2; // Need 2 bits to address 4 buckets
        numRecords = 0;
        nextFreeBlock = 0;
        fName = indexFileName;
        totalNumberOfPage = n;
        idBits = 0;
        

        // Create your EmployeeIndex file 
        ofstream dataFile(fName, ios::out | ios::binary);
        if(!dataFile.is_open()){
            throw "Error creating" + fName;
        }        
        
        // write out the initial 4 buckets
        // make sure to account for the created buckets by incrementing nextFreeBlock appropriately
        char slotDirectory[3*POINTER_ADDRESS_SIZE];
        memcpy(slotDirectory,&addressOfOverflowPage,POINTER_ADDRESS_SIZE);
        memcpy(slotDirectory+POINTER_ADDRESS_SIZE, &numRecords, SLOT_RECORD_NUMBER_SIZE);
        memcpy(slotDirectory+POINTER_ADDRESS_SIZE+SLOT_RECORD_NUMBER_SIZE, &freeSpacePointer, POINTER_ADDRESS_SIZE);
        memcpy(&page[0]+BLOCK_SIZE-3*POINTER_ADDRESS_SIZE, &slotDirectory, sizeof(slotDirectory));

        for (int nextFreeBlock=0; nextFreeBlock<n; nextFreeBlock++){
            blockDirectory.push_back(nextFreeBlock*BLOCK_SIZE);    
            dataFile.write(page, sizeof(page));       
        }

        dataFile.close();
    }

    // Read csv file and add records to the index
    void createFromFile(string csvFName) {
        // Read the csvFName file
        // ifstream inputFile(csvFName);
        ifstream inputFile(csvFName);

        if(!inputFile.is_open()){
            throw "Error inputFile" + csvFName;
        }

        ofstream dataFile(fName, ios::out | ios::binary);
        if(!dataFile.is_open()){
            throw "Error dataFile" + fName;
        }
        
        string line;
        while(getline(inputFile, line)){
            vector<string> fields;
            stringstream ss(line);
            string field;
            while(getline(ss, field, ',')){
                fields.push_back(field);
            }
            Record record(fields);

            insertRecord(record, &dataFile);
        }
        

        inputFile.close();
        dataFile.close();
    }

        // Given an ID, find the relevant record and print it
    Record findRecordById(int id) {

        int idBits = bitHash(id); // Hash the id to find the appropriate bucket.

        ifstream dataFile(fName, ios::in | ios::binary);
        if (!dataFile.is_open()) {
            cout << "Error opening input file " + fName << endl;
            return Record(); // Return an empty Record if file can't be opened
        }

        int blockOffset = blockDirectory[idBits];
        dataFile.seekg(blockOffset);

        // char page[BLOCK_SIZE];
        dataFile.read(page, BLOCK_SIZE);

        memcpy(&freeSpacePointer, &page[BLOCK_SIZE - POINTER_ADDRESS_SIZE], POINTER_ADDRESS_SIZE);

        memcpy(&numRecords, &page[BLOCK_SIZE - 2 * POINTER_ADDRESS_SIZE], POINTER_ADDRESS_SIZE);

        memcpy(&addressOfOverflowPage, &page[BLOCK_SIZE - 3 * POINTER_ADDRESS_SIZE], POINTER_ADDRESS_SIZE);
        
        Record foundRecord;
 
        foundRecord = findingRecord(id);

        if(foundRecord.id==-1){
        // Overflow page
            while(addressOfOverflowPage > 0){
                dataFile.seekg(addressOfOverflowPage);
                page[BLOCK_SIZE] = {};
                dataFile.read(page,BLOCK_SIZE);

                foundRecord = findingRecord(id);

                if(foundRecord.id!=-1){
                    break;
                }

                memcpy(&addressOfOverflowPage, &page[BLOCK_SIZE - 3 * POINTER_ADDRESS_SIZE], POINTER_ADDRESS_SIZE);
            }
        }

        dataFile.close();
        return foundRecord; // Return an empty Record as a signal that the search was unsuccessful.
    
    }

    Record findingRecord (int id) {
        int trackRecordOffset = 0;
            
        // variable for record length in slot directory
        int recordLength = 0;

        // Pointer to record 
        int recordOffsetPointer = BLOCK_SIZE - POINTER_ADDRESS_SIZE * 3 - RECORD_LENGTH_SIZE - RECORD_LENGTH_SIZE;

        for(int i = 0; i < numRecords; i++) {
                // Next Record Offset
                memcpy(&trackRecordOffset, &page[recordOffsetPointer] , sizeof(int));

                // Next Record Length
                memcpy(&recordLength, &page[recordOffsetPointer + sizeof(int)] , sizeof(int));

                // Get the variable-length record using (Record Offset, Record Length)
                char variableRecord[MAX_RECORD_SIZE];
                memcpy(&variableRecord[0], &page[trackRecordOffset], recordLength);

                // Read all field pointers from record
                int idPointer = 0;
                int namePointer = 0;
                int bioPointer = 0;
                int managerIdPointer = 0;
                int endRecordPointer = 0;
                memcpy(&idPointer, &variableRecord[0], POINTER_ADDRESS_SIZE);
                memcpy(&namePointer, &variableRecord[POINTER_ADDRESS_SIZE], POINTER_ADDRESS_SIZE);
                memcpy(&bioPointer, &variableRecord[2 * POINTER_ADDRESS_SIZE], POINTER_ADDRESS_SIZE);
                memcpy(&managerIdPointer, &variableRecord[3 * POINTER_ADDRESS_SIZE], POINTER_ADDRESS_SIZE);
                memcpy(&endRecordPointer, &variableRecord[4 * POINTER_ADDRESS_SIZE], POINTER_ADDRESS_SIZE);

                trackRecordOffset = endRecordPointer;

                // Read employee ID attribute
                int employeeId = 0;
                memcpy(&employeeId, &variableRecord[idPointer], sizeof(int));

                if (employeeId == id) {
                    vector<string> employeeInfo;
                    int eId, eManager_id;
                    string eName, eBio;
                    // read the string
                    memcpy(&eId, &variableRecord[idPointer], sizeof(int));
                    eName.resize(bioPointer - namePointer);
                    memcpy(&eName[0], &variableRecord[namePointer], bioPointer - namePointer);
                    eBio.resize(managerIdPointer - bioPointer);
                    memcpy(&eBio[0], &variableRecord[bioPointer], managerIdPointer - bioPointer);
                    memcpy(&eManager_id, &variableRecord[managerIdPointer], sizeof(int));
                
                    // Assign the string into empty record;
                    employeeInfo.push_back(to_string(eId));
                    employeeInfo.push_back(eName);
                    employeeInfo.push_back(eBio);
                    employeeInfo.push_back(to_string(eManager_id));

                    // dataFile.close();
                    return Record(employeeInfo);
                } else {
                    recordOffsetPointer = recordOffsetPointer - POINTER_ADDRESS_SIZE - RECORD_LENGTH_SIZE;
                }

            }
            
            return Record();
    }

    int bitHash(int id){
        bitset<8> decimalBitset(id % 216);
        i=2;
        unsigned int lsbBits = (decimalBitset.to_ulong()) & ((1<<i)-1);
        
        // int idMode216 = id % 216;
        // int lsbBits = 0;
        // int bitConditionForBucket = pow(2,i);
        // int powI = pow(2,i);
        // int powI1 = pow(2,i-1);

        // if ( bitConditionForBucket % 2 != 0){
        //     lsbBits = idMode216 % powI1;
        // }
        // else{
        //     lsbBits = idMode216 % powI;
        // }
        
        //cout<<"i:"<<i<<", pow(2,i):"<<powI<<", lsbBits:"<<lsbBits<<endl;

        return lsbBits;
    }

    void printVector(vector<int> const &a){
        for (int i=0; i<a.size();i++){
            cout<<a.at(i)<<' ';
        }
    }
};
