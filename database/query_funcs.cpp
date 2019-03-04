#include "query_funcs.h"

void add_player(connection *C, int team_id, int jersey_num, string first_name,
                string last_name, int mpg, int ppg, int rpg, int apg,
                double spg, double bpg) {}

void add_team(connection *C, string name, int state_id, int color_id, int wins,
              int losses) {}

void add_state(connection *C, string name) {}

void add_color(connection *C, string name) {}

inline void generateBTSQL(std::string para, int min, int max, std::string &sql,
                          int &btcount, int &andcount) {
  if (andcount < btcount) {
    sql += " AND ";
    ++andcount;
  }
  sql += "(\"" + para + "\" BETWEEN " + std::to_string(min) + " AND " +
         std::to_string(max) + " )";
  ++btcount;
}
void query1(connection *C, int use_mpg, int min_mpg, int max_mpg, int use_ppg,
            int min_ppg, int max_ppg, int use_rpg, int min_rpg, int max_rpg,
            int use_apg, int min_apg, int max_apg, int use_spg, double min_spg,
            double max_spg, int use_bpg, double min_bpg, double max_bpg) {
  std::string sql = "SELECT * FROM \"PLAYER\" ";
  if (use_mpg || use_ppg || use_rpg || use_apg || use_spg || use_bpg)
    sql += "WHERE ";
  int btcount = 0;
  int andcount = 0;
  if (use_mpg)
    generateBTSQL("MPG", min_mpg, max_mpg, sql, btcount, andcount);
  if (use_ppg)
    generateBTSQL("PPG", min_ppg, max_ppg, sql, btcount, andcount);
  if (use_rpg)
    generateBTSQL("RPG", min_rpg, max_rpg, sql, btcount, andcount);
  if (use_apg)
    generateBTSQL("APG", min_apg, max_apg, sql, btcount, andcount);
  if (use_spg)
    generateBTSQL("SPG", min_spg, max_spg, sql, btcount, andcount);
  if (use_bpg)
    generateBTSQL("BPG", min_bpg, max_bpg, sql, btcount, andcount);
  sql += ";";
  nontransaction N(*C);
  result R(N.exec(sql));
  std::cout << "PLAYER_ID TEAM_ID UNIFORM_NUM FIRST_NAME LAST_NAME MPG PPG RPG "
               "APG SPG BPG\n";
  for (auto c : R) {
    std::cout << c[0].as<int>() << " " << c[1].as<int>() << " "
              << c[2].as<int>() << " " << c[3].as<string>() << " "
              << c[4].as<string>() << " " << c[5].as<int>() << " "
              << c[6].as<int>() << " " << c[7].as<int>() << " "
              << c[8].as<int>() << " " << c[9].as<float>() << " "
              << c[10].as<float>() << " " << endl;
  }
}

void query2(connection *C, string team_color) {
  /*std::string sql = "SELECT \"TEAM\".\"NAME\" FROM \"TEAM\" WHERE "
                    "\"TEAM\".\"COLOR_ID\" IN (SELECT \"COLOR_ID\"FROM "
                    "\"COLOR\" WHERE \"COLOR\".\"NAME\"='" +
                    team_color + "');";*/
  // OR
  std::string sql = "SELECT \"TEAM\".\"NAME\" FROM \"TEAM\",\"COLOR\" WHERE "
                    "\"TEAM\".\"COLOR_ID\"=\"COLOR\".\"COLOR_ID\" AND"
                    "\"COLOR\".\"NAME\"='" +
                    team_color + "';";

  nontransaction N(*C);
  result R(N.exec(sql));
  std::cout << "NAME\n";
  for (auto c : R) {
    std::cout << c[0].as<string>() << endl;
  }
}

void query3(connection *C, string team_name) {
  std::string sql =
      "SELECT \"FIRST_NAME\", \"LAST_NAME\" FROM \"PLAYER\",\"TEAM\" WHERE"
      "\"PLAYER\".\"TEAM_ID\"=\"TEAM\".\"TEAM_ID\" AND \"TEAM\".\"NAME\"='" +
      team_name + "';";
  nontransaction N(*C);
  result R(N.exec(sql));
  std::cout << "FIRST_NAME LAST_NAME\n";
  for (auto c : R) {
    std::cout << c[0].as<string>() << " " << c[1].as<string>() << std::endl;
  }
}

void query4(connection *C, string team_state, string team_color) {
  std::string sql =
      "SELECT \"FIRST_NAME\", \"LAST_NAME\",\"UNIFORM_NUM\" FROM "
      "\"PLAYER\",\"STATE\",\"COLOR\",\"TEAM\" WHERE "
      "\"STATE\".\"STATE_ID\"=\"TEAM\".\"STATE_ID\" AND "
      "\"PLAYER\".\"TEAM_ID\"=\"TEAM\".\"TEAM_ID\" AND "
      "\"COLOR\".\"COLOR_ID\"=\"TEAM\".\"COLOR_ID\" AND \"STATE\".\"NAME\"='" +
      team_state + "' AND \"COLOR\".\"NAME\"='" + team_color + "';";
  nontransaction N(*C);
  result R(N.exec(sql));
  std::cout << "FIRST_NAME LAST_NAME UNIFORM_NUM\n";
  for (auto c : R) {
    std::cout << c[0].as<string>() << " " << c[1].as<string>() << " "
              << c[2].as<int>() << std::endl;
  }
}

void query5(connection *C, int num_wins) {
  std::string sql =
      "SELECT \"FIRST_NAME\", \"LAST_NAME\",\"TEAM\".\"NAME\",\"WINS\" FROM "
      "\"PLAYER\", \"TEAM\" WHERE \"PLAYER\".\"TEAM_ID\"=\"TEAM\".\"TEAM_ID\" "
      "AND \"TEAM\".\"WINS\">" +
      std::to_string(num_wins) + ";";
  nontransaction N(*C);
  result R(N.exec(sql));
  std::cout << "FIRST_NAME LAST_NAME NAME WINS\n";
  for (auto c : R) {
    std::cout << c[0].as<string>() << " " << c[1].as<string>() << " "
              << c[2].as<string>() << " " << c[3].as<int>() << std::endl;
  }
}
