#pragma once

#include <thread>
#include <variant>

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/illuminance.hpp>
#include <sensor_msgs/msg/range.hpp>

#include "events.h"
#include "log.h"


namespace android_ros {
class RosInterface {
 public:
  RosInterface();
  ~RosInterface() = default;

  void Initialize(size_t ros_domain_id);
  void Shutdown();

  bool Initialized() const;

  rclcpp::Context::SharedPtr get_context() const;
  rclcpp::Node::SharedPtr get_node() const;

  void AddObserver(std::function<void(void)> init_or_shutdown);

 private:
  rclcpp::Context::SharedPtr context_;
  rclcpp::Node::SharedPtr node_;
  rclcpp::Executor::SharedPtr executor_;

  std::vector<std::function<void(void)>> observers_;

  std::thread executor_thread_;

  void NotifyInitChanged();
};

/// Interface to A ROS publisher
// I want this class to insulate the Sensor interface from ROS going up or down
// or publishers being created etc when settings change
// This is to be used by the sensor interface to publish data
template <typename MsgT>
class Publisher {
 public:
  Publisher(RosInterface& ros) : ros_(ros) {
  }

  virtual ~Publisher() {}

  // Moves ok
  Publisher(Publisher&& other) = default;
  Publisher& operator=(Publisher&& other) = default;
  // No copies please
  Publisher(const Publisher& other) = delete;
  Publisher& operator=(const Publisher& other) = delete;


  void CreatePublisher() {
    LOGI("Created publisher!");
    // Check just in case publisher became disabled
    if (enable_) {
      auto node = ros_.get_node();
      publisher_ = node->template create_publisher<MsgT>(topic_, qos_);
      // Tell ROS interface to destroy publisher when it's shutdown
      ros_.AddObserver(std::bind(&Publisher::DestroyPublisher, this));
    }
  }

  void DestroyPublisher() {
    LOGI("Destroyed publisher!");
    publisher_.reset();
  }

  void Enable() {
    LOGI("Asked to enable publisher");
    enable_ = true;
    if (!publisher_) {
      if (ros_.Initialized()) {
        CreatePublisher();
      } else {
        // Tell ROS interface to create publisher when it's initialized
        ros_.AddObserver(std::bind(&Publisher::CreatePublisher, this));
      }
    }
  }

  void Disable() {
    LOGI("Asked to disable publisher");
    enable_ = false;
    DestroyPublisher();
  }

  // Big messages to avoid copies
  void Publish(const typename MsgT::SharedPtr& msg) const {
    if (publisher_) {
      publisher_->publish(msg);
    }
  }

  // Little messages to avoid heap allocation
  void Publish(const MsgT& msg) const {
    LOGI("asked to publish message");
    if (publisher_) {
      LOGI("did publish message");
      publisher_->publish(msg);
    }
  }

 private:
  bool enable_ = false;
  RosInterface &ros_;
  std::string topic_ = "default_topic";
  rclcpp::QoS qos_ = rclcpp::QoS(1);
  typename rclcpp::Publisher<MsgT>::SharedPtr publisher_;
};
}  // namespace android_ros
