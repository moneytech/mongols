#include <mongols/lib/hash/hash_engine.hpp>
#include <mongols/lua_server.hpp>
#include <mongols/util.hpp>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>

class person {
public:
    person()
        : name("Tom")
        , age(0)
    {
    }
    virtual ~person() = default;

    void set_name(const std::string& name)
    {
        this->name = name;
    }

    void set_age(unsigned int age)
    {
        this->age = age;
    }

    const std::string& get_name() const
    {
        return this->name;
    }

    unsigned int get_age()
    {
        return this->age;
    }

private:
    std::string name;
    unsigned int age;
};

class studest : public person {
public:
    studest()
        : person()
    {
    }
    virtual ~studest() = default;

    double get_score()
    {
        return this->score;
    }

    void set_score(double score)
    {
        this->score = score;
    }

private:
    double score;
};

int main(int, char**)
{
    //    daemon(1, 0);

    int port = 9090;
    const char* host = "127.0.0.1";
    mongols::lua_server
        server(host, port, 5000, 8192, 0 /*2*/);
    server.set_root_path("html/lua");
    server.set_enable_bootstrap(true);
    server.set_enable_lru_cache(true);
    server.set_lru_cache_expires(1);
    //    if (!server.set_openssl("openssl/localhost.crt", "openssl/localhost.key")) {
    //        return -1;
    //    }

     server.set_function([](const std::string& str) {
        return mongols::hash_engine::sha1(str);
    },
        "sha1");
    server.set_function([](const std::string& str) {
        return mongols::hash_engine::md5(str);
    },
        "md5");

    server.set_class(
        kaguya::UserdataMetatable<person>()
            .setConstructors<person()>()
            .addFunction("get_age", &person::get_age)
            .addFunction("get_name", &person::get_name)
            .addFunction("set_age", &person::set_age)
            .addFunction("set_name", &person::set_name),
        "person");

    server.set_class(
        kaguya::UserdataMetatable<studest, person>()
            .setConstructors<studest()>()
            .addFunction("get_score", &studest::get_score)
            .addFunction("set_score", &studest::set_score),
        "studest");

    std::function<void(pthread_mutex_t*, size_t*)> f = [&](pthread_mutex_t* mtx, size_t* data) {
        std::string i;
        pthread_mutex_lock(mtx);
        if (*data > std::thread::hardware_concurrency() - 1) {
            *data = 0;
        }
        i = std::move(std::to_string(*data));
        *data = (*data) + 1;
        pthread_mutex_unlock(mtx);
        server.set_db_path("html/leveldb/" + i);
        server.set_shutdown([&, i]() {
            std::cout << i << "\tprocess\t" << getpid() << "\texit.\n";
        });
        server.run("html/lua/package/?.lua;", "html/lua/package/?.so;");
    };

    std::function<bool(int)> g = [&](int status) {
        return false;
    };

    mongols::multi_process main_process;
    main_process.run(f, g);
}
