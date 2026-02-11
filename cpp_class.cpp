#include <iostream>
#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <cerrno>

// I can also get those as user input via command line 
// for simplicity I hard code them here
#define DEST_IP "127.0.0.1" // there's no place like local host
#define DEST_PORT 12345
#define DELAY_SECONDS  5
#define INTERVAL 2

using namespace std;

class SendUDP {
    int sock;

  public:
    int send_immediately(string ip, unsigned short port, string payload);
    int send_delayed(string ip, unsigned short port, string payload, uint8_t delay);
    int send_repeatedly(string ip, unsigned short port, string payload, uint8_t interval);    
    void close_socket();
};



int SendUDP::send_repeatedly(string ip, unsigned short port, string payload, uint8_t interval){
  if (interval < 0 || interval > 255) {
    cout << "Interval wrong. Must be between 1 and 255.\n";
    exit(EXIT_FAILURE);    
  }

  sockaddr_in dest{};
  dest.sin_family = AF_INET;
  dest.sin_port = htons(DEST_PORT);
  if (inet_pton(AF_INET, ip.c_str(), &dest.sin_addr) != 1) {
      std::cerr << "inet_pton failed\n";
      close(sock);
      return 1;
  }

  std::cout << "Starting to broadcast. Will send every " << static_cast<int>(interval) << " seconds...\n";

  // Create non-blocking UDP socket
  int sock = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
  if (sock < 0) {
      perror("socket");
      return 1;
  }

  std::string msg = "Periodic UDP message";
  std::cout << "Sending every " << static_cast<int>(interval) << " seconds...\n";

  auto next_send = std::chrono::steady_clock::now();

  while (true) {
    next_send += std::chrono::seconds(interval);

    ssize_t n = sendto(sock, msg.data(), msg.size(), 0, reinterpret_cast<sockaddr*>(&dest), sizeof(dest));

    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            std::cout << "Send buffer full — skipping this round\n";
        } else {
            std::cerr << "sendto failed: " 
                      << std::strerror(errno) << "\n";
        }
    } else {
        std::cout << "Sent " << n << " bytes\n";
    }

    std::this_thread::sleep_until(next_send);
  }
}





int SendUDP::send_delayed(string ip, unsigned short port, string payload, uint8_t delay){
  if (delay < 0 || delay > 255) {
    cout << "Delay wrong. Must be between 1 and 255.\n";
    exit(EXIT_FAILURE);    
  }
  std::cout << "Starting delayed broadcast. Will send after " << static_cast<int>(delay) << " seconds...\n";

  // Delay before sending
  std::this_thread::sleep_for(std::chrono::seconds(delay));

  // Create non-blocking UDP socket (simple way)
  int sock = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
  if (sock < 0) {
      perror("socket");
      return 1;
  }

  sockaddr_in dest{};
  dest.sin_family = AF_INET;
  dest.sin_port = htons(DEST_PORT);
  if (inet_pton(AF_INET, ip.c_str(), &dest.sin_addr) != 1) {std::cerr << "inet_pton failed\n";
      close(sock);
      return 1;
  }

  std::string msg = "Hello after delay (non-blocking)!";

  ssize_t n = sendto(sock, msg.data(), msg.size(), 0,
                     reinterpret_cast<sockaddr*>(&dest), sizeof(dest));

  if (n < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
          std::cout << "Would block; try again later.\n";
      } else {
          std::cerr << "sendto failed: " << std::strerror(errno) << "\n";
      }
  } 
  return n;
}




int SendUDP::send_immediately(string ip, unsigned short port, string payload) {

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        cout << "Failed to create socket\n";
        exit(EXIT_FAILURE);
    }

    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) != 1) {
        cout << "Invalid IP address\n";
        exit(EXIT_FAILURE);
    }

    int sent = sendto(sock, payload.c_str(), payload.size(), 
      0, (sockaddr*)&addr, sizeof(addr));

    if (sent < 0) {
        cout << "UDP send failed\n";
        exit(EXIT_FAILURE);
    }

    return sent;
}


void SendUDP::close_socket() {
    if (sock >= 0)
        close(sock);
}

int main() {
    SendUDP sender;
// send udp packet
    int bytes = sender.send_immediately(DEST_IP, DEST_PORT, "hello world");
    cout << "sent " << bytes << " bytes\n";
    sender.close_socket();

// send delayed udp packet
    bytes = sender.send_delayed(DEST_IP, DEST_PORT, "hello world, sorry I am late!", DELAY_SECONDS);
    cout << "sent " << bytes << " bytes\n";
    sender.close_socket();    
    
// send repeatedly udp packets
    sender.send_repeatedly(DEST_IP, DEST_PORT, "its me again!", INTERVAL);
    cout << "sent " << bytes << " bytes\n";
    sender.close_socket();    
    return 0;
}
