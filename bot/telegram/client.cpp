#include "client.h"

#include <Poco/SharedPtr.h>
#include <Poco/URI.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
// для одновременной поддержки https и http
#include <Poco/Net/HTTPSessionFactory.h>
#include <Poco/Net/HTTPSSessionInstantiator.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/Net/AcceptCertificateHandler.h>

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <assert.h>
#include <fstream>

NewMessage::NewMessage(int64_t chat_id, int64_t message_id, std::string text,
                       std::vector<std::string> commands)
    : chat_id_(chat_id), message_id_(message_id), text_(text), commands_(std::move(commands)) {
}

bool NewMessage::AreCommandsInText() {
    return are_there_commands_;
}

void NewMessage::SetCommandsChecker(bool are_there_commands) {
    are_there_commands_ = are_there_commands;
}

std::vector<std::string>& NewMessage::Commands() {
    return commands_;
}

std::string NewMessage::GetText() {
    return text_;
}

int64_t NewMessage::GetChatId() {
    return chat_id_;
}

int64_t NewMessage::GetMessageId() {
    return message_id_;
}

ClientTelegramBotAPI::ClientTelegramBotAPI(const std::string& token, const std::string& uri)
    : token_(token), uri_(uri) {
    offset_file_name_ = "offset.txt";
    GetOffset();
/*
    // для одновременной поддержки https и http (код из stackoverflow)
    Poco::Net::HTTPSessionFactory::defaultFactory().registerProtocol(
        "http", new Poco::Net::HTTPSessionInstantiator);
    Poco::Net::HTTPSessionFactory::defaultFactory().registerProtocol(
        "https", new Poco::Net::HTTPSSessionInstantiator);
    //prepare for SSLManager
    Poco::SharedPtr ptr_cert = new Poco::Net::AcceptCertificateHandler(false);
    const Poco::Net::Context::Ptr context = new Poco::Net::Context(
        Poco::Net::Context::CLIENT_USE, "", "", "",
        Poco::Net::Context::VERIFY_NONE, 9, false,
        "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
    Poco::Net::SSLManager::instance().initializeClient(0, ptr_cert, context);*/
}

std::vector<std::shared_ptr<AbstractCPPClass>> ClientTelegramBotAPI::FormCppStructFromJson(std::istream&
                                                                                  response_body) {
    std::vector<std::shared_ptr<AbstractCPPClass>> answer;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var reply = parser.parse(response_body);

        Poco::JSON::Object::Ptr responsed_answer = reply.extract<Poco::JSON::Object::Ptr>();
        bool checker = responsed_answer->getValue<bool>("ok");
        // SendMessage(400988361, "passed ok");
        if (!checker) {
            throw TelegramAPIError(111, "my responsed ok = false");
            //                responsed_answer->getValue<std::string>("description")
        }

        Poco::JSON::Array::Ptr array_of_updates = responsed_answer->getArray("result");
        for (size_t idx = 0; idx < array_of_updates->size(); ++idx) {
            // SendMessage(400988361, "entered cycle" + std::to_string(idx));
            Poco::JSON::Object::Ptr cur_obj = array_of_updates->getObject(idx);
            offset_ = cur_obj->getValue<int64_t>("update_id") + 1;
            SetOffset();
            if (!cur_obj->has("message")) {
                continue;
            }
            // SendMessage(400988361, "passed check is it message update");
            std::string text = "";
            if ((cur_obj->getObject("message")->has("text"))) {
                text = cur_obj->getObject("message")->
                                   getValue<std::string>("text");
            }
            int64_t chat_id = 0;
            if ((cur_obj->getObject("message")->has("chat"))) {
                chat_id = cur_obj->getObject("message")->getObject("chat")->
                    getValue<int64_t>("id");
            }
            int64_t message_id = 0;
            if ((cur_obj->getObject("message")->has("message_id"))) {
                message_id = cur_obj->getObject("message")->
                                     getValue<int64_t>("message_id");
            }
            std::vector<std::string> commands;
            if (!(cur_obj->getObject("message")->has("entities")) ||
                !(cur_obj->getObject("message")->has("text"))) {
                answer.push_back(std::make_shared<NewMessage>(NewMessage(chat_id, message_id,
                                                                         text, commands)));
                continue;
            }
            // SendMessage(400988361, "passed entities checker");
            for (size_t idx_command = 0; idx_command < cur_obj->getObject("message")->
                                                       getArray("entities")->size();
                    ++idx_command) {
                if (cur_obj->getObject("message")->getArray("entities")->
                        getObject(idx_command)->getValue<std::string>("type") == "bot_command") {
                    int64_t index_of_command_in_text =
                        cur_obj->getObject("message")->getArray("entities")->
                            getObject(idx_command)->getValue<int64_t>("offset");
                    int64_t len_of_command_in_text =
                        cur_obj->getObject("message")->getArray("entities")->
                            getObject(idx_command)->getValue<int64_t>("length");
                    commands.push_back(text.substr(index_of_command_in_text,
                                                   len_of_command_in_text));
                }
            }
            assert(!text.empty());
            std::shared_ptr<NewMessage> next =
                std::make_shared<NewMessage>(NewMessage(chat_id, message_id, text, commands));
            next->SetCommandsChecker(true);
            answer.push_back(std::static_pointer_cast<AbstractCPPClass>(next));
        }
    return answer;
}

