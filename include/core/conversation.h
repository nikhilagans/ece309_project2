#pragma once
#include "core/message.h"
#include <cstddef>

class Conversation {
public:
    Conversation(); //default constructor 
    ~Conversation(); //destructor 

    Conversation(const Conversation& other); //Deep copy constructor 
    Conversation& operator=(const Conversation& other); //copy assignment constructor

    Conversation(Conversation&& other) noexcept; // move constructor
    Conversation& operator=(Conversation&& other) noexcept; //move assignment constructor

    void append(Message m); //append conversation function

    std::size_t size() const noexcept; //give size of conversation

    const Message& at(std::size_t i) const; //bound check function throws and out of range error

    const Message* begin() const noexcept; //returns a pointer to the first index of the conversation
    const Message* end() const noexcept; //returns a pointer to one past the last index of the conversation
    //purpose of this is to be able to properly iterate through the array

private:
    Message*    data_ = nullptr;
    std::size_t size_ = 0;
    std::size_t capacity_ = 0;
};