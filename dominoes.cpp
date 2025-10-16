#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <deque>
#include <chrono>

// CSCI 300 Midterm
// Group 4
// Oshan Bandara
// Dominoes Game
// Date: 13/10/25

// Domino Structure
struct Domino {
    int left;
    int right;

    Domino(int a = 0, int b = 0) : left(a), right(b) {}

    bool isDouble() const { return left == right; }
    int sum() const { return left + right; }

    std::string toString() const {
        return "[" + std::to_string(left) + "|" + std::to_string(right) + "]";
    }

    Domino flipped() const { return Domino(right, left); }
};


// Randm Generator Class
class CRandom {
private:
    std::mt19937 rng;

public:
    CRandom(unsigned seed = (unsigned)std::chrono::system_clock::now().time_since_epoch().count()) {
        rng.seed(seed);
    }

    ~CRandom() {}

    template<typename T>
    void shuffleVec(std::vector<T>& v) {
        std::shuffle(v.begin(), v.end(), rng);
    }

    int randInt(int lo, int hi) {
        std::uniform_int_distribution<int> dist(lo, hi);
        return dist(rng);
    }
};

// The Domino Set
class CDominoes {
private:
    std::vector<Domino> deck;

public:
    CDominoes() { create_dominoes(); }
    ~CDominoes() {}

    void create_dominoes() {
        deck.clear();
        for (int i = 0; i <= 6; ++i) {
            for (int j = i; j <= 6; ++j) {
                deck.emplace_back(i, j);
            }
        }
    }

    void shuffle(CRandom &r) {
        r.shuffleVec(deck);
    }

    bool draw_domino(Domino &d) {
        if (deck.empty()) return false;
        d = deck.back();
        deck.pop_back();
        return true;
    }

    size_t size() const { return deck.size(); }
};


// Table Class
class CTable {
private:
    std::deque<Domino> train;

public:
    void placeFirst(const Domino &d) { train.clear(); train.push_back(d); }
    void placeLeft(const Domino &d) { train.push_front(d); }
    void placeRight(const Domino &d) { train.push_back(d); }

    int leftPip() const { return train.empty() ? -1 : train.front().left; }
    int rightPip() const { return train.empty() ? -1 : train.back().right; }

    bool empty() const { return train.empty(); }

    void show() const {
        std::cout << "Table: ";
        if (train.empty()) { std::cout << "(empty)\n"; return; }
        for (size_t i = 0; i < train.size(); ++i) {
            std::cout << train[i].toString();
            if (i + 1 < train.size()) std::cout << "--";
        }
        std::cout << "\n";
    }
};

// Player Class
class CPlayer {
private:
    std::string playerName;
    std::vector<Domino> hand;

public:
    CPlayer(std::string name = "Player") : playerName(name) {}
    ~CPlayer() {}

    void receive(const Domino &d) { hand.push_back(d); }
    size_t handSize() const { return hand.size(); }
    std::string getName() const { return playerName; }
    bool hasWon() const { return hand.empty(); }

    void showHand() const {
        std::cout << playerName << " (" << hand.size() << " pieces): ";
        for (size_t i = 0; i < hand.size(); ++i)
            std::cout << hand[i].toString() << " ";
        std::cout << "\n";
    }

    int findHighestDouble() const {
        int highest = -1;
        for (const auto& d : hand)
            if (d.isDouble() && d.left > highest) highest = d.left;
        return highest;
    }

    // (Looking) at a piece without removing it
    const Domino& peekPiece(int index) const {
        return hand[index];
    }

    // Remove and return a piece by index
    Domino playPiece(int index) {
        Domino d = hand[index];
        hand.erase(hand.begin() + index);
        return d;
    }

    bool attemptPlay(int leftEnd, int rightEnd, Domino &playedDomino, bool &placeLeftFlag) {
        for (size_t i = 0; i < hand.size(); ++i) {
            Domino d = hand[i];
            if (d.right == leftEnd) {
                playedDomino = d; placeLeftFlag = true;
                hand.erase(hand.begin() + i); return true;
            } else if (d.left == leftEnd) {
                playedDomino = d.flipped(); placeLeftFlag = true;
                hand.erase(hand.begin() + i); return true;
            } else if (d.left == rightEnd) {
                playedDomino = d; placeLeftFlag = false;
                hand.erase(hand.begin() + i); return true;
            } else if (d.right == rightEnd) {
                playedDomino = d.flipped(); placeLeftFlag = false;
                hand.erase(hand.begin() + i); return true;
            }
        }
        return false;
    }
};


