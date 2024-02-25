#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <cstring>
#include <unistd.h>

namespace dunedaq::daphnemodules {

class OEI {
private:
    int sock;
    struct sockaddr_in target;

public:
    OEI(const char* ipaddr) {
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        target.sin_family = AF_INET;
        target.sin_port = htons(2001);
        inet_pton(AF_INET, ipaddr, &(target.sin_addr));
    }

    std::vector<uint64_t> read(uint64_t addr, uint8_t num) {
        std::vector<uint64_t> temp;
       // temp.erase(0,num);
        uint8_t cmd[10];
        cmd[0] = 0x00;
        cmd[1] = num;
        memcpy(cmd + 2, &addr, sizeof(uint64_t));
        sendto(sock, cmd, sizeof(cmd), 0, (struct sockaddr*)&target, sizeof(target));
        uint8_t buffer[2 + (8 * num)];
        socklen_t addrlen = sizeof(target);
        recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&target, &addrlen);
        uint8_t fmt[4 + num];
        fmt[0] = '<';
        fmt[1] = 'B';
        fmt[2] = 'B';
        fmt[3] = num;
        for (int i = 0; i < num; i++) {
            fmt[4 + i] = 'Q';
        }
        for (int i = 0; i < num; i++) {
            uint64_t value;
            memcpy(&value, buffer + 2 + (8 * i), sizeof(uint64_t));
            temp.push_back(value);
        }
        return temp;
    }

    void write(uint64_t addr, std::vector<uint64_t> data) {
        uint8_t cmd[10 + (8 * data.size())];
        cmd[0] = 0x01;
        cmd[1] = data.size();
        memcpy(cmd + 2, &addr, sizeof(uint64_t));
        for (int i = 0; i < data.size(); i++) {
            memcpy(cmd + 10 + (8 * i), &(data[i]), sizeof(uint64_t));
        }
        sendto(sock, cmd, sizeof(cmd), 0, (struct sockaddr*)&target, sizeof(target));
    }

    std::vector<uint64_t> readf(uint64_t addr, uint8_t num) {
        std::vector<uint64_t> temp;
        uint8_t cmd[10];
        cmd[0] = 0x08;
        cmd[1] = num;
        memcpy(cmd + 2, &addr, sizeof(uint64_t));
        sendto(sock, cmd, sizeof(cmd), 0, (struct sockaddr*)&target, sizeof(target));
        uint8_t buffer[2 + (8 * num)];
        socklen_t addrlen = sizeof(target);
        recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&target, &addrlen);
        uint8_t fmt[4 + num];
        fmt[0] = '<';
        fmt[1] = 'B';
        fmt[2] = 'B';
        fmt[3] = num;
        for (int i = 0; i < num; i++) {
            fmt[4 + i] = 'Q';
        }
        for (int i = 0; i < num; i++) {
            uint64_t value;
            memcpy(&value, buffer + 2 + (8 * i), sizeof(uint64_t));
            temp.push_back(value);
        }
        return temp;
    }

    void writef(uint64_t addr, std::vector<uint64_t> data) {
        uint8_t cmd[10 + (8 * data.size())];
        cmd[0] = 0x09;
        cmd[1] = data.size();
        memcpy(cmd + 2, &addr, sizeof(uint64_t));
        for (int i = 0; i < data.size(); i++) {
            memcpy(cmd + 10 + (8 * i), &(data[i]), sizeof(uint64_t));
        }
        sendto(sock, cmd, sizeof(cmd), 0, (struct sockaddr*)&target, sizeof(target));
    }

    void closes() {
    close(sock);
    }
};
}
