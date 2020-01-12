//
//  sqlite2.hpp
//  SQLiteTool2
//
//  Created by celmer on 11/01/2020.
//  Copyright © 2020 celmer. All rights reserved.
//

#ifndef sqlite2_hpp
#define sqlite2_hpp

#include <sqlite3.h>
#include <ncurses.h>
#include <stdio.h>
#include <string>
#include <vector>

#include "shared.hpp"
#include "table.hpp"

#define field_width 16

#define S_TABLES 1
#define S_FIELDS 2
#define S_RELATIONS 3


class SQLite2 : public Tool {
private:
    std::string fileName;
    std::vector<Table> tables;
    
    std::string editValue;
    
    int selectedScreen;
    int selectedTable;
    int selectedRow;
    int selectedCol;
    int selectedRelation;
    
    void readData(sqlite3* database);
    void drawTitle(const char* titleName);
    void clearScr();
    
public:
    void setEntry(std::string field, std::string value);
    std::string getEntry(std::string field);
    void bindBinds();
    
    void init();
    void redraw();
    
    void openDatabase();
    
    void drawTables();
    void drawFields();
    
    void remove();
    void add();
    void edit();
    
    void rightArrow();
    void leftArrow();
    void upArrow();
    void downArrow();
};



#endif /* sqlite2_hpp */
