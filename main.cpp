#include <ctype.h>
#include <stdio.h>
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#define log(...) printf(__VA_ARGS__);
#define dbg() printf("HERE: %d\n", __LINE__);

void write_max_reg_size(size_t num){
  std::ofstream out("config.toml");

  out << "[vm]" << std::endl;
  out << "max_reg_size = " << std::to_string(num) << std::endl;
}

enum TokenEnum {
  TOKEN_IDEN = 0,  // Identifier
  TOKEN_VAR = 1,
  TOKEN_NUM = 2,
  TOKEN_INST = 3,  // Instruction (as in like macro)
  TOKEN_ASSN = 4,  // Assign
  TOKEN_LP = 5,    // Left Parenthesis
  TOKEN_RP = 6,    // Right Parenthesis
  TOKEN_EOF = -1
};

class Token {
 public:
  std::string value;
  TokenEnum type;

  Token(std::string value, TokenEnum type)
      : value(value), type(type) {}
};

class Scanner {
 public:
  std::string content;
  int cursor;
  char curr;
  std::vector<Token> tokens;
  Scanner(std::string content) {
    this->content = content;
    this->cursor = 0;
    this->curr = this->content[this->cursor];
  }

  void next() {
    cursor++;
    if (cursor >= content.length()) {
      curr = '\0';
      return;
    }
    curr = content[cursor];
  }
  void scan() {
    while (curr != '\0') {
      if (isalpha(curr)) {
        std::string tmp = "";
        while (isalnum(curr)) {
          tmp += curr;
          next();
        }
        if (tmp == "var") {
          tokens.push_back(Token(tmp, TOKEN_VAR));
        }
        next();  // Skipping the space
        tmp = "";
        while (isalpha(curr)) {
          tmp += curr;
          next();
        }
        tokens.push_back(Token(tmp, TOKEN_IDEN));
        next();                // Skipping to the next character
        while (curr == ' ') {  // Checking if there is another space or not
          next();
        }
        if (curr == '=') {
          tokens.push_back(Token("=", TOKEN_ASSN));
        } else {
          log("Error while scanning\n");
          break;
        }
        next();                // Skip space
        while (curr == ' ') {  // Checking if there is another space or not
          next();
        }
        // TODO: add more types in the future if needed, for now only integers are scanned
        tmp = "";
        while (isdigit(curr)) {
          tmp += curr;
          next();
        };
        tokens.push_back(Token(tmp, TOKEN_NUM));

      }

      else if (curr == '#') {
        next();  // Skipping the '#'
        std::string tmp = "";
        while (curr != '(') {
          tmp += curr;
          next();
        }
        tokens.push_back(Token(tmp, TOKEN_INST));

        if (curr == '(') {
          tokens.push_back(Token("(", TOKEN_LP));
        } else {
          log("Error while scanning\n");
          break;
        }

        next();  // Skipping the '('

        tmp = "";
        while (curr != ')') {
          tmp += curr;
          next();
        }
        tokens.push_back(Token(tmp, TOKEN_IDEN));

        if (curr == ')') {
          tokens.push_back(Token(")", TOKEN_RP));
        } else {
          log("Error while scanning\n");
          break;
        }
      }
      next();
    }
  }

  auto getTokens() {
    return this->tokens;
  }
};

class AST {
 public:
  std::string node_name;
  std::string name;
  std::string value;
  virtual std::string codeGen(){};
  AST(std::string node_name, std::string name, std::string value) {
    this->node_name = node_name;
    this->name = name;
    this->value = value;
  }
  virtual ~AST() {}
};
class ASTVar : public AST {
 public:
  int var_n;
  ASTVar(std::string name, std::string value, int var_n)
      : AST("ASTVar", name, value), var_n(var_n) {}
  std::string codeGen() {
    return "mov " + this->value + ", r" + std::to_string(var_n);
    AST::codeGen();
  }
};
class ASTIns : public AST {
 public:
  ASTIns(std::string name, std::string value)
      : AST("ASTIns", name, value) {}
  std::string codeGen() {
    return name + " r" + value;
    AST::codeGen();
  }
};

