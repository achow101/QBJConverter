#include <iostream>
#include <fstream>
#include "json/json.h"

using namespace std;

int main(int argc, char *argv[]) {

    string helpMessage = "Usage: QBJConverter <filename>\n"
                        "<filename>: The path to the .qbj file";

    if(argc != 2)
    {
        cout << helpMessage << endl;
        return -1;
    }

    string qbjFile = argv[1];
    Json::ArrayIndex ZERO = 0;
    int power = 15;
    int base = 10;
    int neg = -5;

    // To Json Object
    Json::Value root;
    std::ifstream neg5Qbj(qbjFile, std::ifstream::binary);
    Json::CharReaderBuilder rbuilder;
    // Configure the Builder, then ...
    std::string errs;
    bool parsingSuccessful = Json::parseFromStream(rbuilder, neg5Qbj, &root, &errs);
    if (!parsingSuccessful)
    {
        // report to the user the failure and their locations in the document.
        std::cout  << "Failed to parse configuration\r\n"
        << errs;
        return -1;
    }

    // Stuff for tracking objects for SQBS
    int teamCount = 0;
    vector<vector<string>> teams;
    vector<string> teamIds;

    // Grab teams
    Json::Value objects = root.get("objects", "none");
    for(int i = 0; i < objects.size(); i++)
    {
        Json::Value obj = objects.get(i, "none");
        Json::Value idObj = obj.get("id", "none");
        string id = idObj.asString();
        if(id.find("school") != string::npos)
        {
            Json::Value teamsObj = obj.get("teams", "none");
            for(int j = 0; j < teamsObj.size(); j++)
            {
                teamCount++;

                Json::Value teamObj = teamsObj.get(j, "none");

                Json::Value players = teamObj.get("players", "none");
                vector<string> team;
                team.push_back(teamObj.get("name", "none").asString());
                for(int j = 0; j < players.size(); j++)
                {
                    team.push_back(players.get(j, "none").get("name", "none").asString());
                }
                teams.push_back(team);
                teamIds.push_back(teamObj.get("id", "none").asString());
            }

        }
    }

    // File write stuff
    ofstream sqbsFile;
    sqbsFile.open (qbjFile + ".sqbs");

    // Write number of teams to file
    sqbsFile << teamCount << "\r\n";

    // Write players to file
    for (int k = 0; k < teams.size(); ++k) {
        vector<string> team = teams.at(k);
        sqbsFile << team.size() << "\r\n";
        for (int i = 0; i < team.size(); ++i) {
            sqbsFile << team.at(i) << "\r\n";
        }
    }

    // Create blank vectors
    vector<double> zeroes; // 6 Zeroes
    for (int n = 0; n < 6; ++n) {
        zeroes.push_back(0);
    }
    vector<double> blankPerson;
    blankPerson.push_back(-1);
    for (int n = 0; n < 6; ++n) {
        blankPerson.push_back(0);
    }

    // Get games
    int gameCount = 0;
    vector<int> teamIndices;
    vector<int> teamFinalScores;
    vector<vector<double>> playerStats;
    vector<int> roundNums;
    vector<int> bnsHrd;
    vector<int> bnsPts;
    int tuh = 20;
    for(int i = 0; i < objects.size(); i++) {
        Json::Value obj = objects.get(i, "none");
        Json::Value idObj = obj.get("id", "none");
        string id = idObj.asString();
        if (id.find("game") != string::npos) {
            gameCount++;

            // Add zeroes
            playerStats.push_back(zeroes);

            // Get TUH
            tuh = obj.get("tossups", "none").asInt();

            // Get round number
            int round = obj.get("round", "none").asInt();
            roundNums.push_back(round);

            // For each team
            vector<vector<double>> teamStats;
            for (int i = 0; i < obj.get("match_teams", "none").size(); ++i) {

                // Get the team
                Json::Value team = obj.get("match_teams", "none").get(i, "none");

                // Find the team index
                int teamInx = 0;
                string teamId = team.get("team", "none").get("$ref", "none").asString();
                for (int m = 0; m < teamIds.size(); ++m) {
                    if (teamIds.at(m).find(teamId) != string::npos) {
                        teamInx = m;
                    }
                }
                teamIndices.push_back(teamInx);

                // Get each player on team
                Json::Value teamPlayers = team.get("match_players", "none");

                // Get scores and stats
                int teamScore = 0;
                int teamBnHrd = 0;
                for (int l = 0; l < 8; ++l) {

                    vector<double> playerStat;

                    if(l < teamPlayers.size()) {

                        string playerName = teamPlayers.get(l, "none").get("player", "none").get("name", "none").asString();
                        // Find player index
                        vector<string> team = teams.at(teamInx);
                        for (int k = 0; k < team.size(); ++k) {
                            if (team.at(k).find(playerName) != string::npos)
                                playerStat.push_back(k - 1);
                        }

                        // Calculate Games Played
                        int tosuphrd = teamPlayers.get(l, "none").get("tossups_heard", "none").asDouble();
                        double gp = teamPlayers.get(l, "none").get("tossups_heard", "none").asDouble() / tuh;
                        playerStat.push_back(gp);

                        // Calculate points for team 1
                        Json::Value answerCounts = teamPlayers.get(l, "none").get("answer_counts", "none");
                        int bases = 0;
                        int playerScore = 0;
                        for (int j = 0; j < answerCounts.size(); ++j) {
                            int value = answerCounts.get(j, "none").get("value", "none").asInt();
                            int number = answerCounts.get(j, "none").get("number", "none").asInt();
                            teamScore += value * number;
                            playerScore += value * number;

                            // Add the numbers for each player
                            if (value > 0)
                                teamBnHrd += number;
                            if (value == base)
                                bases += number;
                            if (value == power) {
                                playerStat.push_back(number);
                                playerStat.push_back(bases);
                            }
                            if (value == neg)
                                playerStat.push_back(number);
                        }
                        playerStat.push_back(0);
                        playerStat.push_back(playerScore);

                        // Add to team stats
                        teamStats.push_back(playerStat);
                    }
                    else
                    {
                        teamStats.push_back(blankPerson);
                    }
                }
                teamScore += team.get("bonus_points", "none").asInt();

                // Add the final scores to vector
                teamFinalScores.push_back(teamScore);
                bnsHrd.push_back(teamBnHrd);
                bnsPts.push_back(team.get("bonus_points", "none").asInt());
            }


            for (int n = 0; n < teamStats.size() / 2; ++n) {
                playerStats.push_back(teamStats.at(n));
                playerStats.push_back(teamStats.at(n + 8));
            }

        }
    }

    // Write games to file
    sqbsFile << gameCount << "\r\n";
    int stopOffset = 0;
    int roundNumCounter = 0;
    int idNum = 0;
    for(int i = 0; i < gameCount * 2; i += 2)
    {
        sqbsFile << idNum << "\r\n";
        sqbsFile << teamIndices.at(i) << "\r\n";
        sqbsFile << teamIndices.at(i + 1) << "\r\n";
        sqbsFile << teamFinalScores.at(i) << "\r\n";
        sqbsFile << teamFinalScores.at(i + 1) << "\r\n";
        sqbsFile << tuh << "\r\n";
        sqbsFile << roundNums.at(roundNumCounter) << "\r\n";
        sqbsFile << bnsHrd.at(i) << "\r\n";
        sqbsFile << bnsPts.at(i) << "\r\n";
        sqbsFile << bnsHrd.at(i + 1) << "\r\n";
        sqbsFile << bnsPts.at(i + 1) << "\r\n";

        for(int j = stopOffset; j < stopOffset + 17; ++j) {
            vector<double> stats = playerStats.at(j);
            for (int k = 0; k < stats.size(); ++k) {
                sqbsFile << stats.at(k) << "\r\n";
            }
        }
        stopOffset += 17;
        roundNumCounter++;
        idNum++;
    }

    // Write sqbs settings stuff
    sqbsFile << "1\r\n1\r\n3\r\n0\r\n1\r\n2\r\n254\r\n0\r\n1\r\n1\r\n1\r\n1\r\n1\r\n1\r\n0\r\n1\r\n1\r\n";

    // Get and write tournament name
    string tournamentName = objects.get(objects.size() - 1, "none").get("name", "name").asString();
    sqbsFile << tournamentName << "\r\n\r\n\r\n\r\n\r\n";

    // Write file suffixes for html
    sqbsFile << "1\r\n_rounds.html\r\n_standings.html\r\n_individuals.html\r\n_games.html\r\n_teamdetail.html\r\n_playerdetail.html\r\n_statkey.html\r\n\r\n";

    // Write division stuff which is not in the qbj file
    sqbsFile << 0 << "\r\n";
    sqbsFile << teamCount << "\r\n";
    for (int i1 = 0; i1 < teamCount; ++i1) {
        sqbsFile << -1 << "\r\n";
    }

    // Point values
    sqbsFile << power << "\r\n" << base << "\r\n" << neg << "\r\n0\r\n0\r\n";

    // Numbers at end I don't understand
    sqbsFile << teamCount << "\r\n";
    for (int i1 = 0; i1 < teamCount; ++i1) {
        sqbsFile << 0 << "\r\n";
    }

    return 0;
}