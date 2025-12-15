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

using namespace std;

//User must be listening for messages from server or administrator so this is important but no idea how to implement it now
void listener(int fd){
    while(1){
        char message[1024];
        ssize_t messageCount = read(fd,message,sizeof(message));
        if(messageCount <= 0) break;
        write(1,message,messageCount);
    }
}

void create_game(int fd){
    cout << "Do you want to create a game? y/n: ";
    char answer;
    cin >> answer;
    if(answer != 'y' && answer != 'Y') {
        const char* joinGameMsg = "JOIN_GAME";
        write(fd, joinGameMsg, strlen(joinGameMsg));
        int game_id;
        int n = read(0,&game_id,sizeof(game_id));
        write(fd,&game_id,n);
    }else{
        cout << "Creating game...\n";
        const char* createGameMsg = "CREATE_GAME";
        write(fd,createGameMsg,strlen(createGameMsg));
    }
}
void joining_game(int fd){
    cout << "Existing games:\n";
    char gamesList[1024];
    int readCount = read(fd,gamesList,sizeof(gamesList));
    cout.write(gamesList,readCount);
    create_game(fd);
}

void logining_in(int fd){
    cout << "Enter your login: \n";
    bool login_correct = false;
    while(login_correct==false){
        char login[1024];
        int n = read(0,login,sizeof(login));
        if(n>0){
            login[n] = '\0';
            write(fd, login, strlen(login));
            int response;
            int readCount = read(fd,&response,sizeof(response));
            if(response==0){
                cout<<"Login taken, try again.\n";
            } else {
                cout << "Welcome to the game " << login << "\n";
                login_correct = true;
                break;
            }
        }
    }
}
int main(int argc, char **argv){
    int gamer_fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(gamer_fd < 0){ 
        perror("socket"); 
        return 1; 
    }
    sockaddr_in gamer;
    gamer.sin_family = AF_INET;
    gamer.sin_port = htons(12345);
    gamer.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(gamer_fd,(sockaddr*)&gamer,sizeof(gamer));
    logining_in(gamer_fd);
    cout << "Login ended...\n";
    
    // start listener before joining_game so you receive broadcasts
    std::thread listenerThread(listener,gamer_fd);
    listenerThread.detach();
    
    joining_game(gamer_fd);
    while(true) sleep(1);
    close(gamer_fd);
    return 0;
}