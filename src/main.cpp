#include <vector>
#include <iostream>
#include <string>
#include <algorithm>

#include <opencv2/opencv.hpp>
#include <tgbot/tgbot.h>
#include <ArtRobot/ArtRobot.h>

#include "Global.h"
#include "Log.h"
#include "UsersData.h"
#include "MakeSticker.h"
// #include "InlineQuery.h"
#include "Tg.h"
#include "StringCheck.h"

using namespace std;
using namespace cv;
using namespace TgBot;

std::string botUsername;
std::string botUsernameLowercase;
int32_t botId = 0;
UsersData usersData;

int main()
{
    cout << "====================" << endl
         << "|  SayStickerBot!  |" << endl
         << "====================" << endl;

    // init
    usersData.readFromFile();

    string token = getenv("TOKEN");
    Bot bot(token);

    bot.getEvents().onAnyMessage([&bot](Message::Ptr message) { // 处理收到的直接消息
        LogI("Message: chat->username=%s, chat->id=%lld, text=%s", message->chat->username.c_str(), message->chat->id, message->text.c_str());

        auto &api = bot.getApi();
        auto chatId = message->chat->id;

        if (message->forwardDate) // 是转发的消息
        {
            if (message->forwardFrom)
            {
                MakeSticker(api, chatId, message->forwardFrom, message->text,message->from->id);
            }
            else
            { // 被转发用户的隐私设置原因无法获取uid
                sendMessage(api, chatId, "该用户的隐私设置不允许转发。");
            }
            return;
        }

        if (message->text.c_str()[0] == '/') // 如果是指令则跳过
            return;

        if (message->chat->type == Chat::Type::Private) // 只有私聊显示help
            sendMessage(api, chatId, "如果想要什么帮助的话请给我发 /help");
    });

    bot.getEvents().onCommand("help", [&bot](Message::Ptr message) { // /help
        auto &api = bot.getApi();
        auto chatId = message->chat->id;

        if (message->chat->type == Chat::Type::Private)
        { // 私聊
            sendMessage(api, chatId, "请转发消息给我。");
        } // 不支持群组
    });

    bot.getEvents().onCommand("start", [&bot](Message::Ptr message) { // /start
        auto &api = bot.getApi();
        auto chatId = message->chat->id;

        sendMessage(api, chatId, "喵～");
    });

    bot.getEvents().onCommand("throw", [&bot](Message::Ptr message) { // /throw
        auto &api = bot.getApi();
        auto chatId = message->chat->id;

        sendMessage(api, chatId, "喵～");
    });

    bot.getEvents().onUnknownCommand([&bot](Message::Ptr message) { // 未知指令
        auto &api = bot.getApi();
        auto chatId = message->chat->id;

        if (message->chat->type == Chat::Type::Private)
        { // 私聊
            sendMessage(api, chatId, "你在说什么？\n如果想要什么帮助的话请给我发 /help");
        }
    });

    bot.getEvents().onInlineQuery([&bot](InlineQuery::Ptr inlineQuery) {
        auto &query = inlineQuery->query;

        LogI("InlineQuery: %s: %s", inlineQuery->from->username.c_str(), query.c_str());

        vector<InlineQueryResult::Ptr> results; // 准备results

        // if (query.c_str()[0] == '@') // 首位是@的话进行精确匹配
        //     pushStickerToResultByUsername(bot.getApi(), results, query.c_str() + 1);
        // else
        //     pushStickerToResultByUsernameFuzzy(bot.getApi(), results, query);

        // if (results.size() == 0) // 如果列表依然是空的，则显示按钮用于创建
        //     pushClickToThrow(bot.getApi(), results, query);

        // debug json
        // TgTypeParser tgTypeParser;
        // cout << tgTypeParser.parseArray<InlineQueryResult>(&TgTypeParser::parseInlineQueryResult, results) << endl;

        try
        {
            bot.getApi().answerInlineQuery(inlineQuery->id, results);
        }
        catch (TgException &e)
        {
            LogE("answerInlineQuery: %s", e.what());
        }
    });

    bot.getEvents().onCallbackQuery([&bot](CallbackQuery::Ptr callbackQuery) {
        auto &username = callbackQuery->data;

        LogI("onCallbackQuery: %s: %s", callbackQuery->from->username.c_str(), username.c_str());

        try
        {
            bot.getApi().answerCallbackQuery(callbackQuery->id, "喵～");
        }
        catch (TgException &e)
        {
            LogE("TgBot::Api::answerCallbackQuery: %s", e.what());
        }
    });

    while (true)
    {
        try
        {
            LogI("Starting ...");
            botUsernameLowercase = botUsername = bot.getApi().getMe()->username;
            lowercase(botUsernameLowercase);
            botId = bot.getApi().getMe()->id;
            LogI("Bot username: %s %d", botUsername.c_str(), botId);

            TgLongPoll longPoll(bot);
            while (true)
            {
                LogI("Long poll started.");
                longPoll.start();
            }

            // TgWebhookTcpServer webhookServer(8888, bot);
            // string webhookUrl(getenv("WEBHOOK_URL"));
            // bot.getApi().setWebhook(webhookUrl);
            // webhookServer.start();
        }
        catch (TgException &e)
        {
            LogE("Error: %s", e.what());
        }
        catch (...)
        {
            LogE("Unknow error.");
        }
        LogI("Restart.");
    }

    return 0;
}