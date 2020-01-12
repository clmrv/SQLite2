//
//  main.cpp
//  SQLiteTool2
//
//  Created by celmer on 11/01/2020.
//  Copyright © 2020 celmer. All rights reserved.
//

#include "sqlite2.hpp"
#include <unistd.h>

int main(int argc, const char * argv[]) {
    
    initscr();
    mvprintw(0,0, "test menu");
    mvprintw(getmaxy(stdscr)-1, 0, "test menu");
    refresh();
    
    SQLite2 tool;
    tool.init();
    tool.openDatabase();
    tool.drawTables();
    tool.drawFields();
    
    getch();
    endwin();
    return 0;
}
