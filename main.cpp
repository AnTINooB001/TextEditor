#include <ncurses.h>
#include <string>
#include <vector>
#include <fstream>

#define MENU_HEIGHT 2

#define CTRL_A 0x01
#define CTRL_B 0x02
#define CTRL_C 0x03
#define CTRL_D 0x04
#define CTRL_E 0x05
#define CTRL_F 0x06
#define CTRL_G 0x07
#define CTRL_H 0x08
#define CTRL_I 0x09
#define CTRL_J 0x0A
#define CTRL_K 0x0B
#define CTRL_L 0x0C
#define CTRL_M 0x0D
#define CTRL_N 0x0E
#define CTRL_O 0x0F
#define CTRL_P 0x10
#define CTRL_Q 0x11
#define CTRL_R 0x12
#define CTRL_S 0x13
#define CTRL_T 0x14
#define CTRL_U 0x15
#define CTRL_V 0x16
#define CTRL_W 0x17
#define CTRL_X 0x18
#define CTRL_Y 0x19
#define CTRL_Z 0x1A
#define KEY_ESC 0x1B
#define HOR_TAB CTRL_I
#define LINE_FEED CTRL_J

int width,height,sx,sy;
void moveLeft();
void moveRight();
void moveDown();
void moveUp();

class editor
{
private:
    std::vector<std::string> rows;
    std::string message;
    std::string displaying_name;
    std::string file_name;
public:
    int xoff,yoff;
    bool modif;
    editor(): xoff{0},yoff{0} { rows.push_back(""); modif = false; };

    void setFileName(std::string str) //
    {
        if(str.empty()) {
            displaying_name = "Unnamed";
            return;
        }
        file_name = str;

        auto it = str.end()-1;
        auto b = str.begin();

        while(it-- > b)
            if(*it == '/' || *it == '\\')
                break;
        it++;
        
        displaying_name.clear();
        displaying_name.append(&(*it));
    }

    void insertChar(char c)
    {
        int temp = getcurx(stdscr)+xoff;
        int y = getcury(stdscr)+yoff;
        
        auto it = rows[y].begin();
        while(temp--)
            it++;
        rows[y].insert(it,c);
    }

    void insertNewRow(std::string str)
    {
        int y = getcury(stdscr);
        int temp = y+yoff;
        auto it = rows.begin();
        while(temp--)
            it++;
        rows.insert(it,str);
    }

    void printMenu()
    {
        int txtsz = 0;
        move(height-MENU_HEIGHT,0);
        if(message.empty()) {
            printw("| CTRL+Q to exit |");
            txtsz += 18;

            if(modif) {
                printw(" Modified |");
                txtsz += 11;
            }
            else {
                printw(" Saved |");
                txtsz += 8;
            }
            int sz = width-txtsz-2;
            if(sz < displaying_name.size()-1) {
                for (size_t i = 0; i < sz; i++)
                {
                    addch(displaying_name[i]);
                }
                addch('|');
            }
            else
                printw(" %s |",displaying_name.c_str());
        }
        else{
            printw("%s",message.c_str());
        }
    }

    void addMessage(std::string str)
    {
        message.append(str);
    }

    void eraseMessage()
    {
        message.clear();
    }

    void printRows()
    {
        sx = getcurx(stdscr);
        sy = getcury(stdscr);
        int ty = 0;
        clear();
        move(0,0);
        for(int y = 0; y < height-MENU_HEIGHT; y++) {
            if(y+yoff < rows.size()) {
                int sz = rows[y+yoff].size();
                if(sz-xoff > 0) {
                    insnstr(&rows[y+yoff].c_str()[xoff],rows[y+yoff].size()-xoff);
                }
            }
            else
                addch('~');
            ty++;
            move(ty,0);
        }
        printMenu();
        move(sy,sx);
        refresh();
    }

    void delChar()
    {
        int temp = getcurx(stdscr)+xoff;
        int y = getcury(stdscr)+yoff;
        
        auto it = rows[y].begin();
        while(temp--)
            it++;
        if(it < rows[y].end())
            rows[y].erase(it);

    }

    void delRow()               //delete row (erase row below)
    {
        sx = getcurx(stdscr);
        sy = getcury(stdscr);

        moveDown();
        int y = getcury(stdscr)+yoff;
        
        auto it = rows.begin();
        while(y--)
            it++;

         auto prev_it = it-1;
         if(it < rows.end())
             prev_it->append(it->c_str());

        if(it < rows.end())
            rows.erase(it);

        move(sy,sx);
        
    }

