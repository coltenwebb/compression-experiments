#include <iostream>
#include <fstream>
#include <string>
#include <queue>

#define MAX_WINDOW 30000
#define MAX_LENGTH 255

struct Pointer {
  unsigned short pos;
  unsigned char length;
  char ch;
};

void readIntoDeque(std::ifstream &fin, std::deque<char> &deq) {
  char buf[1024];
  fin.read(buf, 1024);
  for (int i = 0; i < fin.gcount(); i++) {
    deq.push_back(buf[i]);
  }
}

int encode(std::string iname, std::string oname) { 
  // we will use short as point and window size
  std::ifstream fin(iname);
  std::ofstream fout(oname);
  std::deque<char> lookahead;
  std::deque<char> window;
  readIntoDeque(fin, lookahead);
  while (lookahead.size() > 0) {
    // look through the string for matches
    int cl = 0, bl = 0, bp = 0;
    for (int i = 0; i < window.size(); i++) {
      if (cl >= lookahead.size()) {
        readIntoDeque(fin, lookahead);
        if (cl >= lookahead.size()) break;
      }
      if (window[i] == lookahead[cl]) {
        cl++;
      } else {
        if (cl != 0)i--;
        cl = 0;
      }
      if (cl > bl) {
        bl = cl;
        bp = - ((i-cl) - window.size() + 1);
      }
    }
    //std::cout << window.length() << std::endl;
    if (bl > MAX_LENGTH) bl = MAX_LENGTH;
    if (lookahead.size() == bl) bl--;
    if (bl == 0) bp = 0;
    //std::cout << "(" << bp << "," << bl << ")" << (lookahead[bl]) << std::endl;
    struct Pointer p = {bp, bl, lookahead[bl]};
    fout.write((char *) &p, sizeof(p));
    for (int i = 0; i < bl+1; i++) {
      window.push_back(lookahead[0]);
      lookahead.pop_front();
    }
    while (window.size() > MAX_WINDOW) {
      window.pop_front();
    }
    if (lookahead.size() == 0) {
      readIntoDeque(fin, lookahead);
      if (lookahead.size() == 0) break;
    }
  }
  return 0;
}

int decode(std::string iname, std::string oname) { 
  return 0;
}

int main(const int nargs, const char** args) {
  for (int i = 1; i < nargs; i++) {
    std::string arg = args[i];
    if (arg == "-e") {
      return encode(args[i+1], args[i+2]);
    } else if (arg == "-d") {
      return decode(args[i+1], args[i+2]);
    }
  }
}
