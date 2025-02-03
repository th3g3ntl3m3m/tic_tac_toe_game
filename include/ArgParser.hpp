#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

class ArgParser {
public:
    ArgParser(int argc, char* argv[]) {
        parse(argc, argv);
    }

    const std::unordered_map<std::string, std::string>& getArgs() const {
        return args;
    }

    std::string get(const std::string& key, const std::string& defaultValue = "") const {
        auto it = args.find(key);
        return (it != args.end()) ? it->second : defaultValue;
    }

    bool hasArgument(const std::string& key) const {
        return args.find(key) != args.end();
    }

    template<typename T>
    T getAs(const std::string& key, const T& defaultValue = T()) const {
        auto it = args.find(key);
        if (it != args.end()) {
            std::istringstream iss(it->second);
            T value;
            if (iss >> value)
                return value;
        }
        return defaultValue;
    }

private:
    std::unordered_map<std::string, std::string> args;

    void parse(int argc, char* argv[]) {
        std::vector<std::string> tokens;
        for (int i = 1; i < argc; i++) {
            tokens.push_back(argv[i]);
        }

        for (size_t i = 0; i < tokens.size(); i++) {
            const std::string& token = tokens[i];
            auto pos = token.find('=');
            if (pos != std::string::npos) {
                std::string key = token.substr(0, pos);
                std::string value = token.substr(pos + 1);
                args[key] = value;
            } else {
                std::string key = token;
                std::string value;
                if (i + 1 < tokens.size() && tokens[i + 1].find('=') == std::string::npos) {
                    value = tokens[++i];
                } else {
                    value = "true";
                }
                args[key] = value;
            }
        }
    }
};