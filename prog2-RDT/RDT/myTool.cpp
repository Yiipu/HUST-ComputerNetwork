#include "stdafx.h"
#include "Global.h"

void printWindowStatusDeque(const char* prompt, const std::deque<std::pair<Packet, bool>>& window, int winLen)
{
    cout << prompt;
    for (auto i = window.begin(); i < window.end(); i++) {
        if (i->second) {
            std::cout << GREEN << " " << i->first.seqnum << " " << RESET;
        }
        else {
            std::cout << RED << " " << i->first.seqnum << " " << RESET;
        }
    }
    for (unsigned int i = window.size(); i < winLen; i++) {
        std::cout << YELLOW << " x " << RESET;
    }
    std::cout << std::endl;
}

void printWindowStatusHashTable(const char* prompt, const std::unordered_map<unsigned int, Packet> window, unsigned int base, unsigned int winLen)
{
    cout << prompt;
    for (unsigned int i = base; i < base + winLen; ++i) {
        if (window.find(i) != window.end()) {
            std::cout << GREEN << " " << window.find(i)->second.seqnum << " " << RESET;
        }
        else {
            std::cout << YELLOW << " x " << RESET;
        }
    }
    std::cout << std::endl;
}

void printWindowStatusGBN(const char* prompt, const std::deque<Packet>& window, unsigned int base, int winLen)
{
    cout << prompt;
    for (auto i = window.begin(); i < window.end(); i++) {
        std::cout << RED << " " << i->seqnum << " " << RESET;
    }
    for (unsigned int i = window.size(); i < winLen; i++) {
        std::cout << YELLOW << " x " << RESET;
    }
    std::cout << std::endl;
}
