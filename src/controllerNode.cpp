#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include <SDL.h>

#include <iostream>
#include "PCA9685.h"

const float MAX_JOYSTICK_VALUE = 32767.0;
const float MIN_JOYSTICK_VALUE = -32767.0;

using namespace std::chrono_literals;

class ControllerNode : public rclcpp::Node
{
public:
    ControllerNode() : Node("ControllerNode")
    {
        // Open joystick controller
        if (SDL_Init(SDL_INIT_JOYSTICK) != 0)
        {
            fprintf(stderr, "%s", SDL_GetError());
            throw std::runtime_error("Failed to connect to controller");
        }

        // Open the joystick device
        this->joystick = SDL_JoystickOpen(0);
        if (!this->joystick)
        {
            SDL_Quit();
            throw std::runtime_error("Failed to open joystick");
        }
        this->publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 10);
        this->timer_ = this->create_wall_timer(500ms, std::bind(&ControllerNode::publishTwist, this));
        this->counter_ = 0;
    }
    ~ControllerNode()
    {
        SDL_JoystickClose(this->joystick);
        SDL_Quit();
    }

    void publishTwist()
    {
        float linear_speed;
        float angular_velocity;
        auto twist_msg = std::make_shared<geometry_msgs::msg::Twist>();

        // while the node runs
        while (rclcpp::ok())
        {
            // RCLCPP_INFO(get_logger(), "Number of buttons %d\n", SDL_JoystickNumButtons(joystick));

            // When an event is triggered on the joystick
            while (SDL_PollEvent(&this->event))
            {
                RCLCPP_INFO(get_logger(), "Event type: %d\n", this->event.type);
                for (int i = 0; i < SDL_JoystickNumButtons(joystick); i++)
                {
                    if (SDL_JoystickGetButton(joystick, i))
                    {
                        RCLCPP_INFO(get_logger(), "Button %d pressed\n", i);
                    }
                }
                if (this->event.type == SDL_JOYAXISMOTION)
                {
                    RCLCPP_INFO(get_logger(), "joystick");
                    RCLCPP_INFO(get_logger(), "event.jaxis.axis: %d\n", event.jaxis.axis);
                    if (event.jaxis.axis == 1) // Left stick horizontal
                    {
                        angular_velocity = SDL_JoystickGetAxis(joystick, 0) / MAX_JOYSTICK_VALUE;
                    }
                    else if (event.jaxis.axis == 2) // Left trigger -backwards
                    {
                        float inputData = -SDL_JoystickGetAxis(joystick, 2) / MAX_JOYSTICK_VALUE;
                        linear_speed = inputData < 0 ? inputData : 0;
                    }
                    else if (event.jaxis.axis == 5) // Right trigger -forward
                    {
                        float inputData = SDL_JoystickGetAxis(joystick, 5) / MAX_JOYSTICK_VALUE;
                        linear_speed = inputData > 0 ? inputData : 0;
                    }
                }
                else if (event.type == SDL_JOYBUTTONDOWN)
                {
                    bool button = SDL_JoystickGetButton(joystick, 5);
                    RCLCPP_INFO(get_logger(), "button=%d", button);
                    if (button)
                    {
                        // twist_msg->linear.z = 1; // Borrowing unused variable to send button data
                    }
                }
                else if (event.type == SDL_JOYHATMOTION)
                {
                    RCLCPP_INFO(get_logger(), "D-pad hat value: %d\n", event.jhat.value);
                    if (event.jhat.value == 1)
                    { // D-pad up
                        twist_msg->linear.z = 1;
                    }
                    else if (event.jhat.value == 4)
                    { // D-pad down
                        twist_msg->linear.z = 2;
                    }
                    else if (event.jhat.value == 8)
                    { // D-pad left
                        twist_msg->linear.z = 3;
                    }
                    else if (event.jhat.value == 2)
                    { // D-pad right
                        twist_msg->linear.z = 4;
                    }
                }
                else
                {
                    linear_speed = 0.0;
                    angular_velocity = 0.0;
                    twist_msg->linear.z = 0;
                }
                twist_msg->linear.x = -linear_speed;
                twist_msg->angular.z = angular_velocity;
                RCLCPP_INFO(get_logger(), "linear.=%f, angular.=%f r1.=%f", twist_msg->linear.x, twist_msg->angular.z, twist_msg->linear.z);
                this->publisher_->publish(*twist_msg);
                twist_msg->linear.z = 0;
            }
        }
    }

private:
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    size_t counter_;
    SDL_Event event;
    SDL_Joystick *joystick;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<ControllerNode>();
    node->publishTwist();
    rclcpp::shutdown();
    return 0;
}