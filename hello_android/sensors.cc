#include "sensors.h"

#include "illuminance_sensor.h"
#include "jvm.h"
#include "log.h"

using android_ros::SensorDescriptor;
using android_ros::Sensors;
using android_ros::IlluminanceSensor;

Sensors::Sensors(ANativeActivity* activity) {
  // TODO(sloretz) Query sensors
  std::string package_name = GetPackageName(activity);
  sensor_manager_ = ASensorManager_getInstanceForPackage(package_name.c_str());

  auto descriptors = QuerySensors();
  // Create wrapping classes
  for (const SensorDescriptor & desc : descriptors) {
    if (ASENSOR_TYPE_LIGHT == desc.type) {
      sensors_.push_back(
        std::move(std::make_unique<IlluminanceSensor>(sensor_manager_, desc)));
    }
  }
}

std::vector<SensorDescriptor> Sensors::QuerySensors() {
  std::vector<SensorDescriptor> descriptors;
  ASensorList sensor_list;
  int num_sensors = ASensorManager_getSensorList(sensor_manager_, &sensor_list);
  if (num_sensors < 0) {
    LOGW("Could not get sensor list: %d", num_sensors);
    return {};
  }
  LOGI("Got %d sensors", num_sensors);

  while (num_sensors > 0) {
    --num_sensors;
    ASensorRef sensor_ref = sensor_list[num_sensors];
    if (nullptr == sensor_ref) {
      LOGW("Got null sensor at position :%d", num_sensors);
      continue;
    }
    descriptors.emplace_back(sensor_ref);
    LOGI("Sensor %s name %s", descriptors.back().type_str, descriptors.back().name);
  }
  return descriptors;
}

void Sensors::Initialize() {
  for (auto & sensor : sensors_) {
    sensor->Initialize();
  }
}

void Sensors::Shutdown() {
  for (auto & sensor : sensors_) {
    sensor->Shutdown();
  }
  sensors_.clear();
}
