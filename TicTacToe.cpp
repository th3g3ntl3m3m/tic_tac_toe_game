#include <iostream>
#include <vector>
#include <climits>
#include <map>
#include <unordered_map>
#include <string>
#include <list>
#include <algorithm>
#include <thread>
#include <mutex>
#include <future>

const char X = 'X';
const char O = 'O';
const char EMPTY = '.';
int MAX_DEPTH;
bool USE_CACHE;
size_t MAX_CACHE_SIZE;
bool USE_LRU_CACHE;

std::map<std::string, int> memo;
std::list<std::string> cacheKeys;

struct LRUCache {
    std::list<std::string> keys;
    std::unordered_map<std::string, std::pair<int, std::list<std::string>::iterator>> map;

    void put(const std::string& key, int value) {
        if (map.find(key) != map.end()) {
            keys.erase(map[key].second);
        } else if (keys.size() >= MAX_CACHE_SIZE) {
            map.erase(keys.back());
            keys.pop_back();
        }
        keys.push_front(key);
        map[key] = {value, keys.begin()};
    }

    bool get(const std::string& key, int& value) {
        if (map.find(key) == map.end()) {
            return false;
        }
        keys.erase(map[key].second);
        keys.push_front(key);
        map[key].second = keys.begin();
        value = map[key].first;
        return true;
    }
};

LRUCache cache;
std::mutex cacheMutex;

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
        std::cout << "Введите координаты хода (строка и столбец): ";
        std::cin >> x >> y;
        x--; y--;
        if (x >= 0 && x < board.size() && y >= 0 && y < board.size() && board[x][y] == EMPTY) {
            board[x][y] = X;
            break;
        }
        std::cout << "Неверный ход. Попробуйте снова.\n";
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
        std::lock_guard<std::mutex> lock(cacheMutex);
        if (USE_LRU_CACHE) {
            int cachedValue;
            if (cache.get(boardKey, cachedValue)) {
                return cachedValue;
            }
        }
        else {
            if (memo.find(boardKey) != memo.end()) return memo[boardKey];
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
                int val = minimax(board, depth + 1, !isMax, alpha, beta); // todo нужна многопочка при сильной вложенности долго считает -> породить очередь из тасок на вычисление с ограничением числа потоков, писал это в MULT
                board[i][j] = EMPTY;
                best = isMax ? std::max(best, val) : std::min(best, val);
                if (isMax) alpha = std::max(alpha, best);
                else beta = std::min(beta, best);
                if (beta <= alpha) break;
            }
        }
    }

    if (USE_CACHE) {
        std::lock_guard<std::mutex> lock(cacheMutex);
        if (USE_LRU_CACHE) {
            cache.put(boardKey, best);
        }
        else {
            if (memo.size() >= MAX_CACHE_SIZE) {
                memo.erase(cacheKeys.front());
                cacheKeys.pop_front();
            }

            memo[boardKey] = best;
            cacheKeys.push_back(boardKey);
        }
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
    std::cout << "Введите размер поля: ";
    std::cin >> size;
    std::cout << "Введите сложность для бота (1 - 10): ";
    std::cin >> MAX_DEPTH;

    MAX_DEPTH = MAX_DEPTH > 10 ? 10 : MAX_DEPTH < 1 ? 1 : MAX_DEPTH;

    char useCache;
    std::cout << "Использовать кэш? (y/n): ";
    std::cin >> useCache;
    USE_CACHE = (useCache == 'y' || useCache == 'Y');

    if (USE_CACHE) {
        double sizeInMB;
        std::cout << "Введите размер кэша в мегабайтах: ";
        std::cin >> sizeInMB;
        MAX_CACHE_SIZE = static_cast<size_t>((sizeInMB * 1024 * 1024) / 100);

        char useLRUCache;
        std::cout << "Использовать LRU кэш? (y/n): ";
        std::cin >> useLRUCache;
        USE_LRU_CACHE = (useLRUCache == 'y' || useLRUCache == 'Y');
    }

    std::vector<std::vector<char>> board = createBoard(size);

    while (true) {
        printBoard(board);
        if (!isBoardFull(board)) {
            playerMove(board);
            if (isWinner(board, X)) {
                std::cout << "Поздравляем! Вы выиграли!\n";
                break;
            }
        }

        if (!isBoardFull(board)) {
            botMove(board);
            if (isWinner(board, O)) {
                std::cout << "Бот выиграл. Попробуйте снова!\n";
                break;
            }
        }

        if (isBoardFull(board)) {
            std::cout << "Ничья!\n";
            break;
        }
    }

    printBoard(board);
    return 0;
}
