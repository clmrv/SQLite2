//
//  sqlite2.cpp
//  SQLiteTool2
//
//  Created by celmer on 11/01/2020.
//  Copyright © 2020 celmer. All rights reserved.
//

#include "sqlite2.hpp"
#include <iostream>

void SQLite2::setEntry(std::string field, std::string value) {
    
}
std::string SQLite2::getEntry(std::string field) {
    return "xd";
}

void SQLite2::init() {
    selectedScreen = 0;
    selectedRow = 0;
    selectedCol = 0;
    selectedTable = 1;
    selectedRelation = 0;
}

void SQLite2::redraw() {
    switch(selectedScreen) {
        case S_TABLES:
            drawTables();
            break;
            
        case S_FIELDS:
            drawFields();
            break;
            
        default:
            break;
    }
}

void SQLite2::openDatabase() {
    fileName = "sqlite.db";
    sqlite3* database;
    sqlite3_open(fileName.c_str(), &database);
    readData(database);
    sqlite3_close(database);
}

void SQLite2::readData(sqlite3* database) {
    // TABLES (names, sql)
    Table *tableBuff;
    std::string query = "SELECT name,sql FROM sqlite_master WHERE type = 'table'";
    sqlite3_stmt* statement;
    sqlite3_prepare_v2(database, query.c_str(), (int)query.size(), &statement, 0);
    while(sqlite3_step(statement) != SQLITE_DONE)
    {
        tableBuff = new Table;
        tableBuff->setTableName((char*)sqlite3_column_text( statement, 0));
        tableBuff->setSql((char*)sqlite3_column_text( statement, 1));
        tables.push_back(*tableBuff);
    }
    sqlite3_finalize(statement);
    
    // FIELDS (column headers, types)
    for (int table_nr=0; table_nr<tables.size(); table_nr++)
        tables[table_nr].readData(database);
}

void SQLite2::clearScr() {
    for (int y=1; y<getmaxy(stdscr)-3; y++) {
        move( y, 0);
        clrtoeol();
    }
}

void SQLite2::drawTitle(const char* titleName) {
    std::string title("--- ");
    title.append(titleName);
    title.append(" ---");
    mvprintw(1, getmaxx(stdscr)/2-title.size()/2, title.c_str());
}

void SQLite2::drawTables() {
    selectedScreen = S_TABLES;
    clearScr();
    drawTitle("TABLES");
    for (int i=0; i<tables.size(); i++)
    {
        if (i == selectedTable)
            attron(A_STANDOUT);
        mvprintw(2+i, 0, tables[i].getTableName().c_str());
        if (i == selectedTable)
            attroff(A_STANDOUT);
    }
    refresh();
}

void SQLite2::drawFields() {
    selectedScreen = S_FIELDS;
    clearScr();
    drawTitle(tables[selectedTable].getTableName().c_str());
    int id = selectedTable;
    
    // column headers
    for (int x=0; x < tables[id].getColCount(); x++)
        mvprintw(2, x*field_width, "%s", tables[id].getColName(x).c_str());
    
    // line
    move(3,0);
    for (int i=0; i < field_width*tables[id].getColCount(); i++)
        printw("-");
    
    // data
    for (int y=0; y < tables[id].getRowCount(); y++)
    {
        for (int x=0; x < tables[id].getColCount(); x++)
        {
            if (y == selectedRow && x == selectedCol)
                attron(A_STANDOUT);
            mvprintw(4+y, x*field_width, "%s", tables[id].getData(y,x).c_str());
            if (y == selectedRow && x == selectedCol)
                attroff(A_STANDOUT);
        }
    }
    refresh();
}

void SQLite2::remove() {
    switch(selectedScreen) {
        case S_TABLES:
            tables.erase(tables.begin()+selectedTable);
            break;
            
        case S_FIELDS:
            tables[selectedTable].removeRow(selectedRow);
            break;
            
        default:
            break;
    }
    redraw();
}

void SQLite2::add() {
    switch(selectedScreen) {
        case S_TABLES:
            // ADD TABLE
            break;
            
        case S_FIELDS:
            // ADD ROW
            break;
            
        default:
            break;
    }
    redraw();
}

void SQLite2::edit() {
    switch(selectedScreen) {
        case S_TABLES:
            tables[selectedTable].setTableName( "tmp" );
            break;
            
        case S_FIELDS:
            tables[selectedTable].editField(selectedRow, selectedCol, "tmp" );
            break;
            
        default:
            break;
    }
    redraw();
}
