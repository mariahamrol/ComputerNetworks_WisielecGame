#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <iostream>
#include <cstring>
#include <cstdlib>

using namespace std;

vector<string> users;
mutex users_mutex;
vector<int> games_ids;
mutex games_mutex;

void list_games(int fd){
    lock_guard<mutex> lock(games_mutex);
    string game_list;
    for(const auto &id : games_ids){
        game_list += to_string(id) + "\n";
    }
    if(games_ids.empty()){
        game_list = "No existing games.\n";
    }
    if(write(fd, game_list.c_str(), game_list.size()) <= 0){
        cerr << "Failed to send game list to client.\n";
    }}


void create_game(){
    int game_id = rand();
    games_ids.push_back(game_id);
    cout << "Created game with ID: " << game_id << "\n";
    cout <<games_ids[0] << "\n";
}

void joining_game(int fd){
    char response[1024];
    int n = read(fd, response, sizeof(response) - 1);
    if(n <= 0) return;
    response[n] = '\0';
    if(string(response) == "JOIN_GAME"){
        int game_id;
        n = read(fd,&game_id,sizeof(game_id));
        cout << "Gamer wants to join game with ID: " << game_id << "\n";
    } else if(string(response) == "CREATE_GAME"){
        cout << "Hey creating a game\n";
        create_game();
        list_games(fd);
    }
}

void gamer_login(int fd){
    char buf[1024];
    while(true){
        int n = read(fd, buf, sizeof(buf) - 1);
        if(n <= 0) break;
        buf[n] = '\0';

        string login(buf);
        while(!login.empty() && (login.back() == '\n' || login.back() == '\r')) login.pop_back();
        if(login.empty()) continue;

        {
            lock_guard<mutex> lock(users_mutex);
            bool exists = false;
            for(const auto &u : users){
                if(u == login){ exists = true; break; }
            }
            if(exists){
                int taken = 0;
                if(write(fd, &taken, sizeof(taken)) <= 0) break;
                continue;
            } else {
                users.push_back(login);
                int success = 1;
                if(write(fd, &success, sizeof(success)) <= 0) break;
                cout << "User logged in: " << login << "\n";
                break;
            }
        }
    }
    {
        lock_guard<mutex> lock(users_mutex);
        cout << "Current users (" << users.size() << "):\n";
        for(const auto &u : users) cout << u << "\n";
    }
}

void manage_gamer(int fd){
    gamer_login(fd);
    list_games(fd);
    joining_game(fd);
    close(fd);
}


int main(int argc, char **argv){
    char codeword[1024] = "Little white lies";
    int user_count=0;
	int server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_fd < 0) {
		perror("socket");
		return 1;
	}

	int opt = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	sockaddr_in server{};
    sockaddr_in gamer{};

	server.sin_family = AF_INET;
	server.sin_port = htons(12345);
	server.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (::bind(server_fd, (sockaddr*)&server, sizeof(server)) == -1) {
		perror("bind");
		return 1;
	}

    memset(&server, 0, sizeof(server));

    bind(server_fd,(sockaddr*)&server,sizeof(server));
    listen(server_fd,10);

    while(1){                                   
        socklen_t size = sizeof(gamer);
        int gamer_fd = accept(server_fd,(sockaddr*)&gamer,&size);
        if(gamer_fd < 0){ perror("accept"); continue; }
        user_count ++;
        std::thread t1(manage_gamer,gamer_fd);
        t1.detach();
    }
    close(server_fd);
    return 0;
}