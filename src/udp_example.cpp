#ifndef UDP_EXAMPLE
#define UDP_EXAMPLE

#include <array>
#include <udp_client_server.h>
#include <udp_utils.h>

/**
 * A windows example of using udp_utils. It sends and receives two random floats
 * @see udp_client_server
 * @see udp_utils
 * @author Garrison Johnston
 */
class UdpExample
{
    /** 
    * Min UDP rate in Hz 
    */
    static constexpr uint16_t UDP_RATE = 200;

    /**
    * Number of floats we expect in each message
    */
    static constexpr uint8_t NUM_FLOATS = 2;

    /**
    * The number of bytes we expect to receive per message
    */ 
    static const uint8_t PACKET_SIZE = sizeof(float)*NUM_FLOATS;

    /**
    * Array used for sending/receiving UDP packets on either side of the UDP interface.
    * The size of a received UDP packet cannot be known a priori, so we allocate the max size.
    */
    typedef std::array<char, PACKET_SIZE> UdpPacket;

    /**
    * An array that holds the sent/received floats 
    */
    typedef std::array<float, NUM_FLOATS> UdpData;

public:

    // Delete Default Constructor
    UdpExample() = delete;

    /**
    * Constructor
    * @param local_ip The ip address of this computer
    * @param local_port The UDP port to listen on
    * @param remote_ip The ip address of the computer sending UDP to this computer  
    * @param remote_port The port to send to  
    */
    UdpExample(const std::string &local_ip, uint16_t local_port, 
        const std::string &remote_ip, uint16_t remote_port) 
        :
        udp_recv_(local_ip, local_port),
        udp_send_(remote_ip, remote_port) {}

    /**
    * Default destructor 
    */
    ~UdpExample() = default;

    /**
    * Read UDP Data. Expects two floats 
    */
    void readUDP()
    {
        // UdpUtils::readUdp will return true if there is an error 
        if(!UdpUtils::readUdp<UdpPacket>(receive_packet_, UDP_RATE, udp_recv_))
        {
            float temp; // temp variable to hold each float
            uint8_t j = 0; // Number of floats counter

            // Convert byte array to floats
            for (uint8_t i = 0; i < receive_packet_.size(); i+=sizeof(float))
            {
                // The UDP data is received in char format. Need to convert to float
                std::memcpy(&temp, &receive_packet_[i], sizeof(temp));

                // Assign to array
                receive_data_[j] = temp;

                // Increment the float counter 
                ++j;
            }

            std::cout << "Message Received. First float: " << receive_data_[0] << " Second Float: " <<  receive_data_[1] << std::endl;
        }
        else
        {
            std::cout << "UDP Timeout! " << std::endl;
        }
    }

    /**
    * Send UDP. Sends two random floats. 
    */
    void sendUDP()
    {
        // Generate two random numbers
        send_data_[0] = rand(); send_data_[1] = rand();

        std::cout << "Message Sent. First float: " << send_data_[0] << " Second Float: " <<  send_data_[1] << std::endl;

        uint8_t j = 0; // Char counter 

        // Convert each float to a byte array
        for (uint8_t i = 0; i < send_data_.size(); ++i)
        {
            // Convert the float to a char array
            std::memcpy(&send_packet_[j], &send_data_[i] , sizeof(send_data_[i]));

            // Increment the byte counter
            j += sizeof(float);
        }

        // Send data
        udp_send_.send(&send_packet_[0], PACKET_SIZE);
    }
private:
    // UDP Send object
    UdpUtils::UdpSend udp_send_;

    // UDP Receive Object
    UdpUtils::UdpReceive udp_recv_;
    
    // UDP packet for received byte array
    UdpPacket receive_packet_;

    // UDP packet for sending byte array
    UdpPacket send_packet_;

    //array of floats to send
    UdpData send_data_;

    // array of received floats
    UdpData receive_data_;
};

int main(int argc, char *argv[])
{
    int remote_port = 1001;
    int local_port = 1001; 
    std::string local_ip = "192.168.1.101";
    std::string remote_ip = "192.168.1.101";
    
    std::cout << "Creating UDP Object..." << std::endl;
    UdpExample udp_example(local_ip, local_port, remote_ip, remote_port);
    std::cout << "UDP Object Created!" << std::endl;
    

    while (1)
    {
        std::cout << "Sending UDP..." << std::endl;
        udp_example.sendUDP();
        std::cout << "Receiving UDP..." << std::endl;
        udp_example.readUDP();
    }
    
    return 0;
}

#endif 