/**
 * @file obstacle_publisher.cpp
 *
 */

#include "obstacles_dds.hpp"

// Publisher
ObstaclePublisher::PubListener::PubListener() : matched_(0) {}
ObstaclePublisher::PubListener::~PubListener() = default;

void ObstaclePublisher::PubListener::on_publication_matched(DataWriter * /*writer*/, const PublicationMatchedStatus &info)
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

ObstaclePublisher::ObstaclePublisher()
    : participant_(nullptr), publisher_(nullptr), topic_(nullptr), writer_(nullptr), type_(new ObstaclesPubSubType())
{
}

ObstaclePublisher::~ObstaclePublisher()
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

bool ObstaclePublisher::init()
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
    topic_ = participant_->create_topic("obstacles_local", type_.get_type_name(), TOPIC_QOS_DEFAULT);

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

//! Send a randomized publication
bool ObstaclePublisher::publish_random()
{
    if (listener_.matched_ > 0)
    {
        int num = fRand(2, 5);
        my_obstacles_.obstacles_number(num);
        std::vector<int> x_vals(num);
        std::vector<int> y_vals(num);

        for (int i = 0; i < num; i++)
        {
            x_vals[i] = fRand(0, 100);
            y_vals[i] = fRand(0, 100);
        }
        my_obstacles_.obstacles_x(x_vals);
        my_obstacles_.obstacles_y(y_vals);
        writer_->write(&my_obstacles_);
        return true;
    }
    return false;
}

bool ObstaclePublisher::publish(Obstacles *msg)
{
    if (listener_.matched_ > 0)
    {
        writer_->write(msg);
        return true;
    }
    return false;
}

double ObstaclePublisher::fRand(double fMin, double fMax)
{
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

// Subscriber
ObstacleSubscriber::SubListener::SubListener() : samples_(0) {}
ObstacleSubscriber::SubListener::~SubListener() = default;

void ObstacleSubscriber::SubListener::on_subscription_matched(
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

void ObstacleSubscriber::SubListener::on_data_available(DataReader *reader)
{
    SampleInfo info;
    if (reader->take_next_sample(&my_obstacles_, &info) == eprosima::fastdds::dds::RETCODE_OK)
    {
        if (info.valid_data)
        {
            samples_++;
            std::cout << "Obstacle received: " << my_obstacles_.obstacles_number() << std::endl;
            for (int i = 0; i < my_obstacles_.obstacles_number(); i++)
            {
                std::cout << "\tObstacle " << i << ": ("
                          << my_obstacles_.obstacles_x()[i] << ", "
                          << my_obstacles_.obstacles_y()[i] << ")"
                          << std::endl;
            }
        }
    }
}

ObstacleSubscriber::ObstacleSubscriber()
    : participant_(nullptr), subscriber_(nullptr), topic_(nullptr), reader_(nullptr), type_(new ObstaclesPubSubType())
{
}

ObstacleSubscriber::~ObstacleSubscriber()
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

bool ObstacleSubscriber::init()
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
    topic_ = participant_->create_topic("obstacles_local", type_.get_type_name(), TOPIC_QOS_DEFAULT);

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

void ObstacleSubscriber::run(uint32_t samples)
{
    while (listener_.samples_ < samples)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
