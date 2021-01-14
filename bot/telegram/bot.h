// #pragma once
#include <random>
#include <chrono>

class TelegramBot {
public:
    TelegramBot(const std::string& token, const std::string& uri);

    void Start();

    int64_t RandomNumberResponse();
    // слуайное число, ну что, наконец-то ещё где-то заюзаю chrono.time?)

    std::string WeatherNumberResponse();
    // Winter Is Coming - ответ

    std::string StyleguideResponse();
    // шутка про код ревью в ответ

    void Stop();
    // штатно завершить работу бота (нормально)

    void Crash();
    // аварийно завершить работу бота (выключили свет) - abort()

private:
    std::mt19937_64 generator_;
    const std::string token_;
    const std::string uri_;
    std::string offset_file_name_;
};
