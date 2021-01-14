
set(SOLUTION_SRC)
set(SOLUTION_TEST_SRC test/test_api.cpp)
if (TEST_SOLUTION)
  include_directories(../private/bot)
  set(SOLUTION_SRC ../private/bot/telegram/api.cpp telegram/client.cpp telegram/client.h telegram/bot.cpp telegram/bot.h)
  set(SOLUTION_TEST_SRC ../private/bot/test/test_api.cpp)
endif()

if (TEST_SOLUTION)
  include_directories(/usr/include/jsoncpp)
elseif (APPLE)
  link_directories(/usr/local/lib)
  link_directories(/usr/local/opt/openssl/lib)

  include_directories(/usr/local/include)
  include_directories(/usr/local/opt/openssl/include)
endif()

add_library(telegram
  ${SOLUTION_SRC}
  telegram/fake.cpp
  telegram/fake_data.cpp
  telegram/client.cpp
  telegram/bot.cpp)

target_include_directories(telegram PUBLIC .)

target_link_libraries(telegram
  PocoNet
  PocoNetSSL
  PocoFoundation
  PocoJSON)

if (TEST_SOLUTION)
  target_link_libraries(telegram jsoncpp)
  add_executable(bot-run
    ../private/bot/telegram/main.cpp)
else()
  add_executable(bot-run
    telegram/main.cpp)
endif()

target_link_libraries(bot-run
  telegram)

add_executable(fake
  telegram/fake_main.cpp)

target_link_libraries(fake
  telegram)

# Add test files here

add_catch(test_telegram ${SOLUTION_TEST_SRC})

target_link_libraries(test_telegram
  telegram)
