#include "PCA9685.h"

static int fd = 0;
uint8_t ServoUpDegree = 90;   // Servo Up Angle
uint8_t ServoDownDegree = 90; // Servo Down Angle
/*
// functions：i2c_init
// parameters：i2c_dev：i2c device file path，i2c_addr：i2c address
// return：0 success，-1 failed
*/
int i2c_init(char *i2c_dev, unsigned char i2c_addr)
{
    int res = 0;
    fd = open(i2c_dev, O_RDWR);
    if (fd < 0)
    {
        printf("Failed to open the i2c bus\n");
        return -1;
    }
    res = ioctl(fd, I2C_TENBIT, 0);       // 7-bit mode
    res = ioctl(fd, I2C_SLAVE, i2c_addr); // Sets the slave device address
    return res;
}
/*
// functions：i2c reading data
// parameters：buf：Data buffer，len：length
// return: The length of the actual read
*/
int i2c_readNbyte(unsigned char *buf, int len)
{
    int res = 0;
    res = read(fd, buf, len);
    return res;
}
/*
// functions：i2c writing data
// parameters：buf：Data buffer，len：length
// return: The length of the actual write
*/
int i2c_writeNbyte(unsigned char *buf, int len)
{
    int res = 0;
    res = write(fd, buf, len);
    return res;
}
/*
// functions：read at specified address
// parameters：reg_addr： register address，buf：Data buffer
// return: The length of the actual read
*/
int i2c_readReg(uint8_t reg_addr, uint8_t *data)
{
    int res = 0;
    write(fd, &reg_addr, 1);
    res = read(fd, data, 1);
}
/*
// functions：write at specified address
// parameters：reg_addr： register address，buf：Data buffer
// return: The length of the actual write
*/
int i2c_writeReg(uint8_t reg_addr, uint8_t data)
{
    int res = 0, i;
    uint8_t *buff = 0;
    buff = (uint8_t *)malloc(2);
    buff[0] = reg_addr;
    buff[1] = data;
    res = write(fd, buff, 2);
    free(buff);
    return res;
}
/*
// functions：set frequency
// parameters：freq：value
// return：no
*/
void PCA9685_setPWMFreq(float freq)
{
    freq *= 0.8449;               // Correct the overshoot of the frequency setting
    float prescaleval = 25000000; // 25MHz
    prescaleval /= 4096;          // 4096 PWM cycles
    prescaleval /= freq;          // Calculate the frequency division value
    prescaleval -= 1;
    float prescale = (float)(prescaleval + 0.5);
    uint8_t oldmode;
    i2c_readReg(PCA9685_MODE1, &oldmode);
    uint8_t newmode = (oldmode & 0x7F) | 0x10;
    i2c_writeReg(PCA9685_MODE1, newmode);              // sleep
    i2c_writeReg(PCA9685_PRESCALE, (uint8_t)prescale); // Set predivider
    i2c_writeReg(PCA9685_MODE1, oldmode);
    usleep(5000);
    i2c_writeReg(PCA9685_MODE1, oldmode | 0xa0); // Set the MODE1 register to turn on autoincrement
}
/*
// functions：Set PWM value
// parameters：num：channel number，on：duty cycle high bytes，off： duty cycle low bytes
// return：no
*/
void PCA9685_setPWM(uint8_t num, uint16_t on, uint16_t off)
{
    i2c_writeReg(LED0_ON_L + 4 * num, on);
    i2c_writeReg(LED0_ON_L + 4 * num + 1, (on >> 8));
    i2c_writeReg(LED0_ON_L + 4 * num + 2, off);
    i2c_writeReg(LED0_ON_L + 4 * num + 3, off >> 8);
}
/*
// functions：Set servo pulse
// parameters：num：channel number，pulse：pulse value
// return：no
*/
void setServoPulse(uint8_t n, double pulse)
{
    double pulselength;
    pulselength = 1000; // 1,000 ms per second
    pulselength /= 60;  // 60 Hz
    pulselength /= 4096;
    pulse *= 1000; // ms
    pulse /= pulselength;
    PCA9685_setPWM(n, 0, pulse);
}
/*
// functions：Set the Angle of rotation of the servo
// parameters：n：channel number，Degree：angle value
// return：no
*/
void setServoDegree(uint8_t n, uint8_t Degree)
{
    if (Degree >= 180)
    {
        Degree = 180;
    }
    else if (Degree <= 0)
    {
        Degree = 0;
    }
    double pulse = (Degree + 45) / (90.0 * 1000);
    setServoPulse(n, pulse);
}
/*
// functions：getting key values
// parameters：no
// return：  key values
*/
int get_key_board_from_termios()
{
    int key_value;
    struct termios new_config;
    struct termios old_config;

    tcgetattr(0, &old_config);
    new_config = old_config;
    new_config.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &new_config);
    key_value = getchar();
    tcsetattr(0, TCSANOW, &old_config);
    return key_value;
}
/*
// functions：Increases the degree of the specified channel
// parameters：channel：通道号，step：步数
// retrun：   key values
*/
int ServoDegreeIncrease(uint8_t Channel, uint8_t Step)
{
    switch (Channel)
    {
    case SERVO_UP_CH:
        if (ServoUpDegree >= SERVO_UP_MAX)
        {
            ServoUpDegree = SERVO_UP_MAX;
            setServoDegree(Channel, ServoUpDegree);
        }
        else
        {
            ServoUpDegree += Step;
            setServoDegree(Channel, ServoUpDegree);
        }
        break;
    case SERVO_DOWN_CH:
        if (ServoDownDegree >= SERVO_DOWN_MAX)
        {
            ServoDownDegree = SERVO_DOWN_MAX;
            setServoDegree(Channel, ServoDownDegree);
        }
        else
        {
            ServoDownDegree += Step;
            setServoDegree(Channel, ServoDownDegree);
        }
        break;
    default:
        return 1;
        break;
    }
    usleep(STEP_DELAY * 1000);
    return 0;
}
/*
// functions：Increases the degree of the specified channel
// parameters：channel：通道号，step：步数
// retrun：   key values
*/
int ServoDegreeDecrease(uint8_t Channel, uint8_t Step)
{
    switch (Channel)
    {
    case SERVO_UP_CH:
        if (ServoUpDegree <= SERVO_UP_MIN + Step)
        {
            ServoUpDegree = SERVO_UP_MIN;
            setServoDegree(Channel, ServoUpDegree);
        }
        else
        {
            ServoUpDegree -= Step;
            setServoDegree(Channel, ServoUpDegree);
        }
        break;
    case SERVO_DOWN_CH:
        if (ServoDownDegree <= SERVO_DOWN_MIN + Step)
        {
            ServoDownDegree = SERVO_DOWN_MIN;
            setServoDegree(Channel, ServoDownDegree);
        }
        else
        {
            ServoDownDegree -= Step;
            setServoDegree(Channel, ServoDownDegree);
        }
        break;
    default:
        return 1;
        break;
    }
    usleep(STEP_DELAY * 1000);
    return 0;
}
/*
// functions：keys control the steering gear rotation
// parameters：no
// retrun：no
*/
void processKeyboardEvent(void)
{
    int keyVal = 0;
    while (1)
    {
        usleep(100);
        tcflush(0, TCIOFLUSH);
        keyVal = get_key_board_from_termios();
        printf("%d ", keyVal);

        if (keyVal == 27)
        {
            keyVal = get_key_board_from_termios();
            printf("%d ", keyVal);
            if (keyVal == 91)
            {
                keyVal = get_key_board_from_termios();
                printf("%d\n", keyVal);
                switch (keyVal)
                {
                case 65 /* up */:
                    ServoDegreeIncrease(SERVO_UP_CH, STEP);
                    break;
                case 66 /* down */:
                    ServoDegreeDecrease(SERVO_UP_CH, STEP);
                    break;
                case 67 /* right */:
                    ServoDegreeIncrease(SERVO_DOWN_CH, STEP);
                    break;
                case 68 /* left */:
                    ServoDegreeDecrease(SERVO_DOWN_CH, STEP);
                    break;
                default:
                    break;
                }
            }
        }
    }
}

