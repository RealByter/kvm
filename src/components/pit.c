#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "components/pit.h"
#include "common.h"
#include "components/pic.h"
#include "log.h"

LOG_DEFINE("pit");

typedef struct
{
    uint16_t divisor;
    uint16_t count;
    enum
    {
        PIT_LATCH,
        PIT_LOW,
        PIT_HIGH,
        PIT_BOTH
    } access_mode;
    bool expecting_low;
    enum
    {
        PIT_RATE_GENERATOR1 = 0b010,
        PIT_RATE_GENERATOR2 = 0b110,
    } operating_mode;
} pit_channel_t;

#define PIT_CLOCK_FREQUENCY_HZ 1193182
#define PIT_COMMAND 0x43
#define PIT_CHANNEL_BASE 0x40
#define PIT_CHANNEL_NUM 3
static pit_channel_t pit_channels[PIT_CHANNEL_NUM] = {{0, 0, 0, 0, 0}};
static pthread_t pit_thread = NULL;

void pit_clock_thread(void *arg)
{
    pit_channel_t *pit;
    struct timespec sleep_time;
    sleep_time.tv_sec = 0;
    sleep_time.tv_nsec = 1000000000 / PIT_CLOCK_FREQUENCY_HZ; // nanoseconds per clock tick

    while (1)
    {
        nanosleep(&sleep_time, NULL);
        for (int i = 0; i < PIT_CHANNEL_NUM; i++)
        {
            pit = &pit_channels[i];
            if (pit->operating_mode != PIT_RATE_GENERATOR1 && pit->operating_mode != PIT_RATE_GENERATOR2)
            {
                continue;
            }
            if (pit->divisor == 1)
            {
                continue;
            }
            if (pit->count <= 1)
            {
                pic_raise_interrupt(PIC_IRQ0);
            }
            if (pit->count <= 0)
            {
                pit->count = pit->divisor - 1;
            }
            pit->count--;
        }
    }

    return NULL;
}

void pit_init()
{
    if (pit_thread == NULL)
    {
        if (pthread_create(&pit_thread, NULL, pit_clock_thread, NULL) != 0)
        {
            errx(1, "Failed to create pit clock thread");
        }
    }
    for(int i = 0; i < PIT_CHANNEL_NUM; i++)
    {
        pit_channels[i].divisor = 1; // invalid since the count is set to divisor - 1 which will result in 0
    }
}

void pit_handle(exit_io_info_t *io, uint8_t *base)
{
    if (io->port >= PIT_CHANNEL_BASE && io->port < (PIT_CHANNEL_BASE + PIT_CHANNEL_NUM - 1))
    {
        pit_channel_t *channel = &pit_channels[io->port - PIT_CHANNEL_BASE];
        if (io->direction == EXIT_IO_IN)
        {
            // base[io->data_offset] = channel->divisor;
            unhandled(io, base);
        }
        else
        {
            if (channel->expecting_low && channel->access_mode == PIT_BOTH || channel->access_mode == PIT_LOW)
            {
                LOG_MSG("data: %x", base[io->data_offset]);
                channel->divisor = (channel->divisor & 0xFF00) | (base[io->data_offset] & 0xFF);
                if (channel->access_mode == PIT_BOTH)
                {
                    channel->expecting_low = false;
                }
                LOG_MSG("divisor low: %x", channel->divisor);
            }
            else if (channel->access_mode == PIT_BOTH || channel->access_mode == PIT_HIGH)
            {
                LOG_MSG("data: %x", base[io->data_offset]);
                channel->divisor = (channel->divisor & 0x00FF) | ((base[io->data_offset] & 0xFF) << 8);
                if (channel->access_mode == PIT_BOTH)
                {
                    channel->expecting_low = true;
                }
                LOG_MSG("divisor high: %x", channel->divisor);
            }
            else
            {
                unhandled(io, base);
            }
        }
    }
    else if (io->port == PIT_COMMAND)
    {
        if (io->direction == EXIT_IO_OUT)
        {
            uint8_t data = base[io->data_offset];
            pit_channel_t *channel = &pit_channels[GET_BITS(data, 6, 2)];
            channel->access_mode = GET_BITS(data, 4, 2);
            channel->operating_mode = GET_BITS(data, 1, 3);
            if (channel->access_mode == PIT_LATCH)
            {
                unhandled(io, base);
            }
            if (channel->access_mode == PIT_HIGH)
            {
                channel->expecting_low = false;
            }
            else
            {
                channel->expecting_low = true;
            }
            // dont care about the last since it's always going to be 0 "16-bit binary" instead of 1 "four-digit BCD"
        }
        else
        {
            unhandled(io, base);
        }
    }
}