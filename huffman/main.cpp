#include <fstream>
#include <map>
#include <vector>
#include <iostream>
#include <queue>
#include <bitset>

int maxdepth = 0;
class Node {
public:
  int freq;
  char c;
  Node *l, *r;
  Node() {
    freq = 0;
    c = 0;
  }
  void encodeTree(std::ofstream &fout);
  void decodeTree(std::ifstream &fin, int numsymbols); //read from file using reader
};

bool comp (const Node *c1, const Node *c2) {
    return c1->freq > c2->freq;
}

void gencodes(std::map<char,std::vector<bool> > &codes, std::vector<bool> curcode, const Node &currn) {
  if (currn.c == 0) {
    curcode.push_back(1);
    gencodes(codes, curcode, *currn.l);
    curcode.pop_back();
    curcode.push_back(0);
    gencodes(codes, curcode, *currn.r);
  } else {
    codes[currn.c] = curcode;
    maxdepth = maxdepth < curcode.size() ? curcode.size() : maxdepth;
  }
}


std::vector<bool> wbbuff;
void flushWriteBit(std::ofstream &fout, bool force) {
  if (wbbuff.size() == 8 || force) {
    char ctp = 0;
    for (int i = 0; i < 8; i++) {
      if (wbbuff[i]) ctp |= 1 << i;
    }
    fout << ctp;
    std::bitset<8> bs(ctp);
    std::cout << "[" << bs << "]" << std::endl;
    wbbuff.clear();
  }
}

void writeBit(std::ofstream &fout, bool bit) {
  wbbuff.push_back(bit);
  flushWriteBit(fout, false);
}

void writeChar(std::ofstream &fout, char c) {
  for (int i = 0; i < 8; i++) {
    writeBit(fout, c&1);
    c>>=1;
  }
}
void writeInt(std::ofstream &fout, int num) {
  fout.write(reinterpret_cast<const char *>(&num), sizeof(num));
}

void clearWriteBit() {
  wbbuff.clear();
}

std::vector<bool> rbbuff;
bool readBit(std::ifstream &fin) {
  //std::cout << std::endl;
  if (rbbuff.size() == 0) {
    char c;
    fin.read(reinterpret_cast<char *>(&c), sizeof(c));
    std::bitset<8> bs(c);
    //std::cout << "[" << bs << "]" << std::endl;
    for (int i = 0; i < 8; i++) {
      rbbuff.push_back(c&(1<<(7-i)));
    }
  }
  bool ret = rbbuff.back();
  rbbuff.pop_back();
  return ret;
}

char readChar(std::ifstream &fin) {
  char ret = 0;
  //std::cout<<"rc:";
  for (int i = 0; i < 8; i++) {
    bool b = readBit(fin);
    //std::cout<<b;
    ret |= b ? (1<<(i)) : 0;
  }
  //std::cout<<":\n";
  return ret;
}

int readInt(std::ifstream &fin) {
  int a;
  fin.read(reinterpret_cast<char *>(&a), sizeof(a));
  return a;
}

int nextPowOf2(int n) {
  int ret = 1;
  while (n > ret) ret *= 2;
  return ret;
}

int posFromVecBool(std::vector<bool> vb) {
  int ret = 0;
  for (int i = 0; i < vb.size(); i++) {
    if (vb[i]) ret |= 1 << (vb.size()-i);
  }
  return ret;
}

void Node::encodeTree(std::ofstream &fout) {
  std::vector<Node*> s;
  s.push_back(this);
  while (s.size() != 0) {
    Node *n = s.back();
    s.pop_back();
    if (n->c != 0) {
      writeBit(fout, 1);
      writeChar(fout, n->c);
    } else {
      writeBit(fout, 0);
      s.push_back(n->l);
      s.push_back(n->r);
    }
  }
}

void Node::decodeTree(std::ifstream &fin, int numsymbols) {
  std::vector<Node*> s;
  s.push_back(this);
  while (numsymbols) {
    bool nb = readBit(fin);
    //std::cout << nb << std::endl;
    if (nb) {
      // we got a one!
      Node *n = s.back();
      n->c = readChar(fin);
      s.pop_back();
      numsymbols--;
      // now we go to right sibling
      if (0 == numsymbols) continue;
      n = s.back();
      s.pop_back();
      Node *child = new Node;
      n->r = child;
      s.push_back(child);
      //std::cout << "test" << std::endl;
    } else {
      Node *n = s.back();
      Node *child = new Node;
      n->l = child;
      s.push_back(child);
    }
  }
}

void decode() {
  std::ifstream fin("encout");
  int numchars, numsymbols;
  numchars = readInt(fin);
  numsymbols = readInt(fin);
  //std::cout << fin.read() << std::endl;
  //std::cout << numchars << " " << numsymbols << std::endl;
  Node *parent = new Node;
  parent->decodeTree(fin, numsymbols);
  //std::cout << "decoded tree!" << std::endl;
  //std::cout << (int) parent->r->r->r->c << std::endl;
  Node *curr = parent;
  while (numchars) {
    bool nb = readBit(fin);
  //  std::cout << "runbefseg" << nb;
    if (nb) {
      curr = curr->r;
    } else {
      curr = curr->l;
    }
    if (curr->c) {
      //std::cout << "reset";
      std::cout << curr->c;
      curr = parent;
      numchars--;
    }
  }
}

int main(int argc, char *argv[]) {
  //decode();return 0;
  
  // first we have to figure out how many times each character occurs
  std::map<char, int> occ;
  std::ifstream fin(argv[1]);
  char buff;
  while (fin >> std::noskipws >> buff) {
    occ[buff]++;
  }

  // next we can build the huffman encoding tree
  std::priority_queue<Node*, std::vector<Node*>, decltype(&comp)> pq(&comp);
  for (std::map<char,int>::iterator it=occ.begin(); it!=occ.end(); ++it) {
    Node *n = new Node;
    n->c = it->first;
    n->freq = it->second;
    pq.push(n);
  }

  while (pq.size() > 1) {
    Node *n1 = pq.top(); pq.pop();
    Node *n2 = pq.top(); pq.pop();
    Node *n = new Node;
    n->freq = n1->freq + n2->freq;
    n->l = n1;
    n->r = n2;
    pq.push(n);
  }

  std::map<char, std::vector<bool> > codes;
  std::vector<bool> start;
  gencodes(codes, start, *pq.top());
  for (std::map<char,std::vector<bool> >::iterator it=codes.begin(); it!=codes.end(); ++it) {
    //std::cout << (int) (it->first) << " => ";
    for (int i = 0; i < it->second.size(); i++) {
      //std::cout << ((it->second[i]) ? 1 : 0);
    }
    //std::cout << '\n';
  }

  // now we've figured out the encoding.
  // first we've got to output the tree we built
  std::ofstream fout(argv[2]);
  writeInt(fout, pq.top()->freq);
  writeInt(fout, codes.size());

  // uniquely encodes the tree since it must be full
  pq.top()->encodeTree(fout);
  //std::cout << "finished prefix" << std::endl;

  
  fin.clear(); // reset the file reader
  fin.seekg(0);
  while (-1 != (buff = fin.get())) {
    //std::cout << (int)buff << std::endl;
    std::vector<bool> code = codes[buff];
    for (int i = 0; i < code.size(); i++) {
      //std::cout << (code[i] ? "1" : "0");
      writeBit(fout, code[i]);
    }
  }
  flushWriteBit(fout, true);

  fin.close();
  fout.close();
}