    int rowsCount() { return rows.size(); }
    int strLen() 
    { 
        int y = getcury(stdscr)+yoff;
        return (y >= rows.size())? 0 :rows[y].size(); 
    }

    void writeInFile()
    {
        if(file_name.empty()) {
            std::string buff;
            chtype c;
            addMessage("ECS to cancel | Enter name of the file: ");
            printRows();
            while(c = getch()) {
                if(c == KEY_ESC)
                    break;
                if(c == '\n' && !buff.empty())
                    break;
                if(c == '\n')
                    continue;
                else if(isdigit(c) || isalpha(c) || c == '.')
                    buff.push_back(c);
                addMessage(buff);
                printRows();
                eraseMessage();
                addMessage("ECS to cancel | Enter name of the file: ");
            }
            setFileName(buff);
            eraseMessage();
            printRows();
        }
        std::ofstream file(file_name);
        if(!file.is_open())
            return;
        auto e = rows.end();

        for (auto it = rows.begin(); it < e; it++)
        {
            if(it == e-1)
                file << *it;
            else
                file << *it + "\n";
        }
    
        file.close();
        modif = false;
    }
};
editor E;

void goToRowEnd()
{
    int pos_x = getcurx(stdscr);

    if(pos_x+E.xoff > E.strLen()) {
        while(pos_x+E.xoff != E.strLen()) {
            moveLeft();
            pos_x = getcurx(stdscr);
        }
    }
    else {
        while(pos_x+E.xoff != E.strLen()) {
            moveRight();
            pos_x = getcurx(stdscr);
        }
    }
}

void goToRowStart()
{
    int pos_x = getcurx(stdscr);
    while(pos_x+E.xoff != 0) {
        moveLeft();
        pos_x = getcurx(stdscr);
    }
}

void goToFirstRow()
{
    int pos_y = getcury(stdscr);

    while(pos_y+E.yoff > 0) {
        moveUp();
        pos_y = getcury(stdscr);
    }
}

void goToLastRow()
{
    int pos_y = getcury(stdscr);

    while(pos_y+E.yoff < E.rowsCount()) {
        moveDown();
        pos_y = getcury(stdscr);
    }
}

chtype getKey()
{
    chtype c;
    while((c = getch()) == ERR) {};
    return c;
}

void moveLeft()
{
    int pos_x = getcurx(stdscr);
    int pos_y = getcury(stdscr);

    if(pos_x+E.xoff == 0) {                        //if should go throught left edge, its go to end of prev row
        if(pos_y+E.yoff == 0)  //leave if its first line
            return;
        moveUp();
        goToRowEnd();
        return;
    }

    if(pos_x > 0) {
        move(pos_y,pos_x-1);
    }
    else {
        if(E.xoff > 0) 
            E.xoff--;
    }
}

void moveRight()
{
    int pos_x = getcurx(stdscr);
    int pos_y = getcury(stdscr);

    if(pos_y+E.yoff == E.rowsCount()) {     //check if row not init '~'
        move(pos_y,0);
    }
    else if(pos_x+E.xoff == E.strLen()) {   //if should go throught size of row, its go to start of next row
        moveDown();
        pos_x = getcurx(stdscr);
        goToRowStart();
    }
    else if(pos_x+E.xoff < E.strLen()) {    // move cursor or add offset
        if(pos_x == width-1) 
            E.xoff++;
        else 
            move(pos_y,pos_x+1);
    }
}

void moveDown()
{
    int pos_x = getcurx(stdscr);
    int pos_y = getcury(stdscr);

    if(pos_y+E.yoff < E.rowsCount()) {
        if(pos_y < height-1-MENU_HEIGHT) 
            move(pos_y+1,pos_x);
        else
            E.yoff++;
    }
    int len = E.strLen();
    if(pos_y+E.yoff >= E.rowsCount()) {
        pos_y = getcury(stdscr);
        move(pos_y,0);
    }
    else if(pos_x+E.xoff > E.strLen())
        goToRowEnd();
}

void moveUp()
{
    int pos_x = getcurx(stdscr);
    int pos_y = getcury(stdscr);

    if(pos_y > 0) {
        move(pos_y-1,pos_x);
    }
    else {
        if(E.yoff > 0) {
            E.yoff--;
        }
    }

    if(pos_x+E.xoff > E.strLen())
        goToRowEnd();
}

void insertRowBack(std::string str)
{
    int pos_x = getcurx(stdscr);
    int pos_y = getcury(stdscr);

    moveDown();
    E.insertNewRow(str);

    pos_y = getcury(stdscr);
    move(pos_y,0);
    E.xoff = 0;
}

