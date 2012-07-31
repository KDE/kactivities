#include <memory>
#include <string>
#include <iostream>
#include <utility>

struct Question {
    long answer;
    std::string description;
};

int main()
{
    std::unique_ptr < Question > node_original(new Question());

    node_original->answer = 42;
    node_original->description = "The Answer to the Ultimate Question of Life, the Universe, and Everything";

    std::unique_ptr < Question > node_second(std::move(node_original));

    return (!node_original && (node_second->answer == 42))?0:1;
}
