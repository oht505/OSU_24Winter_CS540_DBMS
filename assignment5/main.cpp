/* This is a skeleton code for Optimized Merge Sort, you can make modifications as long as you meet 
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
#define BLOCK_SIZE 1280

Records buffers[buffer_size]; //use this class object of size 22 as your main memory

/***You can change return type and arguments as you want.***/

//====================PASS 1==========================

void flushEmpBuffer(Records buffers[]){
    for(int i=0; i<buffer_size; i++){
        buffers[i].emp_record.eid = INT32_MAX;
        buffers[i].emp_record.ename = "";
        buffers[i].emp_record.age = INT32_MAX;
        buffers[i].emp_record.salary = INT32_MAX;
    }
}
void flushDeptBuffer(Records buffers[]){
    for(int i=0; i<buffer_size; i++){
        buffers[i].dept_record.did = INT32_MAX;
        buffers[i].dept_record.dname = "";
        buffers[i].dept_record.budget = INT32_MAX;
        buffers[i].dept_record.managerid = INT32_MAX;
    }
}


void flushBuffer(Records buffers[]){
    // Empty Buffer
    for(int i=0; i<buffer_size; i++){
        buffers[i].emp_record.eid = INT32_MAX;
        buffers[i].emp_record.ename = "";
        buffers[i].emp_record.age = INT32_MAX;
        buffers[i].emp_record.salary = INT32_MAX;

        buffers[i].dept_record.did = INT32_MAX;
        buffers[i].dept_record.dname = "";
        buffers[i].dept_record.budget = INT32_MAX;
        buffers[i].dept_record.managerid = INT32_MAX;
    }
}

int countNumRecordsInFile(istream& inputFile){
    int result=0;
    string line;

    while(getline(inputFile, line, '\n')){
        result++;
    }

    return result;
}

void fillBufferFromEmp(Records buffers[], fstream &dataFile, int n, bool isPassOne){

    // Grab employee records from the file
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

        // Insert each field data into emp_record
        buffers[i].emp_record.age = empRecord.emp_record.age;
        buffers[i].emp_record.eid = empRecord.emp_record.eid;
        buffers[i].emp_record.ename = empRecord.emp_record.ename;
        buffers[i].emp_record.salary = empRecord.emp_record.salary;

    }
}

void fillBufferFromDept(Records buffers[], fstream &dataFile, int n, bool isPassOne){

    // Grab dept records from the file
    for(int i=0; i<n; i++){
        Records deptRecord;
        // Get one record
        if(isPassOne) {
            deptRecord = Grab_Dept_Record(dataFile);
            // For checking the total number of records
            buffers->number_of_dept_records++;

            // No more records in the file
            if(deptRecord.no_values == -1){
                break;
            }
        }

        // Insert each field data into dept_record
        buffers[i].dept_record.did = deptRecord.dept_record.did;
        buffers[i].dept_record.dname = deptRecord.dept_record.dname;
        buffers[i].dept_record.budget = deptRecord.dept_record.budget;
        buffers[i].dept_record.managerid = deptRecord.dept_record.managerid;

    }
}

static bool compareByEmployeeId(const Records& a, const Records& b){
    return a.emp_record.eid < b.emp_record.eid;
}

void sortRecordsByEmployeeId(){
    sort(buffers, buffers+buffer_size, compareByEmployeeId);
}
void sortEmpRecords(int startOffset, int endOffset){
    sort(buffers+startOffset, buffers+endOffset, compareByEmployeeId);
}

static bool compareByManagerId(const Records& c, const Records& d){
    return c.dept_record.managerid < d.dept_record.managerid;
}

void sortRecordsByManagerId(){
    sort(buffers, buffers + buffer_size, compareByManagerId);
}
void sortDeptRecords(int startOffset, int endOffset){
    sort(buffers+startOffset, buffers + endOffset, compareByManagerId);
}

string serialize(Records empInfo, int flag)
{
    ostringstream serializedRecord;

    // Emp
    if(flag==0){
        serializedRecord.write(reinterpret_cast<const char *>(&empInfo.emp_record.eid), sizeof(int));
        serializedRecord << empInfo.emp_record.ename <<",";
        serializedRecord.write(reinterpret_cast<const char*>(&empInfo.emp_record.age), sizeof(int));                                                                 // serialize string bio, variable length
        serializedRecord.write(reinterpret_cast<const char *>(&empInfo.emp_record.salary), sizeof(double));
    }

    // Dept
    if(flag==1){
        serializedRecord.write(reinterpret_cast<const char *>(&empInfo.dept_record.did), sizeof(int));
        serializedRecord << empInfo.dept_record.dname <<",";
        serializedRecord.write(reinterpret_cast<const char*>(&empInfo.dept_record.budget), sizeof(double));                                                                 // serialize string bio, variable length
        serializedRecord.write(reinterpret_cast<const char *>(&empInfo.dept_record.managerid), sizeof(int));
    }

    return serializedRecord.str();
}

