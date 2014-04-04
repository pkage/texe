#include <ncurses.h>
#include <vector>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cstring>

using namespace std;

void xmvaddstr(int y, int x, string str);
string xscanw(int y, int x);
string itos(int i);
string btos(bool b);
int _stoi(string s);
bool isnumber(string s);
string xpscanw(int y, int x,string s);
void clearline(int y);
bool editor(string s, bool n);

const string NUMS = "0123456789";

class File {
private:
	vector<string> contents;
	int cpos;
	string path;
	bool newfile,initialized,saved;
public:
	bool init(string path) {
		this->path = path;
		this->initialized = true;
		ifstream ifile;
		ifile.open(path.c_str());
		if (!ifile) {
			newfile = true;
			return true;
		}
		contents.clear();
		string tmp;
		while (!ifile.eof()) {
			getline(ifile,tmp);
			contents.push_back(tmp);
		}
		newfile = false;
		return true;
	}
	File() {
		initialized = false;
		saved = false;
	}
	File(string path) {
		initialized = false;
		saved = false;
		init(path);
	}
	int getSize() {return contents.size();}
	string getLine(int ln) {
		if (ln > 0 && ln < contents.size()) {
			return contents.at(ln);
		}
		return "";
	}
	string getViewPort(int &line) {
		if (line < 0) {line = 0;}
		if (contents.size() == 0) {contents.push_back("");}
		string tmp;
		for (int c = line; (c < line + (LINES - 1)) && (c < contents.size()); c++) {
			tmp += itos(c + 1) + "\t| " + contents.at(c) + "\n";
		}
		return tmp;
	}
	void insertString(int ln, string line) {
		vector<string> tmp;
		for (int c = 0; c < contents.size(); c++) {
			if (c == ln) {
				tmp.push_back(line);
			}
			tmp.push_back(contents.at(c));
		}
		if (ln >= contents.size()) {
			tmp.push_back(line);
		}
		for (int c = 0; c < tmp.size(); c++) {
			if (c < contents.size()) {
				contents.at(c) = tmp.at(c);
			} else {
				contents.push_back(tmp.at(c));
			}
		}
		saved = false;
	}
	void removeLine(int ln) {
		if (ln < 0 || ln >= contents.size()) {
			return;
		}
		vector<string> tmp;
		for (int c = 0; c < contents.size(); c++) {
			if (c != ln) {
				tmp.push_back(contents.at(c));
			}
		}
		for (int c = 0; c < tmp.size(); c++) {
			if (c < contents.size()) {
				contents.at(c) = tmp.at(c);
			}
		}
		contents.pop_back();
		saved = false;
	}
	void replaceLine(int ln,string r) {
		if (ln < 0 || ln >= contents.size()) {
			return;
		}
		vector<string> tmp;
		for (int c = 0; c < contents.size(); c++) {
			if (c == ln) {
				tmp.push_back(r);
			} else {
				tmp.push_back(contents.at(c));
			}
		}
		for (int c = 0; c < tmp.size(); c++) {
			if (c < contents.size()) {
				contents.at(c) = tmp.at(c);
			}
		}
		saved = false;
	}
	void save(char c) {
		if (newfile || c == 'S') {
			xmvaddstr(LINES-1, 0, "File name: ");
			path = xscanw(LINES - 1, 13);
		}
		ofstream ofile;
		ofile.open(path.c_str());
		if (!ofile) {
			clearline(LINES - 1);
			xmvaddstr(LINES - 1,0, "unable to save file");
			noecho();getch();echo();
			return;
		}
		for (int c = 0; c < contents.size(); c++) {
			if (c != contents.size()-1) {ofile << contents.at(c) << '\n';} else {
				ofile << contents.at(c);
			}
		}
		ofile.close();
		clearline(LINES - 1);
		xmvaddstr(LINES - 1, 0, "saved file successfully as \"" + path + "\"\t(press any key to continue)");
		noecho();getch();echo();
		saved = true;
	}
	bool isSaved() {return saved;}
};