void insertRowFront(std::string str)
{
    int pos_x = getcurx(stdscr);
    int pos_y = getcury(stdscr);

    E.insertNewRow(str);
    E.delRow();
    moveDown();
    pos_y = getcury(stdscr);
    move(pos_y,0);
    E.xoff = 0;
}

void addChar(char c)
{
    int pos_x = getcurx(stdscr);
    int pos_y = getcury(stdscr);

    if(pos_y+E.yoff == E.rowsCount()) {     // if cursor located on not init row '~'
        insertRowBack("");
    }

    if(pos_x >= width-1) {                  // if string longer then screen
        E.insertChar((char)c);
        E.xoff++;
    } 

    else {                                  // just add symbol
        E.insertChar((char)c);              
        moveRight();
    }
}

void endProg(const int a)
{
    echo();
    exit(a);
}

void openFile(std::string name)
{
    E.setFileName(name);

    std::ifstream file(name);
    if(!file.is_open())
        return;
    std::string buff;
    while(1) {
        std::getline(file,buff);
        insertRowFront(buff);
        if(file.eof())
            break;
    }
    file.close();

    goToRowStart();
    goToFirstRow();
}

void keyProcess()
{
    chtype c = getKey();
    int pos_x = getcurx(stdscr);
    int pos_y = getcury(stdscr);
    int count;
    switch (c) {
        case CTRL_Q:
            if(E.modif == false) {
                endProg(0);
            }
            else {
                E.addMessage("All changes not will be saved! You are sure? y or n ");
                E.printRows();
                while(1) {
                    c = getKey();
                    if(tolower(c) == 'y')
                        endProg(0);
                    if(tolower(c) == 'n') {
                        E.eraseMessage();
                        E.printRows();
                        return;
                    }
                }
            }
            break;
        
        case CTRL_S:             // here should be CTRL+S
            E.writeInFile();
            break;

        case CTRL_A: // not realesed
        case CTRL_B:
        case CTRL_C:
        case CTRL_D:
        case CTRL_E:
        case CTRL_F:
        case CTRL_G:
        case CTRL_H:
        case CTRL_K:
        case CTRL_L:
        case CTRL_M:
        case CTRL_N:
        case CTRL_O:
        case CTRL_P:
        case CTRL_R:
        case CTRL_T:
        case CTRL_U:
        case CTRL_V:
        case CTRL_W:
        case CTRL_X:
        case CTRL_Y:
        case CTRL_Z:
            break;

        case HOR_TAB:
            E.modif = true;
            count = TABSIZE;
            while(count--)
                addChar(' ');
            break;

        case KEY_LEFT:
            moveLeft();
            break;

        case KEY_RIGHT:
            moveRight();
            break;

        case KEY_DOWN:
            moveDown();
            break;

        case KEY_UP:
            moveUp();
            break;

        case KEY_NPAGE:
            count = height-MENU_HEIGHT;      
            while(pos_y != count) {         //move to down edge of screen
                moveDown();
                pos_y = getcury(stdscr);
            }
            while(count--)                  //move on 1 screen Width below
                moveDown();
            break;

        case KEY_PPAGE:
            count = height-MENU_HEIGHT;     
            while(pos_y != 0) {         //move to up edge of screen
                moveUp();
                pos_y = getcury(stdscr);
            }
            while(count--)                  //move on 1 screen Width upper
                moveUp();
            break;
        
        case KEY_HOME:
            goToRowStart();
            break;

        case KEY_END:
            goToRowEnd();
            break;

        case KEY_IL:
        case LINE_FEED:
        case KEY_ENTER:
            E.modif = true;
            insertRowBack("");
            break;

        case KEY_DC: 
            E.modif = true;
            if(pos_x+E.xoff == E.strLen())
                E.delRow();
            else
                E.delChar();
            break;

            case KEY_DL:
                E.delRow();
            break;

        case KEY_BACKSPACE:  
            E.modif = true;                   
            if(pos_x+E.xoff == 0 && pos_y+E.yoff == 0)
                break;
            moveLeft();
            pos_x = getcurx(stdscr);
            if(pos_x+E.xoff == E.strLen())
                E.delRow();
            else
                E.delChar();
            break;

        default:
            E.modif = true;
            addChar(c);
            break;
    }
}

int main(int argc, char* argv[]) {
    initscr();

    getmaxyx(stdscr,height,width);

    if(argv[1] != nullptr)
        openFile(argv[1]);
    else
        openFile("");
    

    halfdelay(0);
    move(0,0);
    raw();
    noecho();
    keypad(stdscr,TRUE);

    while (1) {
        E.printRows();  
        keyProcess();
    }
    clear();
    endwin();
}