#include <iostream>
#include <vector>
#include <climits>
#include <string>
#include <thread>
#include <limits>
#include <memory>
#include <future>
#include <tuple>
#include <algorithm>
#include <optional>

#include "LRUCache.hpp"

//==================
// Model
//==================

class Board {
public:
    Board(int size) : grid(size, std::vector<char>(size, '.')) {}
    
    int getSize() const { 
        return static_cast<int>(grid.size()); 
    }

    char getCell(int row, int col) const { 
        return grid[row][col]; 
    }

    void setCell(int row, int col, char value) { 
        grid[row][col] = value; 
    }

    bool isFull() const {
        for(const auto& row : grid) {
            for(const auto& cell : row) {
                if(cell == '.') {
                    return false;
                }
            }
        }           
        return true;
    }

    std::string toString() const {
        std::string res;
        for (const auto& row : grid) {
            for(const auto& cell : row) {
                res.push_back(cell);
            }
        }
        return res;
    }

    void print() const {
        for(const auto& row : grid) {
            for(char cell : row) {
                std::cout << cell << std::endl;
            }
            std::cout << "\n";
        }
    }
private:
    std::vector<std::vector<char>> grid;
};

const char X = 'X';
const char O = 'O';
const char EMPTY = '.';

int MAX_DEPTH;
bool USE_CACHE = false;

SKW_WPX::Cache::LRUCache cache;

std::vector<std::vector<char>> createBoard(int size) {
    return std::vector<std::vector<char>>(size, std::vector<char>(size, EMPTY));
}

bool isWinner(const std::vector<std::vector<char>>& board, char player) {
    int size = board.size();
    bool diagonalWin = true;
    bool reverseDiagonalWin = true;

    for (int i = 0; i < size; i++) {
        if (board[i][i] != player) diagonalWin = false;
        if (board[i][size - i - 1] != player) reverseDiagonalWin = false;

        bool rowWin = true;
        bool columnWin = true;

        for (int j = 0; j < size; j++) {
            if (board[i][j] != player) rowWin = false;
            if (board[j][i] != player) columnWin = false;
        }

        if (rowWin || columnWin) return true;
    }

    return diagonalWin || reverseDiagonalWin;
}

bool isBoardFull(const std::vector<std::vector<char>>& board) {
    for (const auto& row : board) {
        for (char cell : row) {
            if (cell == EMPTY) return false;
        }
    }
    return true;
}

void printBoard(const std::vector<std::vector<char>>& board) {
    for (const auto& row : board) {
        for (char cell : row) {
            std::cout << cell << " ";
        }
        std::cout << std::endl;
    }
}

void playerMove(std::vector<std::vector<char>>& board) {
    int x, y;
    while (true) {
        std::cout << "Enter the coordinates of the move (row and column): ";
        std::cin >> x >> y;
        x--; y--;
        if (x >= 0 && x < board.size() && y >= 0 && y < board.size() && board[x][y] == EMPTY) {
            board[x][y] = X;
            break;
        }
        std::cout << "Wrong move. Try again.\n";
    }
}

std::string boardToString(const std::vector<std::vector<char>>& board) {
    std::string boardString;
    for (const auto& row : board) {
        for (char cell : row) {
            boardString.push_back(cell);
        }
    }
    return boardString;
}

int minimax(std::vector<std::vector<char>>& board, int depth, bool isMax, int alpha, int beta) {
    std::string boardKey = boardToString(board) + (isMax ? "1" : "0") + std::to_string(depth);

    if (USE_CACHE) {
        auto cachedValue = cache.get(boardKey);
        if(cachedValue.has_value()) {
            return cachedValue.value();
        }
    }

    if (depth >= MAX_DEPTH) return 0;
    if (isWinner(board, O)) return +10 - depth;
    if (isWinner(board, X)) return -10 + depth;
    if (isBoardFull(board)) return 0;

    int best = isMax ? INT_MIN : INT_MAX;
    int size = board.size();

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if (board[i][j] == EMPTY) {
                board[i][j] = isMax ? O : X;
                int val = minimax(board, depth + 1, !isMax, alpha, beta);
                board[i][j] = EMPTY;
                best = isMax ? std::max(best, val) : std::min(best, val);
                if (isMax) alpha = std::max(alpha, best);
                else beta = std::min(beta, best);
                if (beta <= alpha) break;
            }
        }
    }

    if (USE_CACHE) {
        cache.put(boardKey, best);
    }

    return best;
}

void botMove(std::vector<std::vector<char>>& board) {
    int size = board.size();
    int globalBestVal = INT_MIN;
    int bestMoveRow = -1;
    int bestMoveCol = -1;
    std::vector<std::future<std::tuple<int, int, int>>> futures;

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if (board[i][j] == EMPTY) {
                futures.emplace_back(std::async(std::launch::async, [&board, i, j] {
                    std::vector<std::vector<char>> newBoard = board;
                    newBoard[i][j] = O;
                    int moveVal = minimax(newBoard, 0, false, INT_MIN, INT_MAX);
                    return std::make_tuple(moveVal, i, j);
                }));
            }
        }
    }

    for (auto& f : futures) {
        auto [moveVal, row, col] = f.get();
        if (moveVal > globalBestVal) {
            globalBestVal = moveVal;
            bestMoveRow = row;
            bestMoveCol = col;
        }
    }

    if (bestMoveRow != -1) {
        board[bestMoveRow][bestMoveCol] = O;
    }
}

int main() {
    int size;
    std::cout << "Enter the field size: ";
    std::cin >> size;

    USE_CACHE = (size > 5);

    std::cout << "Enter the difficulty for the bot (1 - 10): ";
    std::cin >> MAX_DEPTH;
    MAX_DEPTH = (MAX_DEPTH > 10) ? 10 : (MAX_DEPTH < 1 ? 1 : MAX_DEPTH);

    std::vector<std::vector<char>> board = createBoard(size);

    while (true) {
        printBoard(board);
        if (!isBoardFull(board)) {
            playerMove(board);
            if (isWinner(board, X)) {
                std::cout << "Congratulations! You've won!\n";
                break;
            }
        }

        if (!isBoardFull(board)) {
            botMove(board);
            if (isWinner(board, O)) {
                std::cout << "Bot won. Try again!\n";
                break;
            }
        }

        if (isBoardFull(board)) {
            std::cout << "Draw!\n";
            break;
        }
    }

    printBoard(board);
    return 0;
}
