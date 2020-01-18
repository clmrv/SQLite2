//
//  sqlite2.cpp
//  SQLiteTool2
//
//  Created by celmer on 11/01/2020.
//  Copyright © 2020 celmer. All rights reserved.
//

#include "sqlite2.hpp"

void SQLite2::setEntry(std::string field, std::string value) {
    if (field == "FILE_OPEN")
    {
        if (value.length() > 0)
        {
            file_status = F_CURR;
            fileName.assign(value);
        }
    }
    
    if (field == "FILE_SAVE")
    {
        if (value.length() > 0 && file_status == F_CURR)
        {
            file_status = F_NEW;
            fileName.assign(value);
        }
    }
    
    if (field == "EDIT_VALUE")
        editValue.assign(value);
    
    if (field == "KEY")
    {
        if (value == "<LARROW>")
            leftArrow();
        if (value == "<RARROW>")
            rightArrow();
        if (value == "<UARROW>")
            upArrow();
        if (value == "<DARROW>")
            downArrow();
    }
}

std::string SQLite2::getEntry(std::string field) {
    if (field == "IS_SAVED")
    {
        if (isSaved)
            return "YES";
        else
            return "NO";
    }
    return "";
}

void SQLite2::bindBinds() {
    // nice-like
    backend->bind("#nice#.File.Open${Database file name:|FILE_OPEN}", [this](){this->openDatabase();}, "Open SQLite database file.");
    backend->bind("#nice#.File.Save${Save as:|FILE_SAVE}", [this](){this->saveDatabase();}, "Save database.");
    backend->bind("#nice#.Database.Tables", [this](){this->drawTables();}, "Show list of tables.");
    backend->bind("#nice#.Database.Fields", [this](){this->drawFields();}, "Show fields of table.");
    backend->bind("#nice#.Database.Relations", [this](){this->drawRelations();}, "Show list of relationships.");
    
    backend->bind("#nice#.Edit.Add", [this](){this->add();}, "Add new item.");
    backend->bind("#nice#.Edit.Edit${New value:|EDIT_VALUE}", [this](){this->edit();}, "Edit selected item.");
    backend->bind("#nice#.Edit.Delete", [this](){this->remove();}, "Delete selected item.");

    // nano-like
    // TUTAJ JEST DODANY DODATKOWY ZNAK
    backend->bind("#nano#<CTRL>O%Open!Database file name:${FILE_OPENX}",[this](){this->openDatabase();}, "Open SQLite database file.");
    backend->bind("#nano#<CTRL>S%Save!Save as:${FILE_SAVEX}",[this](){this->saveDatabase();}, "Save database.");
    backend->bind("#nano#<CTRL>T%Tables", [this](){this->drawTables();}, "Show tables.");
    backend->bind("#nano#<CTRL>F%Fields", [this](){this->drawFields();}, "Show data in table.");
    backend->bind("#nano#<CTRL>R%Relations", [this](){this->drawRelations();}, "Show relationship between tables.");
    
    backend->bind("#nano#<CTRL>A%Add", [this](){this->add();}, "Add new item.");
    backend->bind("#nano#<CTRL>D%Delete", [this](){this->remove();}, "Remove selected item.");
    backend->bind("#nano#<CTRL>E%Edit!New value:${EDIT_VALUEX}", [this](){this->edit();}, "Edit selected item.");
    
    backend->bind("#nano#<DARROW>", [this](){this->downArrow();}, "Navigate.");
    backend->bind("#nano#<UARROW>", [this](){this->upArrow();}, "Navigate.");
    backend->bind("#nano#<LARROW>", [this](){this->leftArrow();}, "Navigate.");
    backend->bind("#nano#<RARROW>", [this](){this->rightArrow();}, "Navigate.");
}

void SQLite2::init() {
    bindBinds();
    file_status = F_NONE;
    isSaved = true;
    selectedScreen = 0;
    selectedRow = 0;
    selectedCol = 0;
    selectedTable = 0;
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
            
        case S_RELATIONS:
            drawRelations();
            break;
            
        default:
            clearScr();
            break;
    }
}

