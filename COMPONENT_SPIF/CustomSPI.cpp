#include "CustomSPI.h"

int CustomSPI::write_with_lock(const char *tx_buffer, int tx_length, char *rx_buffer, int rx_length) {

    lock();
    _acquire();
    int ret = spi_master_block_write(&_spi, tx_buffer, tx_length, rx_buffer, rx_length, _write_fill);

    return ret;
}


int CustomSPI::write_with_unlock(const char *tx_buffer, int tx_length, char *rx_buffer, int rx_length) {

    _acquire();
    int ret = spi_master_block_write(&_spi, tx_buffer, tx_length, rx_buffer, rx_length, _write_fill);
    unlock();

    return ret;
}

int CustomSPI::write_without_mutex(const char *tx_buffer, int tx_length, char *rx_buffer, int rx_length) {

    _acquire();
    int ret = spi_master_block_write(&_spi, tx_buffer, tx_length, rx_buffer, rx_length, _write_fill);
    return ret;

}

int CustomSPI::write_with_lock(int value)
{
    lock();
    _acquire();
    int ret = spi_master_write(&_spi, value);
    return ret;
}

int CustomSPI::write_with_unlock(int value)
{
    _acquire();
    int ret = spi_master_write(&_spi, value);
    unlock();
    return ret;
}

// Note: Private function with no locking
void CustomSPI::_acquire()
{
    if (_owner != this) {
        spi_format(&_spi, _bits, _mode, 0);
        spi_frequency(&_spi, _hz);
        _owner = this;
    }
}