typedef enum { TYPE_VAR,
               TYPE_INS } ASTsType;

class NodeType {
 public:
  std::variant<ASTIns, ASTVar> node;
  ASTsType type;
  NodeType(std::variant<ASTIns, ASTVar> n, ASTsType t)
      : node(n), type(t) {}
};

class Parser {
 public:
  int cursor;
  Token curr = Token("", TOKEN_EOF);
  std::vector<Token> tokens;
  int vars_size = 0;

  std::unordered_map<std::string, int> vars_registers;

  std::vector<NodeType> nodes;
  Parser(std::vector<Token> tokens) {
    this->tokens = tokens;
    this->cursor = 0;
    this->curr = tokens[cursor];
  }
  void next() {
    cursor++;
    if (cursor >= tokens.size()) {
      curr = Token("Die", TOKEN_EOF);
      return;
    }
    curr = tokens[cursor];
  }
  void parse() {
    while (curr.type != TOKEN_EOF) {
      if (curr.type == TOKEN_VAR) {
        next();
        assert(((void)"Token is Identifier", curr.type == TOKEN_IDEN));
        std::string var_name = curr.value;
        next();
        assert(((void)"Token is Assign", curr.type == TOKEN_ASSN));
        next();
        assert(((void)"Token is Number", curr.type == TOKEN_NUM));
        std::string var_val = curr.value;
        ASTVar var = ASTVar(var_name, var_val, vars_size);
        nodes.push_back(NodeType(var, TYPE_VAR));
        vars_registers[var_name] = vars_size;
        vars_size++;
        continue;
      } else if (curr.type == TOKEN_INST) {
        std::string inst_name = curr.value;
        next();
        assert(((void)"Token is Left Parenthesis", curr.type == TOKEN_LP));
        next();
        assert(((void)"Token is Identifier", curr.type == TOKEN_IDEN));
        std::string inst_val = curr.value;
        next();
        assert(((void)"Token is Right Parenthesis", curr.type == TOKEN_RP));

        ASTIns var = ASTIns(inst_name, std::to_string(vars_registers[inst_val]));
        nodes.push_back(NodeType(var, TYPE_INS));
        continue;
      }
      next();
    }
    write_max_reg_size(vars_size);
  }
  auto getTree() {
    return nodes;
  }
};

int main(int argc, char** argv) {
  if (argc < 2) {
    log("No file name was inputted\n")
    return 1;
  }
  std::string filename = argv[1];
  std::ifstream file(filename);
  if (!file) {
    log("File does not seem to open up, make sure it is valid");
    return 1;
  }

  Scanner scanner(std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()));

  scanner.scan();

  // for (auto token : scanner.getTokens()) {
  //   log("%s -> %d\n", token.value.c_str(), token.type);
  // }

  Parser parser(scanner.getTokens());

  parser.parse();

  std::string out_filename = "";
  size_t index = filename.find_last_of(".");
  if (index == std::string::npos) {
    out_filename = filename + ".pvm";
  } else {
    out_filename = filename.substr(0, index) + ".pvm";
  }
  std::ofstream out(out_filename);

  auto tree = parser.getTree();
  for (int i = 0; i < tree.size(); i++) {
    auto node = tree[i];
    if (node.type == TYPE_VAR) {
      // std::cout << std::get<ASTVar>(node.node).codeGen() << std::endl;
      out << std::get<ASTVar>(node.node).codeGen() << std::endl;
    } else if (node.type == TYPE_INS) {
      // std::cout << std::get<ASTIns>(node.node).codeGen() << std::endl;
      out << std::get<ASTIns>(node.node).codeGen() << std::endl;
    } else {
    }
  }
  system(("python pyvm.py "+out_filename).c_str());
  return 0;
}
