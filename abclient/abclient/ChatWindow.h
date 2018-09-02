#pragma once

#include <AB/ProtocolCodes.h>
#include "TabGroup.h"

class Player;

class ChatWindow : public UIElement
{
    URHO3D_OBJECT(ChatWindow, UIElement);
private:
    SharedPtr<BorderImage> background_;
    bool firstStart_;
    void HandleScreenshotTaken(StringHash eventType, VariantMap& eventData);
    void HandleEditFocused(StringHash eventType, VariantMap& eventData);
    void HandleEditDefocused(StringHash eventType, VariantMap& eventData);
    void HandleTextFinished(StringHash eventType, VariantMap& eventData);
    void HandleServerMessage(StringHash eventType, VariantMap& eventData);
    void HandleServerMessageUnknownCommand(VariantMap&);
    void HandleServerMessageInfo(VariantMap& eventData);
    void HandleServerMessageRoll(VariantMap& eventData);
    void HandleServerMessageAge(VariantMap& eventData);
    void HandleServerMessagePlayerNotOnline(VariantMap& eventData);
    void HandleServerMessagePlayerGotMessage(VariantMap& eventData);
    void HandleServerMessageNewMail(VariantMap& eventData);
    void HandleServerMessageMailSent(VariantMap& eventData);
    void HandleServerMessageMailNotSent(VariantMap& eventData);
    void HandleServerMessageMailboxFull(VariantMap& eventData);
    void HandleServerMessageMailDeleted(VariantMap& eventData);
    void HandleServerMessageServerId(VariantMap& eventData);
    void HandleChatMessage(StringHash eventType, VariantMap& eventData);
    void HandleTabSelected(StringHash eventType, VariantMap& eventData);
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    void HandleNameClicked(StringHash eventType, VariantMap& eventData);
    void HandleShortcutChatGeneral(StringHash eventType, VariantMap& eventData);
    void HandleShortcutChatGuild(StringHash eventType, VariantMap& eventData);
    void HandleShortcutChatParty(StringHash eventType, VariantMap& eventData);
    void HandleShortcutChatTrade(StringHash eventType, VariantMap& eventData);
    void HandleShortcutChatWhisper(StringHash eventType, VariantMap& eventData);
    void ParseChatCommand(const String& text, AB::GameProtocol::ChatMessageChannel defChannel);
    void CreateChatTab(TabGroup* tabs, AB::GameProtocol::ChatMessageChannel channel);
    LineEdit* GetActiveLineEdit();
    LineEdit* GetLineEdit(int index);
    int tabIndexWhisper_;
public:
    static void RegisterObject(Context* context);

    void AddLine(const String& text, const String& style);
    void AddLine(const String& name, const String& text, const String& style);
    void AddLine(uint32_t id, const String& name, const String& text,
        const String& style, const String& style2 = String::EMPTY,
        AB::GameProtocol::ChatMessageChannel channel = AB::GameProtocol::ChatChannelUnknown);

    void AddChatLine(uint32_t senderId, const String& name, const String& text,
        AB::GameProtocol::ChatMessageChannel channel);
    void SayHello(Player* player);

    ChatWindow(Context* context);
    ~ChatWindow()
    {
        UnsubscribeFromAllEvents();
    }

    void FocusEdit();
    SharedPtr<ListView> chatLog_;
private:
    SharedPtr<TabGroup> tabgroup_;
};

