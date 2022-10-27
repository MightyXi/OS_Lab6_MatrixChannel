#ifndef BUFFERED_CHANNEL_H_
#define BUFFERED_CHANNEL_H_

#include <queue>
#include <mutex>
#include <condition_variable>

std::mutex write_mutex;
std::mutex read_mutex;
std::condition_variable write_variable;
std::condition_variable read_variable;
bool can_write = true;
bool can_read = true;
bool closed = false;


template<class T>
class Channel {
private:
    const int buffer_size;
    std::queue<T> buff;

public:
    explicit Channel(int size) : buffer_size(size) {
        can_write = buff.size() < buffer_size;
        can_read = !buff.empty();
    }

    void Send(T value) {
        if(closed)
            throw std::runtime_error("The channel is closed.");
        std::unique_lock<std::mutex> lock(write_mutex);
        while(!can_write)
            write_variable.wait(lock);
        buff.push(value);
        can_write = buff.size() < buffer_size;
        can_read = !buff.empty();
        write_variable.notify_one();
        read_variable.notify_one();
    }

    std::pair<T, bool> Recv() {
        if(closed && buff.empty())
            return {T(), false};
        std::unique_lock<std::mutex> lock(read_mutex);
        while(!can_read)
            read_variable.wait(lock);
        if(buff.empty())
            return {T(), false};
        T value = buff.front();
        buff.pop();
        can_write = buff.size() < buffer_size;
        can_read = !buff.empty();
        write_variable.notify_one();
        read_variable.notify_one();
        return {value, true};
    }

    void Close() {
        closed = true;
        read_variable.notify_one();
        write_variable.notify_one();
    }
};

#endif // BUFFERED_CHANNEL_H_