void writeRecordToFile(Records buffers[], int bufferIdx, int startOffset, fstream &runFile, int flag){

    if (buffers[bufferIdx].emp_record.eid != INT32_MAX || buffers[bufferIdx].dept_record.managerid != INT32_MAX ) {

        string serializedRecord = serialize(buffers[bufferIdx], flag);
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

void createEmptyRuns(int startOffset, fstream &dataFile){
    // Seek to the appropriate position at the end of the current page
    dataFile.seekp(startOffset + BLOCK_SIZE - 3*sizeof(int));
    int initFreeSpace = 0;
    int initRecordNumInPage = 0;
    int minPointer = 1;
    dataFile.write(reinterpret_cast<const char*>(&minPointer), sizeof(int));
    dataFile.write(reinterpret_cast<const char*>(&initRecordNumInPage), sizeof(int));
    dataFile.write(reinterpret_cast<const char*>(&initFreeSpace), sizeof(int));
}

//Sorting the buffers in main_memory and storing the sorted records into a temporary file (runs)
void Sort_Buffer(Records buffers[], int startOffset, fstream &RunFile, int flag){
    //Remember: You can use only [AT MOST] 22 blocks for sorting the records / tuples and create the runs

    if(flag==0){
        sortRecordsByEmployeeId();
        for(int bufferIdx=0; bufferIdx<buffer_size; bufferIdx++){
            writeRecordToFile(buffers, bufferIdx, startOffset, RunFile, flag);
        }
    }

    if(flag==1){
        sortRecordsByManagerId();
        for(int bufferIdx=0; bufferIdx<buffer_size; bufferIdx++){
            writeRecordToFile(buffers, bufferIdx, startOffset, RunFile, flag);
        }
    }
}


//====================PASS 2==========================
//get records from EmpRun file.
Records getEmpRecordFromRun(fstream& runFile, int pageNum, int recordNum) {
    Records records;
    int slotOffset, recordLength, RecordNumInPage;
    int startOffset = BLOCK_SIZE*pageNum;
    runFile.seekg(startOffset + BLOCK_SIZE - sizeof(int) * 2);
    runFile.read(reinterpret_cast<char *>(&RecordNumInPage), sizeof(int));

    int eid;
    string ename;
    int age;
    double salary;

    runFile.seekg(startOffset + BLOCK_SIZE - sizeof(int)*3 - recordNum*2*sizeof(int));
    runFile.read(reinterpret_cast<char*>(&slotOffset), sizeof(int));
    runFile.read(reinterpret_cast<char*>(&recordLength), sizeof(int));


    runFile.seekg(startOffset+slotOffset);
    runFile.read(reinterpret_cast<char*>(&eid), sizeof(int));
    std::getline(runFile, ename, ',');
    runFile.read(reinterpret_cast<char*>(&age), sizeof(int));
    runFile.read(reinterpret_cast<char*>(&salary), sizeof(double));
//    cout << "recordNum: " << recordNum << "RecordNumInPage: " << RecordNumInPage <<"\n";
    if(recordNum == RecordNumInPage+1)
        return records.initEmpRecord(INT32_MAX,"-1", -1, -1);

    else return records.initEmpRecord(eid,ename,age,salary);

}
//get records from deptRun file.
Records getDeptRecordFromRun(fstream & runFile,int pageNum, int recordNum) {
    Records records;
    int slotOffset, recordLength, RecordNumInPage;
    int startOffset = pageNum*BLOCK_SIZE;

    int did;
    string dname;
    double budget;
    int managerid;

    runFile.seekg(startOffset + BLOCK_SIZE - sizeof(int) * 2);
    runFile.read(reinterpret_cast<char *>(&RecordNumInPage), sizeof(int));
    runFile.seekg(startOffset + BLOCK_SIZE - sizeof(int)*3 - recordNum*2*sizeof(int));
    runFile.read(reinterpret_cast<char*>(&slotOffset), sizeof(int));
    runFile.read(reinterpret_cast<char*>(&recordLength), sizeof(int));

    runFile.seekg(startOffset+slotOffset);
    runFile.read(reinterpret_cast<char*>(&did), sizeof(int));
    std::getline(runFile, dname, ',');
    runFile.read(reinterpret_cast<char*>(&budget), sizeof(double));
    runFile.read(reinterpret_cast<char*>(&managerid), sizeof(int));
    if(recordNum == RecordNumInPage+1)
        return records.initDeptRecord(INT32_MAX,"-1", -1, INT32_MAX);
    else
        return records.initDeptRecord(did,dname,budget,managerid);
}

void insertEmpToBuffer(Records buffers[buffer_size], fstream &empRun,int n){
    cout << "n : " << n << "\n";
    flushEmpBuffer(buffers);
    //sort
    for(int i=0; i<6; i++){
        Records pageEmp = getEmpRecordFromRun(empRun, i,n);
        if(pageEmp.emp_record.eid == INT32_MAX)
            break;
        buffers[i] = pageEmp;
    }
    //sort
    sortEmpRecords(0,6);

}

void insertDeptToBuffer(Records buffers[buffer_size], fstream &deptRun,int n){
    flushDeptBuffer(buffers);
    for(int i=0; i<15; i++){
        Records pageDept = getDeptRecordFromRun(deptRun, i,n);
        if(pageDept.dept_record.managerid == INT32_MAX)
            break;
        buffers[i+6] = pageDept;
    }
    //sort
    sortDeptRecords(6,21);

}

//Prints out the attributes from empRecord and deptRecord when a join condition is met
//and puts it in file Join.csv
void PrintJoin(fstream &Join) {

    Records records = buffers[buffer_size-1];
    int eid = records.emp_record.eid;
    string ename = records.emp_record.ename;
    int age = records.emp_record.age;
    double salary = records.emp_record.salary;
    int did = records.dept_record.did;
    string dname = records.dept_record.dname;
    double budget = records.dept_record.budget;

    string serializedRecord = std::to_string(eid) + "," + ename + "," + std::to_string(age) + "," + std::to_string(salary);
    serializedRecord += std::to_string(did)+ "," +dname+ "," +std::to_string(budget)+  "\n";

    //cout <<serializedRecord;
    Join << serializedRecord;
    //empSorted.close();

    return;
}

void printBuffer(){
    cout <<"===================\n";
    for(int i=0; i<6;i++){
        cout << buffers[i].emp_record.eid<<"\n";
        cout << buffers[i].emp_record.ename<<"\n";
        cout << buffers[i].emp_record.salary<<"\n";
        cout << buffers[i].emp_record.age<<"\n";
    }
    for(int i=6; i<21;i++) {
        cout << buffers[i].dept_record.did << "\n";
        cout << buffers[i].dept_record.dname << "\n";
        cout << buffers[i].dept_record.budget << "\n";
        cout << buffers[i].dept_record.managerid << "\n";
    }
}

//Use main memory to Merge and Join tuples
//which are already sorted in 'runs' of the relations Dept and Emp
void Merge_Join_Runs(fstream &empRun, fstream &deptRun, int totalNumDeptRecord,fstream &joinout){
    int deptRecordNum = 1;
    int empRecordNum =1;
    int ptrEmp = 0;
    int ptrDept = 6;
    int empAllocSize = ptrEmp+ 6;
    int deptAllocSize = ptrDept+15;

    Records empRecord;
    Records deptRecord;
    //flushBuffer(buffers);
    insertEmpToBuffer(buffers, empRun,empRecordNum);
    insertDeptToBuffer(buffers, deptRun,deptRecordNum);
//    sortEmpRecords(ptrEmp,empAllocSize);
   // sortDeptRecords(ptrDept,deptAllocSize);
   // printBuffer();

    while(totalNumDeptRecord>0){// dept 소진시 까지 while
        //printBuffer();
        empRecord = buffers[ptrEmp];
        deptRecord = buffers[ptrDept];
        int presentEmpId = empRecord.emp_record.eid;
        int presentMangerId = deptRecord.dept_record.managerid;

        if(presentEmpId == presentMangerId && presentEmpId!=INT32_MAX){
            Records joinRecord(empRecord.emp_record,deptRecord.dept_record);
            //PrintJoin();
            cout << "JOINED! : " <<presentEmpId <<"\n";
            totalNumDeptRecord--;
            buffers[buffer_size-1] = joinRecord;
            ptrDept++;
            PrintJoin(joinout);

        }
        else{
            if(ptrEmp+1 == empAllocSize){
                insertEmpToBuffer(buffers, empRun,++empRecordNum);
                ptrDept = 6;
                ptrEmp = 0;
            }
            else if(ptrDept == deptAllocSize){
                insertDeptToBuffer(buffers, deptRun,++deptRecordNum);
                ptrDept = 6;
            }
            else{
                if(presentEmpId > presentMangerId) {
                    ptrDept++;
                }
                else if(presentEmpId < presentMangerId) {
                    ptrEmp++;
                }
                else if(presentEmpId == presentMangerId){
                    break;
                }
            }
        }
    }
    //printBuffer();
    //and store the Joined new tuples using PrintJoin()
    return;
}

void eraseRuns(const char* filename){
    // Attempt to remove the file
    if (std::remove(filename) == 0) {
        std::cout << "File " << filename << " has been successfully deleted.\n";
    } else {
        std::cerr << "Error deleting file " << filename << ".\n";
    }
}


int main() {

    int totalNumEmpRecords = 0;
    int totalNumEmpPages = 0;
    int totalNumDeptRecords = 0;
    int totalNumDeptPages = 0;

    //Open file streams to read and write
    //Opening out two relations Emp.csv and Dept.csv which we want to join
    fstream empin;
    empin.open("Emp.csv", ios::in);
    if(!empin.is_open()){
        cerr<<"Can't open file "<<"Emp.csv"<<endl;
    }

    fstream deptin;
    deptin.open("Dept.csv", ios::in);
    if(!deptin.is_open()){
        cerr<<"Can't open file "<<"Dept.csv"<<endl;
    }

    // Total number of Records and Runs in Emp.csv and Dept.csv
    totalNumEmpRecords = countNumRecordsInFile(empin);
    totalNumEmpPages = 6;
    //totalNumEmpPages = (totalNumEmpRecords / (totalNumEmpRecords/6)) + (totalNumEmpRecords % 15 != 0);

    totalNumDeptRecords = countNumRecordsInFile(deptin);
    totalNumDeptPages = 15;
    //totalNumDeptPages = totalNumDeptRecords / 15 + (totalNumDeptRecords % 15 != 0);

    // Clear flags and Move pointer to the beginning of the file
    empin.clear();
    empin.seekg(0, ios::beg);

    deptin.clear();
    deptin.seekg(0, ios::beg);

    cout<<totalNumEmpRecords<<","<<totalNumEmpPages<<","<<totalNumDeptRecords<<","<<totalNumDeptPages<<endl;

    // Create Emp Runs File
    fstream emp_runs;
    emp_runs.open("Emp_runs", ios::in | ios::out | ios:: trunc | ios::binary);
    if(!emp_runs.is_open()){
        cerr<<"Can't open file "<<"Emp_runs.csv"<<endl;
    }

    // Create Dept Runs File
    fstream dept_runs;
    dept_runs.open("Dept_runs", ios::in | ios::out | ios:: trunc | ios::binary);
    if(!dept_runs.is_open()){
        cerr<<"Can't open file "<<"dept_runs.csv"<<endl;
    }

    //Creating the Join.csv file where we will store our joined results
    fstream joinout;
    joinout.open("Join.csv", ios::out | ios::app);
    if(!joinout.is_open()){
        cerr<<"Can't open file "<<"join.csv"<<endl;
    }

    //1. Create runs for Dept and Emp which are sorted using Sort_Buffer()
    // Emp_runs
    int flagForFile = 0;
    int isPassone = 1;
    int empRecordsNumInPage[6]={};
    int empRemainder = totalNumEmpRecords % 6;

    cout<<"Remainder:"<<empRemainder<<", totalEmpRecords:"<<totalNumEmpRecords<<endl;

    // Array for the number of Emp records in each page
    for(int i=0; i<totalNumEmpPages; i++){
        empRecordsNumInPage[i]=totalNumEmpRecords/6;
        if(empRemainder>0){
            empRecordsNumInPage[i]+=1;
            empRemainder-=1;
        }
        cout<<empRecordsNumInPage[i]<<endl;
    }

    for(int i=0; i<totalNumEmpPages; i++){
        createEmptyRuns(i*BLOCK_SIZE, emp_runs);
        flushBuffer(buffers);
        fillBufferFromEmp(buffers, empin, empRecordsNumInPage[i], isPassone);
        Sort_Buffer(buffers, i*BLOCK_SIZE, emp_runs, flagForFile);
    }

    // Dept_runs
    flagForFile = 1;
    int deptRecordsNumInPage[15]={};
    int deptRemainder = totalNumDeptRecords % 15;

    // Array for the number of Dept records in each page
    for(int i=0; i<totalNumDeptPages; i++){
        deptRecordsNumInPage[i]=totalNumDeptRecords/15;
        if(deptRemainder>0){
            deptRecordsNumInPage[i]+=1;
            deptRemainder-=1;
        }
        //cout<<deptRecordsNumInPage[i]<<endl;
    }

    for(int i=0; i<totalNumDeptPages; i++){
        createEmptyRuns(i*BLOCK_SIZE, dept_runs);
        flushBuffer(buffers);
        fillBufferFromDept(buffers, deptin, deptRecordsNumInPage[i], isPassone);
        Sort_Buffer(buffers, i*BLOCK_SIZE, dept_runs, flagForFile);
    }


    //2. Use Merge_Join_Runs() to Join the runs of Dept and Emp relations
    Merge_Join_Runs(emp_runs,dept_runs,totalNumDeptRecords,joinout);

    //Please delete the temporary files (runs) after you've joined both Emp.csv and Dept.csv
    eraseRuns("Emp_runs");
    eraseRuns("Dept_runs");


}
