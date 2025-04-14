#include "user_session.hxx"

tg::UserSession::UserSession(const std::shared_ptr<TgBot::Bot>& bot) : _bot(bot) {}

void tg::UserSession::manage(const TgBot::Message::Ptr& message)
{
    if(_activeCommand != nullptr) {
        return;
    }


    return;
}
