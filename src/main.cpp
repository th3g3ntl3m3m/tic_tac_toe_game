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
#include "ICache.hpp"

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

namespace GameRules {
    bool isWinner(const Board& board, char player) {
        int size = board.getSize();
        bool diagonalWin = true, reverseDiagonalWin = true;
        for (int i = 0; i < size; i++) {
            if (board.getCell(i, i) != player) {
                diagonalWin = false;
            }
            if (board.getCell(i, size - i - 1) != player) {
                reverseDiagonalWin = false;
            }

            bool rowWin = true, colWin = true;
            for (int j = 0; j < size; j++) {
                if (board.getCell(i, j) != player) {
                    rowWin = false;
                }
                if (board.getCell(j, i) != player) {
                    colWin = false;
                }
            }
            if (rowWin || colWin) {
                return true;
            }
        }
        return diagonalWin || reverseDiagonalWin;
    }
}

//==================
// Engine
//==================

template <typename MoveType>
class IEngine {
public:
    virtual ~IEngine() = default;
    
    virtual MoveType getBestMove(Board& board) = 0;
};

class AIEngine : public IEngine<std::tuple<int,int>> {
public:
    AIEngine(int maxDepth, std::unique_ptr<SKW_WPX::Cache::ICache> cachePtr)
        : MAX_DEPTH(maxDepth), cache(std::move(cachePtr)) {}
    
    int minimax(Board& board, int depth, bool isMax, int alpha, int beta) {
        std::string boardKey = board.toString() + (isMax ? "1" : "0") + std::to_string(depth);
        
        if (cache) {
            std::optional<int> cachedValue = cache->get(boardKey);
            if (cachedValue.has_value())
                return cachedValue.value();
        }

        if (depth >= MAX_DEPTH) {
            return 0;
        }
        if (GameRules::isWinner(board, 'O')) {
            return 10 - depth;
        }
        if (GameRules::isWinner(board, 'X')) {
            return -10 + depth;
        }
        if (board.isFull()) {
            return 0;
        }

        int best = isMax ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
        int size = board.getSize();

        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                if (board.getCell(i, j) == '.') {
                    board.setCell(i, j, isMax ? 'O' : 'X');
                    int value = minimax(board, depth + 1, !isMax, alpha, beta);
                    board.setCell(i, j, '.');
                    if (isMax) {
                        best = std::max(best, value);
                        alpha = std::max(alpha, best);
                    } else {
                        best = std::min(best, value);
                        beta = std::min(beta, best);
                    }
                    if (beta <= alpha)
                        break;
                }
            }
        }

        if (cache) {
            cache->put(boardKey, best);
        }
        return best;          
    }

    std::tuple<int, int> getBestMove(Board& board) override {
        int size = board.getSize();
        int globalBestVal = std::numeric_limits<int>::min();
        int bestRow = -1, bestCol = -1;
        std::vector<std::future<std::tuple<int, int, int>>> futures;

        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                if (board.getCell(i, j) == '.') {
                    futures.emplace_back(std::async(std::launch::async, [this, &board, i, j]() {
                        Board newBoard = board;
                        newBoard.setCell(i, j, 'O');
                        int moveVal = minimax(newBoard, 0, false,
                                              std::numeric_limits<int>::min(),
                                              std::numeric_limits<int>::max());
                        return std::make_tuple(moveVal, i, j);
                    }));
                }
            }
        }

        for (auto& f : futures) {
            auto [moveVal, row, col] = f.get();
            if (moveVal > globalBestVal) {
                globalBestVal = moveVal;
                bestRow = row;
                bestCol = col;
            }
        }
        return std::make_tuple(bestRow, bestCol);
    }

private:
    int MAX_DEPTH;
    std::unique_ptr<SKW_WPX::Cache::ICache> cache;
};

//==================
// IPlayer
//==================

class IPlayer {
public:
    virtual ~IPlayer() = default;
    virtual void makeMove(Board& board) = 0;
    virtual char getSymbol() const = 0;
};

//==================
// View
//==================

class IView {
public:
    virtual ~IView() = default;
    virtual void displayBoard(const Board& board) = 0;
};

class ConsoleView : public IView {
public:
    void displayBoard(const Board& board) override {
        board.print();
    }
    // todo add methods for displaying messages, receiving input
};

