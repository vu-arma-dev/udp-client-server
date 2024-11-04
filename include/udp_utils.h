#ifndef UDP_UTILS_H
#define UDP_UTILS_H

// Includes
#include <chrono>
#include <cstring>
#include <udp_client_server.h>

/**
* Provides utility typedefs and static functions for sending and receiving UDP data.
* @details A wrapper for udp_client_server that provides a more intuitive user interface and additional functionality 
* @see udp_client_server 
* @author Garrison Johnston 
*/
class UdpUtils
{
public:
    /**
    * I can never remember the Server vs. Client notation so just renaming Server to Receive here. 
    */
    typedef udp_client_server::UdpServer UdpReceive;

    /**
    * I can never remember the Server vs. client notation so just renaming Client to Send here
    */
    typedef udp_client_server::UdpClient UdpSend;

    /**
    * Static function for reading UDP. Will try to read udp until 1.2*1/udp_rate time has elapsed
    * @tparam T Should be an std::array/vector of chars with the number of elements = number of 
    * expected bytes.
    * @param recv_packet An std::array/vector of chars with the number of elements = number of 
    * expected bytes. If successful, the function will change the values in this array/vector
    * @param udp_rate The target UDP rate in Hz.
    * @param udp_recv A reference to a UdpReceive (aka udp_client_server::UdpServer) object.
    * @return True if there is an UDP error, False otherwise
    */
    template< typename T> 
    static bool readUdp(T &recv_packet, const uint16_t udp_rate, UdpReceive &udp_recv);

};

template< typename T> 
bool UdpUtils::readUdp(T &recv_packet, const uint16_t udp_rate, UdpReceive &udp_recv)
{
    T recv_packet_temp = {0};

    //1 if there is an UDP error, 0 otherwise
    bool udp_error = 1;

    //bytes received within the loop
    int16_t bytes_received = 0;

    //UDP packet sequence number. We seek the packet with the highest sequence number.
    uint32_t highest_sequence_number = 0;

    //sequence number within the loop
    uint32_t sequence_number = 0;

    //try to read from the udp buffer for a specified timeout time, then give up
    auto start = std::chrono::steady_clock::now();
    uint16_t elapsed_time_us = 0;
    uint16_t timeout_us = 1.2 * (1000000.0 / udp_rate); //1.2 times the udp target rate

    // Loop for the timeout period
    while (elapsed_time_us < timeout_us)
    {
        // Receive UDP data
        bytes_received = udp_recv.recv(recv_packet_temp.begin(), sizeof(recv_packet_temp));
        
        /* Verify that the udp_response_id sent matches the udp_response_id received.
        Two conditions a) response_id (e.g. 0x01) matches,  and b) corresponding number of byte match (e.g. 55)*/
        if (bytes_received == recv_packet.size())
        {
            udp_error = 0; // we now have at least one good packet we can return

            //get the sequence number
            std::memcpy(&sequence_number, &recv_packet_temp[0], 4); //uint32 has 4 bytes

            // if this new packet is more recent, replace it
            if (sequence_number > highest_sequence_number)
            {
                recv_packet.swap(recv_packet_temp);
            }
        }

        //quit searching when we have a good packet and we're no longer reading packets
        if ((!udp_error) && (bytes_received == -1))
        {
            break;
        }

        //update elapsed time
        auto end = std::chrono::steady_clock::now();
        elapsed_time_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    }

    return udp_error;
}


#endif 