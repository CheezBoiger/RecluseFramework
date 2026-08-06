//
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <Recluse/MessageBus.hpp>

#include <Recluse/System/Window.hpp>
#include <Recluse/System/Input.hpp>

class TestEvent : public Recluse::EventMessage
{
public:
    TestEvent(Recluse::EventId id, int data = -100) : Recluse::EventMessage(id)
        , data(data) { }

    int getData() const { return data; } 

private: 
    int data;
};

TEST(MessageBusTest, NotifyAll)
{
    Recluse::MessageBus testMessageBus("TestBus");
    enum Id { TestEventId = -40 };
    
    testMessageBus.initialize();

    testMessageBus.addReceiver("TestReceiver", 
        [] (const Recluse::EventMessage& msg) -> Recluse::ResultCode { 
            EXPECT_EQ(msg.getEvent(), TestEventId);
            const TestEvent& testEvent = Recluse::EventMessage::castTo<TestEvent>(msg);

            EXPECT_NE(testEvent.getData(), -100);

            EXPECT_EQ(testEvent.getData(), 50);
            return Recluse::RecluseResult_Ok;
    });

    Recluse::MessageBus::sendEvent<TestEvent>(&testMessageBus, TestEventId, 50);

    testMessageBus.notifyAll();

    testMessageBus.cleanUp();
}

TEST(WindowTest, CreateWindowSimpleWindowBorderless)
{
    Recluse::Window* window = Recluse::Window::create("Test", 0, 0, 128, 128, Recluse::ScreenMode_WindowBorderless);
    
    window->show();

    Recluse::U32 i = 0;
    
    while (!window->shouldClose())
    {
        Recluse::pollEvents();
        ++i;
        
        EXPECT_EQ(window->getScreenMode(), Recluse::ScreenMode_WindowBorderless);
        EXPECT_EQ(window->getPosX(), 0);
        EXPECT_EQ(window->getPosY(), 0);
        EXPECT_EQ(window->getWidth(), 128);
        EXPECT_EQ(window->getWidth(), 128);

        if (i == 100)
        {
            window->close();

            EXPECT_EQ(window->shouldClose(), true);
        }
    }
    Recluse::Window::destroy(window);
}