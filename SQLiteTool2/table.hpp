//
//  table.hpp
//  sqlitetool
//
//  Created by celmer on 19/11/2019.
//  Copyright © 2019 celmer. All rights reserved.
//

#ifndef table_hpp
#define table_hpp

#include <vector>
#include <string>

class Table {
private:
    std::string name;
    std::vector<std::string> colNames;
    std::vector<std::string> colTypes;
    std::vector< std::vector<std::string> > data;
    std::string sql;

public:
    void setSql(std::string code);
    void setTableName(std::string tableName);
    
    std::string getTableName();
    std::string getColName(int index);
    std::string getColType(int index);
    std::string getData(int row, int col);
    int getColCount();
    int getRowCount();
    
    void readData(sqlite3* database);
    void removeRow(int index);
    void editField(int row, int col, std::string value);
    
    
    std::string getSaveQuery(); // old
};




#endif /* table_hpp */
