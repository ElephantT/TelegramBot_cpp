#include "catch.hpp"
#include "telegram/fake.h"
#include "telegram/client.h"
#include "fstream"

void ClearOffsetBetweenTests() {
    // чищу оффсет после каждого теста, потому что если запускать их по очереди, то после 3го теста
    // оффсет из 3го передаётся 4му, и он падает, как я понимаю если прописать для всех сценариев
    // последовательные update_id, всё тоже будет хорошо работать, но два запуска test_telegram
    // подряд тоже упадут, ибо нужно будет чистить между ними файл
    std::string file_offset_name = "offset.txt";
    std::ofstream file_input(file_offset_name);
    if (file_input.is_open()) {
        try {
            file_input << 0;
        } catch (...) {
        }
    }
}

TEST_CASE("Single getMe") {
    telegram::FakeServer fake("Single getMe");
    fake.Start();
    auto host = fake.GetUrl();
    auto token = "123";
    ClientTelegramBotAPI client(token, host);
    REQUIRE(client.GetMe());
    fake.StopAndCheckExpectations();

    ClearOffsetBetweenTests();
}

TEST_CASE("getMe error handling") {
    telegram::FakeServer fake("getMe error handling");
    fake.Start();
    auto host = fake.GetUrl();
    auto token = "123";

    ClientTelegramBotAPI client(token, host);
    REQUIRE_THROWS_AS(client.GetMe(), std::runtime_error);
    REQUIRE_THROWS_AS(client.GetMe(), std::runtime_error);
    fake.StopAndCheckExpectations();

    ClearOffsetBetweenTests();
}

TEST_CASE("Single getUpdates and send messages") {
    telegram::FakeServer fake("Single getUpdates and send messages");
    fake.Start();
    auto host = fake.GetUrl();
    auto token = "123";

    ClientTelegramBotAPI client(token, host);
    std::vector<std::shared_ptr<AbstractCPPClass>> updates = client.GetUpdates();
    auto update = updates[0];
    std::shared_ptr<NewMessage> received_message = std::dynamic_pointer_cast<NewMessage>(update);
    if (received_message != nullptr) {
        int64_t chat_id = received_message->GetChatId();
        client.SendMessage(chat_id, "Hi!");
    }
    update = updates[1];
    received_message = std::dynamic_pointer_cast<NewMessage>(update);
    if (received_message != nullptr) {
        int64_t chat_id = received_message->GetChatId();
        int64_t message_id = received_message->GetMessageId();
        client.SendMessage(chat_id, "Reply", message_id);
        client.SendMessage(chat_id, "Reply", message_id);
    }
    int chat_id = std::dynamic_pointer_cast<NewMessage>(updates[2])->GetChatId();
    (void)chat_id;

    fake.StopAndCheckExpectations();

    ClearOffsetBetweenTests();
}

TEST_CASE("Handle getUpdates offset") {
    telegram::FakeServer fake("Handle getUpdates offset");
    fake.Start();
    auto host = fake.GetUrl();
    auto token = "123";

    ClientTelegramBotAPI client(token, host);
    auto updates = client.GetUpdates(5);
    REQUIRE(updates.size() == 2);

    updates = client.GetUpdates(5);
    REQUIRE(updates.empty());
    updates = client.GetUpdates(5);
    REQUIRE(updates.size() == 1);

    fake.StopAndCheckExpectations();

    ClearOffsetBetweenTests();
}
