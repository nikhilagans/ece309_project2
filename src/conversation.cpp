#include <stdexcept>
#include "core/conversation.h"

    // Empty conversation: size() == 0, no allocation yet.
    Conversation::Conversation() = default;

    // Releases all owned Message storage. No effect if already empty
    // (e.g. moved-from).
    Conversation::~Conversation(){
        delete[] data_;
    }

    // Deep copy: allocates its own buffer and copies every Message.
    // this->begin() must differ from other.begin() afterward.
    Conversation::Conversation(const Conversation& other){
        size_ = other.size_;
        capacity_ = other.capacity_;
        data_ = new Message[capacity_];

        for(std::size_t i = 0; i < size_; i++){
            data_[i] = other.data_[i];
        }
    }
    Conversation& Conversation::operator=(const Conversation& other){
        if(this == &other){return *this;}
        delete [] data_;
        size_ = other.size_;
        capacity_ = other.capacity_;

        data_ = new Message[capacity_];
        for(std::size_t i = 0; i < size_; i++){
            data_[i] = other.data_[i];
        }
        return *this;
    }

    // Steals other's buffer — no per-element copying. Afterward, other
    // must be left valid and empty (safe to destroy or reassign).
    Conversation::Conversation(Conversation&& other) noexcept{
        size_ = other.size_;
        capacity_ = other.capacity_;
        data_ = other.data_;
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }
    Conversation& Conversation::operator=(Conversation&& other) noexcept{
        if(this == &other){
            return *this;
        }
        delete [] data_;
        data_ = other.data_;
        size_ = other.size_;
        capacity_ = other.capacity_;

        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;

        return *this;
    }

    // Appends m, growing the backing array if needed. Amortized O(1) —
    // document and justify your growth strategy in the design log
    // (see Appendix C if you want a refresher first).
    void Conversation::append(Message m){
        if (size_ == capacity_)
        {
            if(capacity_ == 0){
                capacity_ = 1;
            }
            else{
                capacity_ *= 2; //grow by a multiple of 2 for amortized O(1)
            }
            Message *old_data = data_;
            data_ = new Message[capacity_];

            for(std::size_t i = 0; i < size_; i++){
                data_[i] = old_data[i];
            }
            delete [] old_data;
        }
        data_[size_] = m;
        size_ += 1;    
    }

    // Number of messages currently stored.
    std::size_t Conversation::size() const noexcept{
        return size_;
    }

    // Bounds-checked access. Decide what happens on i >= size() (throw,
    // assert, whatever you pick) and test that behavior explicitly.
    const Message& Conversation::at(std::size_t i) const{
        if(i >= size_){
            throw std::out_of_range("Conversation index is out of range");
        }
        return(data_[i]);
    }

    // Range-for iteration, oldest message first. begin() == end() when
    // size() == 0.
    const Message* Conversation::begin() const noexcept{
        return (data_);
    }//first index
    const Message* Conversation::end() const noexcept{
        if(size_ == 0){
            return (data_);
        }
        return (data_ + size_);
    }//one past the last message