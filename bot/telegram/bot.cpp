#include "bot.h"
#include "client.h"
#include <variant>

TelegramBot::TelegramBot(const std::string& token, const std::string& uri)
    : token_(std::move(token)), uri_(std::move(uri)) {
    generator_ = std::mt19937_64(std::chrono::system_clock::now().time_since_epoch().count());
}

void TelegramBot::Start() {
    ClientTelegramBotAPI my_client_for_tg_api(token_, uri_);
    int i = 0;
    bool check = true;
    while (check) {
        check = true;
        ++i;
        std::vector<std::shared_ptr<AbstractCPPClass>> updates = my_client_for_tg_api.GetUpdates(5);
        for (auto update : updates) {
            // проверка, если произошедшее событие, это приход нового сообщения
            std::shared_ptr<NewMessage> received_message = std::dynamic_pointer_cast<NewMessage>(update);
            if (received_message != nullptr) {
                int64_t chat_id = received_message->GetChatId();
                if (received_message->AreCommandsInText()) {
                    for (auto& command : received_message->Commands()) {
                        if (command == "/random") {
                            my_client_for_tg_api.SendMessage(chat_id,
                                                             std::to_string(RandomNumberResponse()));
                        } else if (command == "/weather") {
                            my_client_for_tg_api.SendMessage(chat_id, WeatherNumberResponse());
                        } else if (command == "/styleguide") {
                            my_client_for_tg_api.SendMessage(chat_id, StyleguideResponse());
                        } else if (command == "/stop") {
                            Stop();
                        } else if (command == "/crash") {
                            Crash();
                        }
                    }
                }
            }
        }
    }
}

int64_t TelegramBot::RandomNumberResponse() {
    return generator_();
}

std::string TelegramBot::WeatherNumberResponse() {
    return "Winter Is Coming";
}

std::string TelegramBot::StyleguideResponse() {
    return "Here is supposed to be a code review joke";
}

void TelegramBot::Stop() {
    exit(0);
}

void TelegramBot::Crash() {
    abort();
}