void SQLite2::openDatabase() {
    sqlite3* database;
    if (sqlite3_open_v2(fileName.c_str(), &database, SQLITE_OPEN_READONLY, NULL) == SQLITE_OK)
    {
        readData(database);
        sqlite3_close(database);
        file_status = F_CURR;
    }
    else
    {
        
        file_status = F_NONE;
    }
}

void SQLite2::saveDatabase() {
    sqlite3* database;
    switch(file_status)
    {
        case F_NONE:
            //mvprintw(getmaxy(stdscr)-2, 0, "Error: Cannot save database.");
            break;
            
        case F_CURR:
            sqlite3_open_v2( fileName.c_str(), &database, SQLITE_OPEN_READWRITE, NULL);
            for (int i=0; i < removedTables.size(); i++)
                sqlite3_exec(database, getDropQuery(removedTables[i]).c_str(), NULL, NULL, NULL );
            for (int i=0; i < tables.size(); i++)
            {
                sqlite3_exec(database, getDropQuery(tables[i].getTableName()).c_str(), NULL, NULL, NULL);
                sqlite3_exec(database, tables[i].getSaveQuery().c_str(), NULL, NULL, NULL);
                if (tables[i].getRowCount() > 0)
                    sqlite3_exec(database, tables[i].getDataQuery().c_str(), NULL, NULL, NULL);
            }
            
            sqlite3_close(database);
            isSaved = true;
            break;
            
        case F_NEW:
            sqlite3_open_v2( fileName.c_str(), &database, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
            
            for (int i=0; i < tables.size(); i++)
            {
                sqlite3_exec(database, tables[i].getSaveQuery().c_str(), NULL, NULL, NULL);
                if (tables[i].getRowCount() > 0)
                    sqlite3_exec(database, tables[i].getDataQuery().c_str(), NULL, NULL, NULL);
            }
            
            sqlite3_close(database);
            isSaved = true;
            break;
    }
}

void SQLite2::readData(sqlite3* database) {
    // CLEAR
    tables.clear();
    relations.clear();
    removedTables.clear();
    
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
    
    // RELATIONS
    std::string toFind1 = "foreign KEY(";
    std::string buffName;
    
    std::string homeCol;
    std::string foreignTable;
    std::string foreignCol;
    
    int pointer_in;
    int pointer_out;
    for (int id=0; id < tables.size(); id++)
    {
        if ((pointer_in = tables[id].getSql().find(toFind1)) != std::string::npos)
        {
            pointer_in = pointer_in + 12;
            pointer_out = tables[id].getSql().find(")");
            homeCol.assign(tables[id].getSql().substr(pointer_in, pointer_out-pointer_in));
            
            buffName.assign(tables[id].getSql().substr(pointer_out));
            pointer_in = 13;
            pointer_out = buffName.find("(");
            foreignTable.assign(buffName.substr(pointer_in, pointer_out - pointer_in));
            
            buffName.assign(buffName.substr(pointer_out));
            pointer_in = 1;
            pointer_out = buffName.find(")");
            foreignCol.assign(buffName.substr(pointer_in, pointer_out-pointer_in));
            
            Table* foreignTablePtr;
            for (int i=0; i< tables.size(); i++)
                if (foreignTable == tables[i].getTableName())
                    foreignTablePtr = &tables[i];
            
            relations.push_back(tables[id].addRelation(homeCol, foreignTablePtr, foreignCol));
        }
    }
}

void SQLite2::clearScr() {
    for (int y=1; y<getmaxy(stdscr)-3; y++) {
        move( y, 0);
        clrtoeol();
    }
    clearErr();
}

void SQLite2::clearErr() {
    move(getmaxy(stdscr)-2,0);
    clrtoeol();
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
            if (tables[id].getData(y,x).length() == 0)
            {
                move(4+y,x*field_width);
                for (int i=0; i<field_width-1; i++)
                    printw(" ");
            }
            else
                mvprintw(4+y, x*field_width, "%s", tables[id].getData(y,x).c_str());
            if (y == selectedRow && x == selectedCol)
                attroff(A_STANDOUT);
        }
    }
    refresh();
}

