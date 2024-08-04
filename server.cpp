#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <thread>
#include <atomic>
#include <vector>

using namespace std;

#define PORT 9090
#define BUFFER_SIZE 65507
#define MAX_CLIENTS 5
#define END_MESSAGE "END_OF_FILE"

atomic<int> file_count(0);

void handleClient(int sockfd) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    int client_id = file_count++;
    string filename = "received_file_" + to_string(client_id) + ".mp4";
    ofstream outfile(filename, ios::binary);
    if (!outfile.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    cout << "Receiving data for " << filename << endl;

    int received_bytes;
    while(true) {
        received_bytes = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
        if(received_bytes < 0){
            perror("Receive Failed");
            break;
        }
        buffer[received_bytes] = '\0';
        if(strcmp(buffer, END_MESSAGE)==0){
            break;
        }
        outfile.write(buffer, received_bytes);
    }

    if (received_bytes < 0) {
        perror("Receive failed");
    }

    outfile.close();
    cout << "File received and saved as '" << filename << "'" << endl;
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    cout << "Server listening on port " << PORT << endl;

    vector<thread> threads;
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        threads.emplace_back(handleClient, sockfd);
    }

    for (auto& th : threads) {
        th.join();
    }

    close(sockfd);
    return 0;
}
