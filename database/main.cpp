#include "exerciser.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <pqxx/pqxx>
#include <string>
#define STATE "INSERT INTO \"STATE\"(\"STATE_ID\", \"NAME\")"
#define COLOR "INSERT INTO \"COLOR\"(\"COLOR_ID\",\"NAME\")"
#define TEAM                                                                   \
  "INSERT INTO "                                                               \
  "\"TEAM\"(\"TEAM_ID\",\"NAME\",\"STATE_ID\",\"COLOR_ID\",\"WINS\","          \
  "\"LOSSES\")"
#define PLAYER                                                                 \
  "INSERT INTO "                                                               \
  "\"PLAYER\"(\"PLAYER_ID\",\"TEAM_ID\",\"UNIFORM_NUM\",\"FIRST_NAME\","       \
  "\"LAST_"                                                                    \
  "NAME\",\"MPG\",\"PPG\",\"RPG\",\"APG\",\"SPG\",\"BPG\")"
using namespace pqxx;
class File {
private:
  ifstream ifs;

public:
  File(string filename) { ifs.open(filename, std::ifstream::in); }
  ~File() { ifs.close(); }
  std::string getNextLine() {
    std::string str;
    getline(ifs, str);
    if (str.empty())
      return {};
    return str;
  }
};
std::string trimLeadingSpace(std::string &msg) {
  if (msg.empty())
    return "";
  size_t target = msg.find_first_not_of(' ');
  if (target == std::string::npos)
    return msg;
  return msg.substr(target);
}
std::string fetchNextSeg(std::string &msg, char c = ' ', size_t substrlen = 1) {
  msg = trimLeadingSpace(msg);
  size_t target = msg.find(c);
  std::string res = msg.substr(0, target);
  if (target != std::string::npos)
    msg = msg.substr(target + substrlen);
  else
    msg = "";

  return res;
}
int DATA_initializer(connection *C, const std::string &target,
                     const std::string filename);
int TABLE_initializer(connection *C);
int main(int argc, char *argv[]) {

  // Allocate & initialize a Postgres connection object
  std::unique_ptr<connection> C;

  try {
    // Establish a connection to the database
    // Parameters: database name, user name, user password
    C = std::unique_ptr<connection>(
        new connection("dbname=ACC_BBALL user=postgres password=passw0rd"));
    if (C->is_open()) {
      cout << "Opened database successfully: " << C->dbname() << endl;
    } else {
      cout << "Can't open database" << endl;
      return 1;
    }
  } catch (const std::exception &e) {
    cerr << e.what() << std::endl;
    return 1;
  }

  // TODO: create PLAYER, TEAM, STATE, and COLOR tables in the ACC_BBALL
  // database
  //      load each table with rows from the provided source txt files
  if (TABLE_initializer(C.get()) == 0)
    cout << "Created tables successfully" << endl;
  exercise(C.get());

  DATA_initializer(C.get(), STATE, "state.txt");
  DATA_initializer(C.get(), COLOR, "color.txt");
  DATA_initializer(C.get(), TEAM, "team.txt");
  DATA_initializer(C.get(), PLAYER, "player.txt");
  // Close database connection
  C->disconnect();
  return 0;
}
bool arealphas(std::string &str) {
  for (auto it = str.begin(); it != str.end(); ++it) {
    if (*it == '.' || *it == '-')
      continue;
    else if (*it == '\'') {
      str.insert(it, '\'');
      ++it;
    } else if (!isalpha(*it))
      return false;
  }
  return true;
}
std::string generateValue(std::string &line) {
  std::string res;
  std::string seg = fetchNextSeg(line);
  if (arealphas(seg))
    res += "'" + seg + "'";
  else
    res += seg;
  while ((seg = fetchNextSeg(line)) != "") {
    if (arealphas(seg))
      res += ", '" + seg + "'";
    else
      res += ", " + seg;
  }
  return res;
}
int DATA_initializer(connection *C, const std::string &target,
                     const std::string filename) {
  work W(*C);
  try {
    File file(filename);
    std::string line;
    std::string sql;
    while ((line = file.getNextLine()) != std::string()) {
      line = generateValue(line);
      sql = target + "VALUES(" + line + ");";
      W.exec(sql);
    }
    W.commit();
  } catch (const std::exception &e) {
    W.abort();
    cerr << e.what() << endl;
    return 1;
  }
  return 0;
}
int TABLE_initializer(connection *C) {
  work W(*C);
  try {
    // drop table if exists
    W.exec("DROP TABLE IF EXISTS\"PLAYER\";DROP TABLE IF EXISTS\"TEAM\";DROP "
           "TABLE IF EXISTS"
           "\"STATE\";DROP TABLE IF EXISTS \"COLOR\";");

    // create table STATE
    string sql = "CREATE TABLE \"STATE\"("
                 "\"STATE_ID\" INT PRIMARY KEY NOT NULL,"
                 "\"NAME\" CHAR(2) NOT NULL);";
    W.exec(sql);

    // create table COLOR
    sql = "CREATE TABLE \"COLOR\"("
          "\"COLOR_ID\" INT PRIMARY KEY NOT NULL,"
          "\"NAME\"VARCHAR(10) NOT NULL);";
    W.exec(sql);

    // create table TEAM
    sql = "CREATE TABLE \"TEAM\"("
          "\"TEAM_ID\" INT PRIMARY KEY NOT NULL,"
          "\"NAME\" VARCHAR(20) NOT NULL,"
          "\"STATE_ID\" INT REFERENCES \"STATE\"(\"STATE_ID\"),"
          "\"COLOR_ID\" INT REFERENCES \"COLOR\"(\"COLOR_ID\"),"
          "\"WINS\" INT NOT NULL DEFAULT 0,"
          "\"LOSSES\" INT NOT NULL DEFAULT 0);";
    W.exec(sql);

    // create table PLAYER
    sql = "CREATE TABLE \"PLAYER\"(\"PLAYER_ID\" INT PRIMARY KEY NOT NULL,"
          "\"TEAM_ID\" INT REFERENCES \"TEAM\"(\"TEAM_ID\"),"
          "\"UNIFORM_NUM\" INT NOT NULL, \"FIRST_NAME\" VARCHAR(20) NOT NULL,"
          "\"LAST_NAME\" VARCHAR(20) NOT NULL,"
          "\"MPG\" INT, "
          "\"PPG\" INT,"
          "\"RPG\" INT, "
          "\"APG\" INT, "
          "\"SPG\" DECIMAL(3, 1),"
          "\"BPG\" DECIMAL(3, 1));";
    W.exec(sql);
    W.commit();
  } catch (const std::exception &e) {
    cerr << e.what() << std::endl;
    W.abort();
    return 1;
  }
  return 0;
}
