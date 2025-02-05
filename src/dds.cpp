/**
 * @file obstacle_publisher.cpp
 *
 */

#include "dds.hpp"

// =========== Publisher ===========
template <typename T, typename U>
DDSPublisher<T, U>::PubListener::PubListener() : matched_(0) {}

template <typename T, typename U>
DDSPublisher<T, U>::PubListener::~PubListener() = default;

template <typename T, typename U>
void DDSPublisher<T, U>::PubListener::on_publication_matched(DataWriter * /*writer*/, const PublicationMatchedStatus &info)
{
    if (info.current_count_change == 1)
    {
        matched_ = info.total_count;
        std::cout << "Publisher matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.total_count;
        std::cout << "Publisher unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change." << std::endl;
    }
}

template <typename T, typename U>
DDSPublisher<T, U>::DDSPublisher(const std::string &topic_name)
    : topic_name_(topic_name), participant_(nullptr),
      publisher_(nullptr), topic_(nullptr),
      writer_(nullptr), type_(new U())
{
}

template <typename T, typename U>
DDSPublisher<T, U>::~DDSPublisher()
{
    if (writer_ != nullptr)
    {
        publisher_->delete_datawriter(writer_);
    }
    if (publisher_ != nullptr)
    {
        participant_->delete_publisher(publisher_);
    }
    if (topic_ != nullptr)
    {
        participant_->delete_topic(topic_);
    }
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

template <typename T, typename U>
bool DDSPublisher<T, U>::init()
{
    DomainParticipantQos participantQos;
    participantQos.name("Participant_publisher");
    participant_ = DomainParticipantFactory::get_instance()->create_participant(DOMAIN_ID, participantQos);

    if (participant_ == nullptr)
    {
        return false;
    }

    // Register the Type
    type_.register_type(participant_);

    // Create the publications Topic
    topic_ = participant_->create_topic(topic_name_, type_.get_type_name(), TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        return false;
    }

    // Create the Publisher
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);

    if (publisher_ == nullptr)
    {
        return false;
    }

    // Create the DataWriter
    writer_ = publisher_->create_datawriter(topic_, DATAWRITER_QOS_DEFAULT, &listener_);

    if (writer_ == nullptr)
    {
        return false;
    }
    return true;
}

template <typename T, typename U>
bool DDSPublisher<T, U>::publish(T *msg)
{
    if (listener_.matched_ > 0)
    {
        writer_->write(msg);
        return true;
    }
    return false;
}

double fRand(double fMin, double fMax)
{
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

// Explicit template instantiations
template class DDSPublisher<Obstacles, ObstaclesPubSubType>;
template class DDSPublisher<Targets, TargetsPubSubType>;

// =========== Subscriber ===========
template <typename DataType, typename PubSubType>
DDSSubscriber<DataType, PubSubType>::SubListener::SubListener(std::function<void(const DataType &)> callback) : samples_(0), callback_(callback) {}

template <typename DataType, typename PubSubType>
DDSSubscriber<DataType, PubSubType>::SubListener::~SubListener() = default;

template <typename DataType, typename PubSubType>
void DDSSubscriber<DataType, PubSubType>::SubListener::on_subscription_matched(
    DataReader *,
    const SubscriptionMatchedStatus &info)
{
    if (info.current_count_change == 1)
    {
        std::cout << "Subscriber matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        std::cout << "Subscriber unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change." << std::endl;
    }
}

template <typename DataType, typename PubSubType>
void DDSSubscriber<DataType, PubSubType>::SubListener::on_data_available(DataReader *reader)
{
    SampleInfo info;
    if (reader->take_next_sample(&data, &info) == eprosima::fastdds::dds::RETCODE_OK)
    {
        if (info.valid_data)
        {
            samples_++;
            if (callback_)
            {
                callback_(data);
            }
        }
    }
}

template <typename DataType, typename PubSubType>
DDSSubscriber<DataType, PubSubType>::DDSSubscriber(
    const std::string &topic_name,
    std::function<void(const DataType &)> callback)
    : topic_name_(topic_name), callback_(callback), listener_(callback),
      participant_(nullptr),
      subscriber_(nullptr), topic_(nullptr),
      reader_(nullptr), type_(new PubSubType())
{
}

template <typename DataType, typename PubSubType>
DDSSubscriber<DataType, PubSubType>::~DDSSubscriber()
{
    if (reader_ != nullptr)
    {
        subscriber_->delete_datareader(reader_);
    }
    if (topic_ != nullptr)
    {
        participant_->delete_topic(topic_);
    }
    if (subscriber_ != nullptr)
    {
        participant_->delete_subscriber(subscriber_);
    }
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

template <typename DataType, typename PubSubType>
bool DDSSubscriber<DataType, PubSubType>::init()
{
    DomainParticipantQos participantQos;
    participantQos.name("Participant_subscriber");
    participant_ = DomainParticipantFactory::get_instance()->create_participant(DOMAIN_ID, participantQos);

    if (participant_ == nullptr)
    {
        return false;
    }

    // Register the Type
    type_.register_type(participant_);

    // Create the subscriptions Topic
    topic_ = participant_->create_topic(topic_name_, type_.get_type_name(), TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        return false;
    }

    // Create the Subscriber
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);

    if (subscriber_ == nullptr)
    {
        return false;
    }

    // Create the DataReader
    reader_ = subscriber_->create_datareader(topic_, DATAREADER_QOS_DEFAULT, &listener_);

    if (reader_ == nullptr)
    {
        return false;
    }

    return true;
}

template <typename DataType, typename PubSubType>
void DDSSubscriber<DataType, PubSubType>::run(uint32_t samples)
{
    while (listener_.samples_ < samples)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// Explicit template instantiations
template class DDSSubscriber<Obstacles, ObstaclesPubSubType>;
template class DDSSubscriber<Targets, TargetsPubSubType>;
