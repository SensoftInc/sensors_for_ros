FROM ubuntu:latest

# Set locale to UTF-8
RUN apt update && apt install locales
RUN locale-gen en_US en_US.UTF-8
RUN update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
RUN export LANG=en_US.UTF-8

# Update and install required repositories amd tools
RUN apt install software-properties-common -y
RUN add-apt-repository universe
RUN apt update && apt install -y curl wget git cmake

# Set locale
RUN apt install locales
RUN locale-gen en_US en_US.UTF-8
RUN update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
RUN export LANG=en_US.UTF-8

# Add ZScaler certtificatte 
ADD ZscalerRootCertificate-2048-SHA256.crt /root/
RUN mv ~/ZscalerRootCertificate-2048-SHA256.crt /usr/local/share/ca-certificates \
   && update-ca-certificates \
   && apt update   

# Add Android SDK and NDK
RUN apt install -y android-sdk
RUN mkdir /usr/lib/android-sdk/ndk && cd /usr/lib/android-sdk/ndk && wget https://dl.google.com/android/repository/android-ndk-r25b-linux.zip && unzip android-ndk-r25b-linux.zip && mv android-ndk-r25b 25.1.8937393

# Add required libs for building ROS2 libraries
RUN curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key -o /usr/share/keyrings/ros-archive-keyring.gpg
RUN echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros2/ubuntu $(. /etc/os-release && echo $UBUNTU_CODENAME) main" | tee /etc/apt/sources.list.d/ros2.list > /dev/null
RUN apt update && apt install -y python3-flake8-docstrings python3-pip python3-pytest-cov python3-vcstool 
RUN pip3 install -U catkin-tools
RUN apt update && apt install -y python3-flake8-blind-except python3-flake8-builtins python3-flake8-class-newline python3-flake8-comprehensions python3-flake8-deprecated python3-flake8-import-order python3-flake8-quotes python3-pytest-repeat python3-pytest-rerunfailures python3-lark python3-empy

# Clone sensors_for_ros repo
WORKDIR /usr/src
RUN git clone https://www.github.com/SensoftInc/sensors_for_ros

# Install ROS2 built with Android NDK
RUN mkdir sensors_for_ros/build && cd sensors_for_ros/build && cmake -DANDROID_HOME=/usr/lib/android-sdk -DANDROID_PLATFORM=31 -DANDROID_ABI=arm64-v8a .. && make && cd deps && cp -r * /usr/local

# Update LD_LIBRARY_PATH and PYTHONPATH for ROS2 libs
ENV PYTHONPATH "$PYTHONPATH:/usr/local/_python_"
ENV LD_LIBRARY_PATH "$LD_LIBRARY_PATH:/usr/local/lib"

# Expose port that ROS2 uses
EXPOSE 9090