// #pragma once
#include <stdexcept>
#include <optional>
#include <string>
#include <variant>
#include <vector>
#include <queue>
#include <memory>

struct TelegramAPIError;

class AbstractCPPClass {
public:
    virtual ~AbstractCPPClass() = default;
};

class NewMessage : public AbstractCPPClass {
public:
    NewMessage(int64_t chat_id, int64_t message_id, std::string text,
               std::vector<std::string> commands);

    bool AreCommandsInText();
    std::vector<std::string>& Commands();
    std::string GetText();
    int64_t GetChatId();
    void SetCommandsChecker(bool are_there_commands);
    int64_t GetMessageId();

private:
    int64_t chat_id_;
    int64_t message_id_;

    std::string text_;
    std::vector<std::string> commands_;
    bool are_there_commands_;
};
// если мы хотим, чтобы наш API работал ещё с какими-то командами, запросами или ещё чем,
// то нужно создать классы с++ и отнаследовать их к JsonToCPP
// дальше работаем со smart-ptr от них

class ClientTelegramBotAPI {
public:
    ClientTelegramBotAPI(const std::string& token, const std::string& uri);

    ~ClientTelegramBotAPI() = default;

    bool GetMe();

    std::vector<std::shared_ptr<AbstractCPPClass>> GetUpdates(int timeout = 0);

    void SendMessage(int64_t chat_id, std::string response, int64_t message_id = -1);
    // отправляет ответ бота на один из полученных апдейтов из getUpdate
    // используем шаблон, смотрим что там может быть, делаем по нему Json и отправляем на сервер

private:
    const std::string token_;
    const std::string uri_;
    int64_t offset_;
    std::string offset_file_name_;

    std::vector<std::shared_ptr<AbstractCPPClass>> FormCppStructFromJson(std::istream& response_body);
    // делает с++ структуру из json, берём один объект из getUpdates

    void SetOffset();
    // изменить offset в файле
    void GetOffset();
    // получить offset из файла
};

struct TelegramAPIError : public std::runtime_error {
    TelegramAPIError(int error_code, const std::string& details)
        : std::runtime_error("api error: code=" + std::to_string(error_code) +
                             " details=" + details),
          http_code(error_code),
          details(details) {
    }

    int http_code;
    std::string details;
};