std::vector<std::shared_ptr<AbstractCPPClass>> ClientTelegramBotAPI::GetUpdates(int timeout) {
    std::vector<std::shared_ptr<AbstractCPPClass>> all_updates_from_last_request;
    Poco::URI url(uri_ + "bot" + token_ + "/getUpdates");
    if (offset_) {
        url.addQueryParameter("offset", std::to_string(offset_));
    }
    if (timeout) {
        url.addQueryParameter("timeout", std::to_string(timeout));
    }

    //Poco::Net::HTTPClientSession *session = Poco::Net::HTTPSessionFactory::defaultFactory().createClientSession(url);
    Poco::Net::HTTPClientSession session(url.getHost(), url.getPort());

    Poco::Net::HTTPRequest request("GET", url.getPathAndQuery());
    session.sendRequest(request);

    Poco::Net::HTTPResponse response;
    std::istream& response_body = session.receiveResponse(response);

    if (response.getStatus() != 200) {
        throw TelegramAPIError(response.getStatus(), url.toString() + " getUpdates");
    }
    // SendMessage(400988361, "i m going inside of parser");

    all_updates_from_last_request = FormCppStructFromJson(response_body);

    return all_updates_from_last_request;
}

void ClientTelegramBotAPI::SendMessage(int64_t chat_id, std::string response, int64_t message_id) {
    Poco::URI url(uri_ + "bot" + token_ + "/sendMessage");
    //url.setQuery("chat_id=" + std::to_string(chat_id) + "&" + "text=" + response);
    Poco::JSON::Object json_for_send;
    json_for_send.set("chat_id", std::to_string(chat_id));
    json_for_send.set("text", response);
    if (message_id != -1) {
        json_for_send.set("reply_to_message_id", message_id);
    }

    //Poco::Net::HTTPClientSession *session =
        //Poco::Net::HTTPSessionFactory::defaultFactory().createClientSession(url);

    Poco::Net::HTTPClientSession session(url.getHost(), url.getPort());
    Poco::Net::HTTPRequest request("POST", url.getPathAndQuery());

    std::stringstream stringstream;
    json_for_send.stringify(stringstream);
    std::string data = stringstream.str();
    request.setContentType("application/json");
    request.setContentLength(data.size());

    std::ostream& request_body = session.sendRequest(request);
    request_body << data;

    Poco::Net::HTTPResponse http_response;
    session.receiveResponse(http_response);

    if (http_response.getStatus() != 200) {
        throw TelegramAPIError(http_response.getStatus(), "never give up! sendmessage");
    }
}

bool ClientTelegramBotAPI::GetMe() {
    Poco::URI url(uri_ + "bot" + token_ + "/getMe");
    // now you have the HTTP(S)ClientSession

    //Poco::Net::HTTPClientSession *session =
        //Poco::Net::HTTPSessionFactory::defaultFactory().createClientSession(url);
    Poco::Net::HTTPClientSession session(url.getHost(), url.getPort());
    Poco::Net::HTTPRequest request("GET", url.getPath());
    session.sendRequest(request);
    Poco::Net::HTTPResponse http_response;
    std::istream& response_body = session.receiveResponse(http_response);

    if (http_response.getStatus() != 200) {
        throw TelegramAPIError(http_response.getStatus(), http_response.getReason());
    }

    Poco::JSON::Parser parser;
    auto reply = parser.parse(response_body);
    return reply.extract<Poco::JSON::Object::Ptr>()->getValue<bool>("ok");
}

void ClientTelegramBotAPI::SetOffset() {
    std::ofstream file_out(offset_file_name_);
    file_out << offset_;
}

void ClientTelegramBotAPI::GetOffset() {
    std::ifstream file_input(offset_file_name_);
    if (file_input.is_open()) {
        try {
            file_input >> offset_;
        } catch (...) {
            offset_ = 0;
        }
    } else {
        offset_ = 0;
    }
}
