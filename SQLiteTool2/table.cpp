//
//  table.cpp
//  sqlitetool
//
//  Created by celmer on 19/11/2019.
//  Copyright © 2019 celmer. All rights reserved.
//

#include "sqlite2.hpp"
using namespace std;

Table::Table() {
    relation = nullptr;
}

string Table::getSaveQuery() {
    string query = "CREATE TABLE ";
    query.append(name);
    query.append(" (");
    for (int i=0; i<colNames.size(); i++)
    {
        query.append(colNames[i]);
        query.append(" ");
        query.append(colTypes[i]);
        if (i != colNames.size()-1)
            query.append(", ");
    }
    if (relation != nullptr)
    {
        query.append(", foreign KEY(");
        query.append(getColName(relation->homeCol));
        query.append(") REFERENCES ");
        query.append(relation->foreignTable->getTableName());
        query.append("(");
        query.append(relation->foreignTable->getColName(relation->foreignCol));
        query.append(")");
    }
    query.append(");");
    return query;
}

string Table::getDataQuery() {
    string query = "INSERT INTO ";
    query.append(name);
    query.append(" VALUES ");
    for (int i=0; i < getRowCount(); i++)
    {
        query.append("(");
        for (int j=0; j < getColCount(); j++)
        {
            if ( !isdigit(getData(i, j)[0]))
                query.append("\"");
            
            query.append(getData(i, j));
            
            if ( !isdigit(getData(i, j)[0]))
                query.append("\"");
            
            if (j != getColCount()-1)
                query.append(",");
        }
        query.append(")");
        
        if (i != getRowCount()-1)
            query.append(",");
    }
    query.append(";");
    return query;
}

void Table::setTableName(string tableName) {
    name.assign(tableName);
}

void Table::setColName(int col, std::string name) {
    colNames[col].assign(name);
}
void Table::setColType(int col, std::string type) {
    colTypes[col].assign(type);
}

string Table::getTableName() {
    return name;
}

void Table::setSql(string code) {
    sql.assign(code);
}
int Table::getColCount() {
    return (int)colNames.size();
}
int Table::getRowCount() {
    return (int)data.size();
}

std::string Table::getColName(int index) {
    return colNames[index];
}
std::string Table::getColType(int index) {
    return colTypes[index];
}
std::string Table::getData(int row, int col) {
    return data[row][col];
}

std::string Table::getSql() {
    return sql;
}

void Table::readData(sqlite3* database) {
    // FIELDS (column headers, types)
    string query;
    query.assign("PRAGMA table_info('");
    query.append(name);
    query.append("');");
    sqlite3_stmt* statement;
    sqlite3_prepare_v2(database, query.c_str(), (int)query.size(), &statement, 0);
    while(sqlite3_step(statement) != SQLITE_DONE)
    {
        colNames.push_back((char*)sqlite3_column_text(statement, 1));
        colTypes.push_back((char*)sqlite3_column_text(statement, 2));
    }
    sqlite3_finalize(statement);
    
    // FIELDS (data)
    query.assign("SELECT * FROM ");
    query.append(name);
    query.append(";");
    sqlite3_prepare_v2(database, query.c_str(), (int)query.size(), &statement, 0);
    vector<string> *dataBuff;
    while(sqlite3_step(statement) != SQLITE_DONE)
    {
        dataBuff = new vector<string>;
        for (int i=0; i<sqlite3_column_count(statement); i++)
        {
            // na razie wszystko jako text
            dataBuff->push_back((char*)sqlite3_column_text(statement, i));
        }
        data.push_back(*dataBuff);
    }
    sqlite3_finalize(statement);
}

void Table::removeRow(int index) {
    data.erase(data.begin()+index);
}

void Table::addRow() {
    string strBuff;
    vector<string> rowBuff;
    for (int i=0; i<getColCount(); i++)
        rowBuff.push_back(strBuff);
    data.push_back(rowBuff);
}

void Table::addCol() {
    string colname = "col";
    colname.append(to_string(getColCount()));
    colNames.push_back(colname);
    colTypes.push_back("text");
    for (int i=0; i < getRowCount(); i++)
        data[i].push_back("");
}

void Table::delCol(int col) {
    colNames.erase(colNames.begin()+col);
    colTypes.erase(colTypes.begin()+col);
    for (int i=0; i < data.size(); i++)
        data[i].erase(data[i].begin()+col);
}

void Table::editField(int row, int col, std::string value) {
    data[row][col].assign(value);
}

Relation* Table::addRelation(std::string homeCol, Table* foreignTable, std::string foreignCol) {
    relation = new Relation;
    
    relation->homeTable = this;
    for (int i=0; i < getColCount(); i++)
        if (homeCol == colNames[i])
            relation->homeCol = i;
    
    relation->foreignTable = foreignTable;
    for (int i=0; i < foreignTable->getColCount(); i++)
        if (foreignCol == foreignTable->colNames[i])
            relation->foreignCol = i;
    
    return relation;
}

void Table::removeRelation() {
    delete relation;
    relation = nullptr;
}


