/* This is a skeleton code for External Memory Sorting, you can make modifications as long as you meet 
   all question requirements*/  

#include <string>
#include <ios>
#include <fstream>
#include <vector>
#include <string>
#include <string.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cmath> 
#include <algorithm>
#include "record_class.h"

using namespace std;

//defines how many blocks are available in the Main Memory 
#define buffer_size 22
#define BLOCK_SIZE 1536

Records buffers[buffer_size]; //use this class object of size 22 as your main memory
Records getMinRecord(fstream &runFile,int pageNum);

void flushBuffer(Records buffers[]){

    // Empty buffer
    for(int i=0; i<buffer_size; i++){

        buffers[i].emp_record.age = INT32_MAX;
        buffers[i].emp_record.eid = INT32_MAX;
        buffers[i].emp_record.ename = "";
        buffers[i].emp_record.salary = INT32_MAX;
    }
}

//====================PASS 1==========================
int countNumRecordsInFile(istream& inputFile){
    int totalNumRecords = 0;
    string line;


    while(getline(inputFile, line, '\n')){
        totalNumRecords++;
    }

    return totalNumRecords;
}

void PrintBufferEmployeeInfo(int j){

    cout<<"bufferIdx:"<<j<<"\n";

    cout<<"eid:"<<buffers[j].emp_record.eid<<", ";
    cout<<"ename:"<<buffers[j].emp_record.ename<<", ";
    cout<<"age:"<<buffers[j].emp_record.age<<", ";
    cout<<"salary:"<<buffers[j].emp_record.salary<<endl;

}

void fillBufferFromFile(Records buffers[buffer_size], fstream &dataFile,int n, bool isPassOne){

    // Grab 22 employ records from the file
    for(int i=0; i<n; i++){
        Records empRecord;
        // Get one record
        if(isPassOne) {
            empRecord = Grab_Emp_Record(dataFile);
            // For checking the total number of records
            buffers->number_of_emp_records++;

            // No more records in the file
            if(empRecord.no_values == -1){
                break;
            }
        }
        else {
            empRecord = getMinRecord(dataFile,i);
        }

        // Insert each field data into emp_record
        buffers[i].emp_record.age = empRecord.emp_record.age;
        buffers[i].emp_record.eid = empRecord.emp_record.eid;
        buffers[i].emp_record.ename = empRecord.emp_record.ename;
        buffers[i].emp_record.salary = empRecord.emp_record.salary;

        PrintBufferEmployeeInfo(i);

    }
}



static bool compareByEmployeeId(const Records& a, const Records& b){
    return a.emp_record.eid < b.emp_record.eid;
}

void sortRecordsByEmployeeId(){
        sort(buffers, buffers+buffer_size, compareByEmployeeId);
}

string serialize(Records empInfo)
{
    ostringstream serializedRecord;
    serializedRecord.write(reinterpret_cast<const char *>(&empInfo.emp_record.eid), sizeof(int));
    serializedRecord << empInfo.emp_record.ename <<",";
    serializedRecord.write(reinterpret_cast<const char*>(&empInfo.emp_record.age), sizeof(int));                                                                 // serialize string bio, variable length
    serializedRecord.write(reinterpret_cast<const char *>(&empInfo.emp_record.salary), sizeof(double));
    return serializedRecord.str();
}