void SQLite2::drawRelations() {
    selectedScreen = S_RELATIONS;
    clearScr();
    drawTitle("RELATIONSHIPS");
    for (int i=0; i < relations.size(); i++)
    {
        if (i == selectedRelation)
            attron(A_STANDOUT);
        mvprintw(2+i, 0, "%s( %s ) ~ ", relations[i]->homeTable->getTableName().c_str(), relations[i]->homeTable->getColName(relations[i]->homeCol).c_str());
        printw("%s( %s )", relations[i]->foreignTable->getTableName().c_str(), relations[i]->foreignTable->getColName(relations[i]->foreignCol).c_str());
        if (i == selectedRelation)
            attroff(A_STANDOUT);
    }
    refresh();
}

void SQLite2::remove() {
    switch(selectedScreen) {
        case S_TABLES:
            for (int i=0; i < relations.size(); i++)
                if (relations[i]->foreignTable == &tables[selectedTable])
                    relations.erase(relations.begin() + i);
            removedTables.push_back(tables[selectedTable].getTableName());
            tables.erase(tables.begin() + selectedTable);
            break;
            
        case S_FIELDS:
            tables[selectedTable].removeRow(selectedRow);
            break;
            
        case S_RELATIONS:
            relations[selectedRelation]->homeTable->removeRelation();
            relations.erase(relations.begin() + selectedRelation);
            break;
            
        default:
            return;
    }
    isSaved = false;
    redraw();
}

void SQLite2::add() {
    switch(selectedScreen) {
        case S_TABLES:
            // ADD TABLE
            break;
            
        case S_FIELDS:
            tables[selectedTable].addRow();
            selectedRow = tables[selectedTable].getRowCount()-1;
            selectedCol = 0;
            break;
            
        default:
            return;
    }
    isSaved = false;
    redraw();
}

void SQLite2::edit() {
    switch(selectedScreen) {
        case S_TABLES:
            tables[selectedTable].setTableName( editValue );
            break;
            
        case S_FIELDS:
            tables[selectedTable].editField(selectedRow, selectedCol, editValue );
            break;
            
        default:
            return;
    }
    isSaved = false;
    redraw();
}

void SQLite2::rightArrow() {
    switch(selectedScreen) {
            
        case S_FIELDS:
            selectedCol++;
            if (selectedCol >= tables[selectedTable].getColCount())
                selectedCol = tables[selectedTable].getColCount()-1;
            break;
            
        default:
            return;
    }
    redraw();
}

void SQLite2::leftArrow() {
    switch(selectedScreen) {
            
        case S_FIELDS:
            selectedCol--;
            if (selectedCol < 0)
                selectedCol = 0;
            break;
            
        default:
            return;
    }
    redraw();
}

void SQLite2::upArrow() {
    switch(selectedScreen) {
        case S_TABLES:
            selectedTable--;
            if (selectedTable < 0)
                selectedTable = 0;
            break;
            
        case S_FIELDS:
            selectedRow--;
            if (selectedRow < 0)
                selectedRow = 0;
            break;
            
        case S_RELATIONS:
            selectedRelation--;
            if (selectedRelation < 0)
                selectedRelation = 0;
            
        default:
            return;
    }
    redraw();
}

void SQLite2::downArrow() {
    switch(selectedScreen) {
        case S_TABLES:
            selectedTable++;
            if (selectedTable >= tables.size())
                selectedTable = (int)tables.size()-1;
            break;
            
        case S_FIELDS:
            selectedRow++;
            if (selectedRow >= tables[selectedTable].getRowCount())
                selectedRow = tables[selectedTable].getRowCount()-1;
            break;
            
        case S_RELATIONS:
            selectedRelation++;
            if (selectedRelation >= relations.size())
                selectedRelation = (int)relations.size()-1;
            
        default:
            return;
    }
    redraw();
}

std::string SQLite2::getDropQuery(std::string name) {
    std::string query = "DROP TABLE ";
    query.append(name);
    query.append(";");
    return query;
}
