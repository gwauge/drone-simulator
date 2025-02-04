#pragma once

// #include "generated/ObstaclesPubSubTypes.hpp"
#include "ObstaclesPubSubTypes.hpp"

#include <chrono>
#include <thread>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>

using namespace eprosima::fastdds::dds;

#define DOMAIN_ID 1

class ObstaclePublisher
{
private:
    Obstacles my_obstacles_;
    DomainParticipant *participant_;
    Publisher *publisher_;
    Topic *topic_;
    DataWriter *writer_;
    TypeSupport type_;

    class PubListener : public DataWriterListener
    {
    public:
        std::atomic_int matched_;
        PubListener();
        ~PubListener() override;
        void on_publication_matched(DataWriter *writer, const PublicationMatchedStatus &info) override;
    } listener_;

public:
    ObstaclePublisher();
    virtual ~ObstaclePublisher();

    bool init();
    bool publish_random();
    bool publish(Obstacles *msg);
    void run(uint32_t samples);

private:
    double fRand(double fMin, double fMax);
};

class ObstacleSubscriber
{
private:
    DomainParticipant *participant_;
    Subscriber *subscriber_;
    DataReader *reader_;
    Topic *topic_;
    TypeSupport type_;

    class SubListener : public DataReaderListener
    {
    public:
        SubListener();
        ~SubListener() override;
        void on_subscription_matched(DataReader *, const SubscriptionMatchedStatus &info) override;
        void on_data_available(DataReader *reader) override;
        Obstacles my_obstacles_;
        std::atomic_int samples_;
    } listener_;

public:
    ObstacleSubscriber();
    virtual ~ObstacleSubscriber();

    //! Initialize the subscriber
    bool init();

    //! Run the Subscriber
    void run(uint32_t samples);
};