// GameCore Class
class GameCore {
private:
    CRandom randGen;
    CTable table;
    CDominoes boneyard;
    std::vector<CPlayer> players;
    int starter = 0;

public:
    GameCore() {
        players.emplace_back("Player 1");
        players.emplace_back("Player 2");
    }

    ~GameCore() {}

    int who_first() {
        int d1 = players[0].findHighestDouble();
        int d2 = players[1].findHighestDouble();
        if (d1 > d2) return 0;
        if (d2 > d1) return 1;
        return randGen.randInt(0, 1);
    }

    //removes the chosen piece once
    void playInitialPiece(int playerIndex) {
        CPlayer& starterPlayer = players[playerIndex];
        int bestIndex = -1;
        int maxPips = -1;

        // Looking at all pieces without removing them
        for (size_t i = 0; i < starterPlayer.handSize(); ++i) {
            const Domino& d = starterPlayer.peekPiece(i);
            if (d.sum() > maxPips) {
                bestIndex = i;
                maxPips = d.sum();
            }
        }

        // best piece found
        if (bestIndex != -1) {
            Domino d = starterPlayer.playPiece(bestIndex);
            table.placeFirst(d);
            std::cout << starterPlayer.getName()
                      << " placed initial piece: " << d.toString() << "\n";
        }
    }

    void setup() {
        boneyard.create_dominoes();
        boneyard.shuffle(randGen);
        for (int i = 0; i < 10; ++i) {
            Domino d;
            if (boneyard.draw_domino(d)) players[0].receive(d);
            if (boneyard.draw_domino(d)) players[1].receive(d);
        }

        starter = who_first();
        std::cout << "\nGame Setup Complete.\nStarter: " << players[starter].getName() << "\n";
        playInitialPiece(starter);
    }

    void run_game() {
        int current = starter;
        int consecutivePasses = 0;

        while (true) {
            std::cout << "\n----------------------------------------\n";
            std::cout << players[current].getName() << "'s turn.\n";
            players[0].showHand();
            players[1].showHand();
            table.show();

            CPlayer& currentPlayer = players[current];
            Domino played; bool placeLeft = false; bool playedFlag = false;
            int leftEnd = table.leftPip();
            int rightEnd = table.rightPip();

            while (true) {
                if (currentPlayer.attemptPlay(leftEnd, rightEnd, played, placeLeft)) {
                    if (placeLeft) table.placeLeft(played); else table.placeRight(played);
                    std::cout << currentPlayer.getName() << " played " << played.toString() << "\n";
                    playedFlag = true;
                    break;
                } else if (boneyard.size() > 0) {
                    Domino d;
                    boneyard.draw_domino(d);
                    currentPlayer.receive(d);
                    std::cout << currentPlayer.getName() << " draws " << d.toString() << "\n";
                } else {
                    std::cout << currentPlayer.getName() << " passes.\n";
                    consecutivePasses++;
                    break;
                }
            }

            if (currentPlayer.hasWon()) {
                std::cout << "\n========================================\n";
                std::cout << currentPlayer.getName() << " WINS!\n";
                table.show();
                break;
            }

            if (!playedFlag && consecutivePasses >= 2) {
                std::cout << "\n========================================\n";
                std::cout << "Game Blocked! No possible plays.\n";
                table.show();
                break;
            }

            if (playedFlag) consecutivePasses = 0;
            current = 1 - current;
        }

        std::cout << "Game Over.\n";
    }
};


// Main Function
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::cout << "CSCI-300 Dominoes Game - Console Demo\n\n";

    GameCore* core = new GameCore();
    core->setup();
    core->run_game();
    delete core;

    std::cout << "\nProgram finished.\n";
    return 0;
}
