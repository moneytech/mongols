#ifndef BBADA2A5_79C2_4376_B7F1_FA0FFD05965A
#define BBADA2A5_79C2_4376_B7F1_FA0FFD05965A

#include <list>
#include <mutex>
#include <queue>

namespace mongols {
class id {
public:
    id() = default;
    virtual ~id() = default;
    void push(size_t);
    size_t poll();

private:
    static size_t data;
    static std::mutex mtx;
    static std::queue<size_t, std::list<size_t>> data_pool;
};
}

#endif /* BBADA2A5_79C2_4376_B7F1_FA0FFD05965A */
