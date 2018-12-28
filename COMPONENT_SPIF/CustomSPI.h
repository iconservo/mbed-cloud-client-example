#ifndef CUSTOM_SPI_H
#define CUSTOM_SPI_H

#include "drivers/SPI.h"
#include "platform/mbed_critical.h"
#include "mbed.h"

class CustomSPI : public SPI {

    public:

        friend class SPI;

        int write_with_lock(const char *tx_buffer, int tx_length, char *rx_buffer, int rx_length);

        int write_with_unlock(const char *tx_buffer, int tx_length, char *rx_buffer, int rx_length);

        int write_without_mutex(const char *tx_buffer, int tx_length, char *rx_buffer, int rx_length);

        int write_with_lock(int value);

        int write_with_unlock(int value);


    protected:
        
        void _acquire(void);

   
};



#endif