int main(int argc, char* argv[]) {
	initscr(); cbreak(); noecho(); keypad(stdscr,true);
	clear();
	if (argc == 1) {
		editor("", true);
	} else {
		editor(argv[1], false);
	}
	endwin();
	return 0;
}
bool editor(string fpath, bool n) {
	clear();
	File file;
	if (!n) {
		if (!file.init(fpath)) {
			xmvaddstr(0,0,"failed to open " + fpath + "\t<press any key to continue>");getch();
		}
	}
	char c; int line = 0, tmp2, p_line = 0;string tmp;
	do {
		clear();
		xmvaddstr(0,0,file.getViewPort(line));
		//xmvaddstr(0,10,itos(file.getSize()));
		refresh();
		curs_set(0);
		c = getch();
		curs_set(1);
		if (c == 'k') {line--;}
		if (c == 'K') {line -= 5;}
		if (c == 'j') {line++;}
		if (c == 'J') {line += 5;}
		if (c == 'i') {
			xmvaddstr(LINES - 1,0,":");
			tmp = xscanw(LINES - 1,1);
			if (isnumber(tmp)) {
				xmvaddstr(LINES - 1, tmp.length() + 1, ":");
				tmp2 = _stoi(tmp);
				p_line = tmp2;
				tmp = xscanw(LINES - 1, tmp.length() + 2);
				file.insertString(tmp2, tmp);
			}
			clear();
		}
		if (c == 'r') {
			xmvaddstr(LINES - 1,0,":");
			tmp = xscanw(LINES -1,1);
			file.removeLine(_stoi(tmp) - 1);
		}
		if (c == 'w') {
			xmvaddstr(LINES - 1,0,":");
			tmp = xscanw(LINES -1,1);
			file.replaceLine(_stoi(tmp) - 1,"");
		}
		/*if (c == 'e') {
			xmvaddstr(LINES - 1,0,":");
			tmp = xscanw(LINES -1,1);
			tmp2 = _stoi(tmp);
			xmvaddstr(LINES - 1,tmp.length() + 1,":");
			tmp = xpscanw(LINES - 1, tmp.length() + 2,file.getLine(tmp2));
			file.replaceLine(_stoi(tmp) - 1,"");
		}*/
		if ((int)c == 10) {
			p_line++;
			xmvaddstr(LINES - 1, 0, ":" + itos(p_line) + ":");
			file.insertString(p_line, xscanw(LINES - 1,(itos(p_line).length() + 2)));
		}
		if (c == 's' || c == 'S') {
			file.save(c);
		}
		if (c == 'q') {
			if (!file.isSaved()) {
				xmvaddstr(LINES - 1, 0, "file not saved, use SHIFT-Q to quit anyways.\t<press any key to continue>");
				getch();
			} else {
				break;
			}
		}
	} while (c != 'Q'); // too lazy to fix
	return true;
}

void xmvaddstr(int y, int x, string str) {
	mvaddstr(y,x,str.c_str());
}

string itos(int i) {
	stringstream ss; ss << i;
	return ss.str();
}

string btos(bool b) {
	if (b) {
		return "true";
	} else {
		return "false";
	}
}
string xscanw(int y, int x) {
	string s;
	echo();
	char str[1000];
	mvgetstr(y,x,str);
	s = str;
	noecho();
	return s;
}
string xpscanw(int y, int x,string s) {
	echo();
	char str[1000]; strcpy(str, s.c_str());
	for (int c = 0; c < 1000; c++) {
		if (str[c] != '\0') {
			insch(str[c]);
		} else {
			break;
		}
	}
	mvgetstr(y,x,str);
	s = str;
	noecho();
	return s;
}
int _stoi(string s) {
	return atoi(s.c_str());
}

bool isnumber(string s) {
	for (int c = 0; c < s.length(); c++) {
		if (NUMS.find(s.at(c)) == -1) {
			return false;
		}
	}
	return true;
}
void clearline(int y) {
	string s;
	for (int c = 0; c < COLS; c++) {
		s += " ";
	}
	xmvaddstr(y,0,s);
}
