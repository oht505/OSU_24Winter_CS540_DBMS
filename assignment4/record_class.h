/* This is a skeleton code for External Memory Sorting, you can make modifications as long as you meet 
   all question requirements*/  
/* This record_class.h contains the class Records, which can be used to store tuples form Emp.csv (stored
in the EmpRecord).
*/
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

using namespace std;

class Records{
    public:
    
    struct EmpRecord {
        int eid;
        string ename;
        int age;
        double salary;
    }emp_record;

    /*** You can add more variables if you want below ***/

    int no_values = 0; //You can use this to check if you've don't have any more tuples
    int number_of_emp_records = 0; // Tracks number of emp_records you have on the buffer

    //constructor
    Records(int eid, string ename, int age, double salary,int no_values) {
        emp_record.eid = eid;
        emp_record.ename = ename;
        emp_record.age = age;
        emp_record.salary = salary;
        this->no_values = no_values;
    }
    //default Controller
    Records(){};


};

// Grab a single block from the Emp.csv file and put it inside the EmpRecord structure of the Records Class
Records Grab_Emp_Record(fstream &empin) {
    string line, word;
    Records emp;

    // grab entire line
    if (getline(empin, line, '\n')) {
        // turn line into a stream
        stringstream s(line);
        // gets everything in stream up to comma
        getline(s, word,',');
        emp.emp_record.eid = stoi(word);
        getline(s, word, ',');
        emp.emp_record.ename = word;
        getline(s, word, ',');
        emp.emp_record.age = stoi(word);
        getline(s, word, ',');
        emp.emp_record.salary = stod(word);

        return emp;
    } else {
        emp.no_values = -1;
        return emp;
    }
}