void writeRecordToFile(Records buffers[], int bufferIdx, int startOffset, fstream &runFile){
    if (buffers[bufferIdx].emp_record.eid != INT32_MAX) {

        string serializedRecord = serialize(buffers[bufferIdx]);
        int nextFreeSpace;
        int RecordNumInPage;
        int minPointer;
        int recordLength = serializedRecord.size();

        // Get minPointer, RecordNumInpage, nextFreeSpace pointer
        runFile.seekg(startOffset + BLOCK_SIZE - sizeof(int) * 3);
        runFile.read(reinterpret_cast<char *>(&minPointer), sizeof(int));
        runFile.read(reinterpret_cast<char *>(&RecordNumInPage), sizeof(int));
        runFile.read(reinterpret_cast<char *>(&nextFreeSpace), sizeof(int));

        // Write the serialized record to the file
        runFile.seekp(startOffset + nextFreeSpace);
        runFile.write(serializedRecord.c_str(), serializedRecord.size());


        // Add slot (offset, recold)
        runFile.seekp(startOffset + BLOCK_SIZE - (sizeof(int) * 3 + sizeof(int) * 2 * (RecordNumInPage + 1)));
        runFile.write(reinterpret_cast<char *>(&nextFreeSpace), sizeof(int));
        runFile.write(reinterpret_cast<char *>(&recordLength), sizeof(int));

        // Update the next free space pointer
        nextFreeSpace += recordLength;
        RecordNumInPage++;

        // Write the updated next free space poointer back to the file
        runFile.seekp(startOffset + BLOCK_SIZE - sizeof(int) * 2);
        runFile.write(reinterpret_cast<const char *>(&RecordNumInPage), sizeof(int));
        runFile.write(reinterpret_cast<const char *>(&nextFreeSpace), sizeof(int));
    }
}

void createRunPage(int startOffset, fstream &dataFile){
        // Seek to the appropriate position at the end of the current page
        dataFile.seekp(startOffset + BLOCK_SIZE - 3*sizeof(int));
        int initFreeSpace = 0;
        int initRecordNumInPage = 0;
        int minPointer = 1;
        dataFile.write(reinterpret_cast<const char*>(&minPointer), sizeof(int));
        dataFile.write(reinterpret_cast<const char*>(&initRecordNumInPage), sizeof(int));
        dataFile.write(reinterpret_cast<const char*>(&initFreeSpace), sizeof(int));
}


//====================PASS 2==========================
Records getMinRecord(fstream &runFile,int pageNum){
    int slotOffset;
    int recordLength;
    int RecordNumInPage;
    int minPointer;

    int startOffset = BLOCK_SIZE*pageNum;

    runFile.seekg(startOffset + BLOCK_SIZE - sizeof(int)*3);
    runFile.read(reinterpret_cast<char*>(&minPointer), sizeof(int));
    runFile.read(reinterpret_cast<char*>(&RecordNumInPage), sizeof(int));

    runFile.seekg(startOffset + BLOCK_SIZE - sizeof(int)*3 - minPointer*2*sizeof(int));
    runFile.read(reinterpret_cast<char*>(&slotOffset), sizeof(int));
    runFile.read(reinterpret_cast<char*>(&recordLength), sizeof(int));

    int eid;
    string ename;
    int age;
    double salary;

    runFile.seekg(startOffset+slotOffset);
    runFile.read(reinterpret_cast<char*>(&eid), sizeof(int));
    std::getline(runFile, ename, ',');
    runFile.read(reinterpret_cast<char*>(&age), sizeof(int));
    runFile.read(reinterpret_cast<char*>(&salary), sizeof(double));

    if(minPointer == RecordNumInPage+1)
        return Records(INT32_MAX, "-1", -1,-1,-1);
    else
        return Records(eid,ename,age,salary,0);
}

int findMinIndexFromBuffer(){
    int minIdx=0; // Assuming the first buffer is the initial minimum
    for(int i=0; i<19; i++){
        if(buffers[minIdx].no_values == -1  && buffers[i].no_values == -1) {
            continue;
        }
        else if(buffers[minIdx].emp_record.eid > buffers[i].emp_record.eid)
            minIdx= i;
    }
    return minIdx;
}

