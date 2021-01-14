#include <iostream>
#include "bot.h"

int main(int argc, char* argv[]) {
    std::string for_tests = "http://35.207.130.78/";
    std::string for_real_tg = "https://api.telegram.org/";
    std::string token_for_tests = "token for fake server";
    std::string token_for_tg = "token, that was given by TG";
    TelegramBot my_bot(token_for_tg, for_real_tg);
    my_bot.Start();
    return 0;
}