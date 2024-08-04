#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <vector>


using namespace std;

#define SERVER_IP "127.0.0.1"
#define PORT 9090
#define BUFFER_SIZE 65507
#define MAX_CLIENTS 5
#define END_MESSAGE "END_OF_FILE"

void sendFile(const char* filename) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    ifstream infile(filename, ios::binary);
    if (!infile.is_open()) {
        cerr << "Error opening file" << endl;
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    cout << "Sending file to server..." << endl;

   // int read_bytes = infile.gcount();
    while ( infile.read(buffer, BUFFER_SIZE) ||infile.gcount() > 0) {
        if (sendto(sockfd, buffer, infile.gcount(), 0, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Send failed");
            close(sockfd);
            infile.close();
            exit(EXIT_FAILURE);
        }
    }

    if (sendto(sockfd, END_MESSAGE, strlen(END_MESSAGE), 0, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Send failed");
        close(sockfd);
        infile.close();
        exit(EXIT_FAILURE);
    }

    infile.close();
    close(sockfd);
    cout << "File sent successfully" << endl;
}

int main() {
    vector<thread> threads;
    const char* filename = "client_file.mp4";

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        threads.emplace_back(sendFile, filename);
    }

   
 for (auto& th : threads) {
        th.join();
    }

    return 0;
}
