#pragma once

// #include "generated/ObstaclesPubSubTypes.hpp"
#include "ObstaclesPubSubTypes.hpp"
#include "TargetsPubSubTypes.hpp"

#include <chrono>
#include <thread>
#include <functional>

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

double fRand(double fMin, double fMax);

template <typename T, typename U>
class DDSPublisher
{
private:
    T data;
    U additional_data;
    DomainParticipant *participant_;
    Publisher *publisher_;
    Topic *topic_;
    DataWriter *writer_;
    TypeSupport type_;
    std::string topic_name_;

    class PubListener : public DataWriterListener
    {
    public:
        std::atomic_int matched_;
        PubListener();
        ~PubListener() override;
        void on_publication_matched(DataWriter *writer, const PublicationMatchedStatus &info) override;
    } listener_;

public:
    DDSPublisher(const std::string &topic_name);
    virtual ~DDSPublisher();

    bool init();
    bool publish(T *msg);
    // void run(uint32_t samples);
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

template <typename DataType, typename PubSubType>
class DDSSubscriber
{
private:
    DomainParticipant *participant_;
    Subscriber *subscriber_;
    DataReader *reader_;
    Topic *topic_;
    TypeSupport type_;
    std::string topic_name_;
    std::function<void(const DataType &)> callback_;

    class SubListener : public DataReaderListener
    {
    public:
        SubListener(std::function<void(const DataType &)> callback);
        ~SubListener() override;
        void on_subscription_matched(DataReader *, const SubscriptionMatchedStatus &info) override;
        void on_data_available(DataReader *reader) override;
        DataType data;
        std::atomic_int samples_;
        std::function<void(const DataType &)> callback_;
    } listener_;

public:
    DDSSubscriber(const std::string &topic_name, std::function<void(const DataType &)> callback);
    virtual ~DDSSubscriber();

    //! Initialize the subscriber
    bool init();

    //! Run the Subscriber
    void run(uint32_t samples);
};

/*
template <typename DataType, typename PubSubType>
DDSSubscriber<DataType, PubSubType>::DDSSubscriber(const std::string &topic_name, std::function<void(const DataType &)> callback)
    : topic_name_(topic_name), callback_(callback), listener_(callback)
{
}

template <typename DataType, typename PubSubType>
DDSSubscriber<DataType, PubSubType>::~DDSSubscriber()
{
    if (reader_ != nullptr)
    {
        subscriber_->delete_datareader(reader_);
    }
    if (subscriber_ != nullptr)
    {
        participant_->delete_subscriber(subscriber_);
    }
    if (topic_ != nullptr)
    {
        participant_->delete_topic(topic_);
    }
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

template <typename DataType, typename PubSubType>
bool DDSSubscriber<DataType, PubSubType>::init()
{
    // Initialization code here
    return true;
}

template <typename DataType, typename PubSubType>
void DDSSubscriber<DataType, PubSubType>::run(uint32_t samples)
{
    // Running code here
}

template <typename DataType, typename PubSubType>
DDSSubscriber<DataType, PubSubType>::SubListener::SubListener(std::function<void(const DataType &)> callback)
    : samples_(0), callback_(callback)
{
}

template <typename DataType, typename PubSubType>
DDSSubscriber<DataType, PubSubType>::SubListener::~SubListener()
{
}

template <typename DataType, typename PubSubType>
void DDSSubscriber<DataType, PubSubType>::SubListener::on_subscription_matched(DataReader *, const SubscriptionMatchedStatus &info)
{
    // Handle subscription matched event
}

template <typename DataType, typename PubSubType>
void DDSSubscriber<DataType, PubSubType>::SubListener::on_data_available(DataReader *reader)
{
    SampleInfo info;
    if (reader->take_next_sample(&data, &info) == ReturnCode_t::RETCODE_OK)
    {
        if (info.instance_state == ALIVE_INSTANCE_STATE)
        {
            samples_++;
            if (callback_)
            {
                callback_(data);
            }
        }
    }
}
*/
