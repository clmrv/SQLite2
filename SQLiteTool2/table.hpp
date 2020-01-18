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

class Relation;

class Table {
private:
    std::string name;
    std::vector<std::string> colNames;
    std::vector<std::string> colTypes;
    std::vector< std::vector<std::string> > data;
    Relation* relation;
    
    std::string sql;

public:
    Table();
    void setSql(std::string code);
    void setTableName(std::string tableName);
    
    std::string getTableName();
    std::string getColName(int index);
    std::string getColType(int index);
    std::string getData(int row, int col);
    int getColCount();
    int getRowCount();
    std::string getSql();
    
    void readData(sqlite3* database);
    void removeRow(int index);
    void editField(int row, int col, std::string value);
    void addRow();
    Relation* addRelation(std::string homeCol, Table* foreignTable, std::string foreignCol);
    void removeRelation();
    
    std::string getSaveQuery();
    std::string getDataQuery();
};

class Relation {
public:
    Table* homeTable;
    Table* foreignTable;
    
    int homeCol;
    int foreignCol;
};




#endif /* table_hpp */
