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
    bool null_count;
    uint16_t latched_count;
    bool latch_count;
    bool latch_status;
    bool output_pin_state;
} pit_channel_t;

#define PIT_CLOCK_FREQUENCY_HZ 1193182
#define PIT_COMMAND 0x43
#define PIT_CHANNEL_BASE 0x40
#define PIT_CHANNEL_NUM 3
static pit_channel_t pit_channels[PIT_CHANNEL_NUM] = {{0, 0, 0, 0, 0}};
static pthread_t pit_thread = NULL;
static uint8_t read_back;

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
            if (pit->count == 0 && pit->null_count)
            {
                continue;
            }
            pit->output_pin_state ^= 1; // flip the state each cycle
            if (pit->count == 1)
            {
                pic_raise_interrupt(PIC_IRQ0);
            }
            if (pit->count == 0)
            {
                pit->count = pit->divisor - 1;
                pit->null_count = false;
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
    for (int i = 0; i < PIT_CHANNEL_NUM; i++)
    {
        pit_channels[i].divisor = 1; // invalid since the count is set to divisor - 1 which will result in 0
        pit_channels[i].null_count = true;
    }
}

void pit_handle(exit_io_info_t *io, uint8_t *base)
{
    // LOG_MSG("Handling pit port: 0x%x, direction: 0x%x, size: 0x%x, count: 0x%x, data: 0x%lx", io->port, io->direction, io->size, io->count, base[io->data_offset]);
    if (io->port >= PIT_CHANNEL_BASE && io->port < (PIT_CHANNEL_BASE + PIT_CHANNEL_NUM - 1))
    {
        pit_channel_t *channel = &pit_channels[io->port - PIT_CHANNEL_BASE];
        if (io->direction == EXIT_IO_IN)
        {
            // uint8_t channel_num = io->port - PIT_CHANNEL_BASE;
            // if (!(read_back & (1 << (1 + channel_num)))) // if the bit of the timer channel for the current channel is not set (bit 0 is zero)
            // {
            //     unhandled(io, base);
            // }
            // pit_channel_t *channel = &pit_channels[channel_num];
            // uint8_t last_command = 0 | channel->operating_mode << 1 | channel->access_mode << 4;
            // base[io->data_offset] = last_command | channel->null_count << 6;
            // bit 7 should also be handled but I think it's not relevant for the rate generator

            uint16_t count_value;
            if (channel->latch_count)
            {
                count_value = channel->latched_count;
                channel->latch_count = false;
            }
            else
            {
                count_value = channel->null_count ? channel->divisor : channel->count;
            }

            if (channel->access_mode == PIT_LOW || (channel->access_mode == PIT_BOTH && channel->expecting_low))
            {
                base[io->data_offset] = count_value & 0xFF;
                LOG_MSG("low count: %x", base[io->data_offset]);
                channel->expecting_low = false;
            }
            else if (channel->access_mode == PIT_HIGH || (channel->access_mode == PIT_BOTH && !channel->expecting_low))
            {
                base[io->data_offset] = (count_value >> 8) & 0xFF;
                LOG_MSG("high count: %x", base[io->data_offset]);
                channel->expecting_low = true;
            }
            else if (channel->latch_status)
            {
                uint8_t status = (channel->output_pin_state << 7) |
                                 (channel->null_count << 6) |
                                 (channel->access_mode << 4) |
                                 (channel->operating_mode << 1); // bit 0 is always zero for 8086
                base[io->data_offset] = status;
                channel->latch_status = false;
            }
            // LOG_MSG("data: %x, count: %x", base[io->data_offset], count_value);
        }
        else
        {
            if (channel->expecting_low && channel->access_mode == PIT_BOTH || channel->access_mode == PIT_LOW)
            {
                // LOG_MSG("data: %x", base[io->data_offset]);
                channel->divisor = (channel->divisor & 0xFF00) | (base[io->data_offset] & 0xFF);
                LOG_MSG("Set divisor low: %x", channel->divisor);
                channel->null_count = true;
                if (channel->access_mode == PIT_BOTH)
                {
                    channel->expecting_low = false;
                }
                else
                {
                    channel->count = channel->divisor - 1;
                    channel->null_count = false;
                }
                // LOG_MSG("divisor low: %x", channel->divisor);
            }
            else if (channel->access_mode == PIT_BOTH || channel->access_mode == PIT_HIGH)
            {
                // LOG_MSG("data: %x", base[io->data_offset]);
                channel->divisor = (channel->divisor & 0x00FF) | ((base[io->data_offset] & 0xFF) << 8);
                LOG_MSG("Set divisor high: %x", channel->divisor);
                channel->null_count = true;
                if (channel->access_mode == PIT_BOTH)
                {
                    channel->expecting_low = true;
                }
                channel->count = channel->divisor - 1;
                channel->null_count = false;
                // LOG_MSG("divisor high: %x", channel->divisor);
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
            uint8_t channel_num = GET_BITS(data, 6, 2);
            if (channel_num != 0b11)
            {
                pit_channel_t *channel = &pit_channels[channel_num];
                channel->access_mode = GET_BITS(data, 4, 2);
                channel->operating_mode = GET_BITS(data, 1, 3);
                if (channel->access_mode == PIT_LATCH)
                {
                    channel->latched_count = channel->null_count ? channel->divisor : channel->count;
                    channel->latch_count = true;
                }
                if (channel->access_mode == PIT_HIGH)
                {
                    channel->expecting_low = false;
                }
                else
                {
                    channel->expecting_low = true;
                }
            }
            else
            {
                pit_channel_t *channel = NULL;
                for (int i = 0; i < PIT_CHANNEL_NUM; i++)
                {
                    if (GET_BIT(data, i + 1)) // channel
                    {
                        channel = &pit_channels[i];
                        if (!GET_BIT(data, 5)) // latch count
                        {
                            channel->latched_count = channel->null_count ? channel->divisor : channel->count;
                            channel->latch_count = true;
                        }
                        if (!GET_BIT(data, 4)) // latch status
                        {
                            channel->latch_status = true;
                        }
                    }
                }
            }
            // dont care about the last since it's always going to be 0 "16-bit binary" instead of 1 "four-digit BCD"
        }
        else
        {
            unhandled(io, base);
        }
    }
}