// ROS Client library
#include "rclcpp/rclcpp.hpp"

// Twist library - expresses velocity in terms of linear and angular parts
#include "geometry_msgs/msg/twist.hpp"

// Utility that launches pigpio libaray as a daemon in ROS to control GPIO's
#include <pigpiod_if2.h>

// Pan Tilt library
// #include "PCA9685.h"

#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4

class PanTilt
{
public:
    PanTilt()
    {
        char i2c_dev[32];
        snprintf(i2c_dev, sizeof(i2c_dev), "/dev/i2c-1");
        i2c_init(i2c_dev, I2C_ADDR);
        i2c_writeReg(MODE1, 0x80);
        usleep(10000);
        PCA9685_setPWMFreq(1000); // Set the default frequency
        PCA9685_setPWMFreq(60);
        setServoDegree(SERVO_UP_CH, ServoUpDegree);
        setServoDegree(SERVO_DOWN_CH, ServoDownDegree);
    }

    void update(int direction)
    {
        switch (direction)
        {
        case UP:
            ServoDegreeIncrease(SERVO_UP_CH, STEP);
            break;
        case DOWN:
            ServoDegreeDecrease(SERVO_UP_CH, STEP);
            break;
        case RIGHT:
            ServoDegreeIncrease(SERVO_DOWN_CH, STEP);
            break;
        case LEFT:
            ServoDegreeDecrease(SERVO_DOWN_CH, STEP);
            break;
        default:
            break;
        }
    }
};

class PanTiltSubscriber : public rclcpp::Node
{
private:
    // Twist msg for holding values from controllerNode
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr subscription_;
    PanTilt servos;

    /**
     * @brief Functor for processing data published to cmd_vel
     *
     * @param msg Twist message published by the controllerNode
     */
    void twistCallback(const geometry_msgs::msg::Twist::SharedPtr msg)
    {
        // Process received Twist message
        RCLCPP_INFO(this->get_logger(), "linear.x=%f, angular.z=%f, linear.z=%f",
                    msg->linear.x, msg->angular.z, msg->linear.z);

        int direction = msg->linear.z;
        servos.update(direction);
    }

public:
    /**
     * @brief Construct ROS subscriber Node for Pan Tilt
     *
     */
    PanTiltSubscriber() : Node("PanTiltSubscriber")
    {
        subscription_ = this->create_subscription<geometry_msgs::msg::Twist>(
            "cmd_vel", 10, std::bind(&PanTiltSubscriber::twistCallback, this, std::placeholders::_1));
    }
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<PanTiltSubscriber>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}