//==================
// Players
//==================

class HumanPlayer : public IPlayer {
public:
    HumanPlayer(char symbol, IView* view) : symbol(symbol), view(view) {}

    void makeMove(Board& board) override {
        int x, y;
        while (true) {
            std::cout << "Enter the coordinates of your move (row and column): ";
            std::cin >> x >> y;
            x--; y--;
            if (x >= 0 && x < board.getSize() && y >= 0 && y < board.getSize() && board.getCell(x, y) == '.') {
                board.setCell(x, y, symbol);
                break;
            }
            std::cout << "Invalid move. Try again.\n";
        }
    }

    char getSymbol() const override {
        return symbol;
    }

private:
    char symbol;
    IView* view;
};

class BotPlayer : public IPlayer {
public:
    BotPlayer(char symbol, int difficulty, std::unique_ptr<SKW_WPX::Cache::ICache> cachePtr)
        : symbol(symbol) { engine = std::make_unique<AIEngine>(difficulty, std::move(cachePtr)); }

    BotPlayer(char symbol, int difficulty, bool useCache, size_t cacheSize = 1024) 
        : symbol(symbol) 
    { 
        engine = std::make_unique<AIEngine>(
            difficulty,
            useCache ? std::make_unique<SKW_WPX::Cache::LRUCache>(cacheSize) : nullptr
        );
    }

    void makeMove(Board& board) override {
        std::cout << "Bot is thinking...\n";
        auto [row, col] = engine->getBestMove(board);
        if (row != -1 && col != -1) {
            board.setCell(row, col, symbol);
        }
    }

    char getSymbol() const override {
        return symbol;
    }

private:
    char symbol;
    std::unique_ptr<IEngine<std::tuple<int,int>>> engine;
};

class RemotePlayer : public IPlayer {
public:
    RemotePlayer(char symbol) : symbol(symbol) {}
    void makeMove(Board& board) override {
        // todo logic for getting a move through the network
        std::cout << "Remote move received (stub).\n";
        int x, y;
        std::cout << "Enter remote move coordinates (row and column): ";
        std::cin >> x >> y;
        board.setCell(x - 1, y - 1, symbol);
    }
    char getSymbol() const override {
        return symbol;
    }
private:
    char symbol;
};

//==================
// Controller
//==================

class GameController {
public:
    GameController(std::unique_ptr<IPlayer> player1,
                   std::unique_ptr<IPlayer> player2,
                   std::unique_ptr<IView> view,
                   int boardSize)
        : player1(std::move(player1)),
          player2(std::move(player2)),
          view(std::move(view)),
          board(boardSize) {}

    void runGameLoop() {
        bool gameOver = false;
        IPlayer* currentPlayer = player1.get();
        while (!gameOver) {
            view->displayBoard(board);
            std::cout << "Player " << currentPlayer->getSymbol() << "'s turn.\n";
            currentPlayer->makeMove(board);

            if (GameRules::isWinner(board, currentPlayer->getSymbol())) {
                view->displayBoard(board);
                std::cout << "Player " << currentPlayer->getSymbol() << " wins!\n";
                gameOver = true;
            } else if (board.isFull()) {
                view->displayBoard(board);
                std::cout << "Draw!\n";
                gameOver = true;
            }
            currentPlayer = (currentPlayer == player1.get()) ? player2.get() : player1.get();
        }
    }

private:
    std::unique_ptr<IPlayer> player1;
    std::unique_ptr<IPlayer> player2;
    std::unique_ptr<IView> view;
    Board board;
};

int main() {
    int boardSize;
    std::cout << "Enter the board size: ";
    std::cin >> boardSize;

    bool useCache = (boardSize > 5);

    int difficulty;
    std::cout << "Enter bot difficulty (1-10): ";
    std::cin >> difficulty;
    if(difficulty < 1)
        difficulty = 1;
    else if(difficulty > 10)
        difficulty = 10;

    auto view = std::make_unique<ConsoleView>();
    auto human = std::make_unique<HumanPlayer>('X', view.get());
    auto bot = std::make_unique<BotPlayer>('O', difficulty, useCache);

    GameController controller(std::move(human), std::move(bot), std::move(view), boardSize);
    controller.runGameLoop();

    return 0;
}