void moveMinToLast(fstream &runFile){
    int minIdx = findMinIndexFromBuffer();
    int minPointer;
    buffers[19].emp_record = buffers[minIdx].emp_record;

    runFile.seekg((minIdx+1)*BLOCK_SIZE - sizeof(int)*3);
    runFile.read(reinterpret_cast<char*>(&minPointer), sizeof(int));
    minPointer++;

    runFile.seekg((minIdx+1)*BLOCK_SIZE - sizeof(int)*3);
    runFile.write(reinterpret_cast<char*>(&minPointer), sizeof(int));

}



/***You can change return type and arguments as you want.***/
//PASS 1
//Sorting the buffers in main_memory and storing the sorted records into a temporary file (Runs) 
void Sort_Buffer(Records buffers[], int startOffset, fstream &runFile){
    //Remember: You can use only [AT MOST] 22 blocks for sorting the records / tuples and create the runs

    // Sort records in the buffer
    sortRecordsByEmployeeId();
    // Insert 22 records into the Run page in the file
    for(int bufferIdx=0; bufferIdx<buffer_size; bufferIdx++){
        writeRecordToFile(buffers, bufferIdx, startOffset, runFile);
    }
    return;
}

//PASS 2
//Use main memory to Merge the Runs
//which are already sorted in 'runs' of the relation Emp.csv
void Merge_Runs(fstream &runFile){
    //and store the Sorted results of your Buffer using PrintSorted()
    fillBufferFromFile(buffers,runFile,19,false);
    moveMinToLast(runFile);
    return;
}


void PrintSorted(fstream &empSorted)   {
    //Store in EmpSorted.csv

    Records records = buffers[19];
    int eid = records.emp_record.eid;
    string ename = records.emp_record.ename;
    int age = records.emp_record.age;
    double salary = records.emp_record.salary;

    string serializedRecord = std::to_string(eid) + "," + ename + "," + std::to_string(age) + "," + std::to_string(salary) + "\n";

    //cout <<serializedRecord;
    empSorted << serializedRecord;
    //empSorted.close();
}


int main() {
    int totalNumRecords = 0;
    int totalNumPages = 0;

    //Open file streams to read and write
    //Opening out the Emp.csv relation that we want to Sort
    fstream empin;
    empin.open("Emp.csv", ios::in);
    if(!empin.is_open()){
        cerr << "Error opening file"<<endl;
        return 1;
    }

    // Total number of Records and Run Pages
    totalNumRecords = countNumRecordsInFile(empin);
    totalNumPages = totalNumRecords / 22 + (totalNumRecords % 22 != 0);

    // Clear flags 
    empin.clear();
    // Move reading pointer at the beginning of the file
    empin.seekg(0, ios::beg);

    //Creating the EmpSorted.csv file where we will store our sorted results
    fstream SortOut("EmpSorted.csv", ios::in | ios:: trunc | ios::out | ios::binary);

    if(!SortOut.is_open()){
        cerr << "Error opening the file to write"<<endl;
        return 1;
    }

    //1. Create runs for Emp which are sorted using Sort_Buffer()
    fstream Runs("run", ios::in | ios:: trunc | ios::out | ios::binary);
    //Runs << fixed << setprecision(0) ;
    if(!Runs.is_open()){
        cerr << "Error opening the file to write" << endl;
        return 1;
    }    

    for(int i=0; i<totalNumPages; i++){
        //1-1. Create an Run page in the file
        createRunPage(i*BLOCK_SIZE, Runs);
        //1-2. Fill buffer with 22 employee records
        flushBuffer(buffers);
        fillBufferFromFile(buffers, empin,buffer_size, true);
        //1-3. Sort records in the buffer and write to the Run File
        Sort_Buffer(buffers, i*BLOCK_SIZE ,Runs);
    }

    //2. Use Merge_Runs() to Sort the runs of Emp relations
    for(int i=0;i<totalNumRecords;i++){
        Merge_Runs(Runs);
        PrintSorted(SortOut);
    }

    cout << totalNumRecords;

    //Please delete the temporary files (runs) after you've sorted the Emp.csv
    empin.close();
    SortOut.close();
    Runs.close();

    return 0;
}
