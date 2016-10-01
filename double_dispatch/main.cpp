#include <iostream>
#include <vector>

// Inspired by https://www.youtube.com/watch?v=uFq1T4UL11o
// Video Title: C++ Messaging Systems
// Channel: Game Engine Architecture Club
// Compile with -std=c++11

// Forward declare the IMessageHandler so Message knows about it
class IMessageHandler;
// Need to forward declare these for the IMessageHandler
// mess created by me trying to cram this whole example into one file!
class CollisionMessage;
class MouseMessage;
class KeyMessage;

// The base message class from which the others will derrive
class Message
{
public:
    Message() {}
    virtual ~Message() {};
    // The derrived messages knows their own type, and so are able to call the correct
    // overload of the HandleMessage() function from the IMessageHandler
    virtual void Dispatch(IMessageHandler& handler) = 0;
};

class IMessageHandler
{
public:
    virtual void HandleMessage(const CollisionMessage& msg) {}
    virtual void HandleMessage(const MouseMessage& msg) {}
    virtual void HandleMessage(const KeyMessage& msg) {}
};

// Each subclass of Message implements it's own Dispatch(), and thus knows which overload
// of HandleMessage() to call on itelsf
class CollisionMessage : public Message
{
public:
    virtual void Dispatch(IMessageHandler& handler) override { handler.HandleMessage(*this); }
};
class MouseMessage : public Message
{
public:
    virtual void Dispatch(IMessageHandler& handler) override { handler.HandleMessage(*this); }
};
class KeyMessage : public Message
{
public:
    virtual void Dispatch(IMessageHandler& handler) override { handler.HandleMessage(*this); }
};

// The message handler interface provides the overloads for each type of message
// Our GameObject will actually do something with these messages
class GameObject : public IMessageHandler
{
    public:
        GameObject() {}
        ~GameObject() {}

        void HandleMessage(const CollisionMessage& msg) override { std::cout << "Ouch!" << std::endl; }
        void HandleMessage(const MouseMessage& msg) override { std::cout << "Swoosh" << std::endl; }
        void HandleMessage(const KeyMessage& msg) override { std::cout << "Click..." << std::endl; }
};

int main()
{
    GameObject myObject;

    std::vector<Message*> messages;
    messages.push_back(new CollisionMessage());
    messages.push_back(new MouseMessage());
    messages.push_back(new KeyMessage());

    // Process all our messages
    for( auto msg : messages )
        msg->Dispatch( myObject );

    // Cleanup pointers
    for( auto p : messages )
        delete p;
    return 0;
}
