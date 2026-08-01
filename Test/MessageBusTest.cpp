//
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <Recluse/MessageBus.hpp>

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