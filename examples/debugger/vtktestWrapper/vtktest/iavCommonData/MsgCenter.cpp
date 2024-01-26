#include "MsgCenter.h"

#include <map>

namespace MsgCenter
{
    std::map<void*, callback_t> msgObservers{};
}

// This function is not thread-safe
void MsgCenter::send(const Msg msg, const std::any& data)
{
    static std::list<std::pair<Msg, std::any>> msgList{};
    static auto isExecuting = false;

    msgList.push_back({ msg, data });

    if (isExecuting)
    {
        return;
    }

    while (isExecuting = !msgList.empty())
    {
        const auto msg = std::move(msgList.front());
        msgList.pop_front();

        for (const auto& i : msgObservers)
        {
            i.second(msg.first, msg.second);
        }
    }
}

void MsgCenter::attach(void* id, const callback_t& cb)
{
    msgObservers[id] = cb;
}

void MsgCenter::detach(void* id)
{
    msgObservers.erase(id);
}