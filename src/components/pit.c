#include <stdbool.h>
#include "components/pit.h"
#include "common.h"

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

#define PIC_FREQUENCY 1193182
#define PIT_COMMAND 0x43
#define PIT_CHANNEL_BASE 0x40
#define PIT_CHANNEL_NUM 3
static pit_channel_t pit_channels[PIT_CHANNEL_NUM] = {{0, 0, 0, 0, 0}};

void pit_handle(exit_io_info_t *io, uint8_t *base)
{
    if (io->port >= PIT_CHANNEL_BASE && io->port < (PIT_CHANNEL_BASE + PIT_CHANNEL_NUM - 1))
    {
        pit_channel_t *channel = &pit_channels[io->port - PIT_CHANNEL_BASE];
        if (io->direction == EXIT_IO_IN)
        {
            // base[io->data_offset] = channel->divisor;
            unhandled(io);
        }
        else
        {
            if (channel->expecting_low && channel->access_mode == PIT_BOTH || channel->access_mode == PIT_LOW)
            {
                channel->divisor = (channel->divisor & 0xFF00) | (base[io->data_offset] & 0xFF);
                if (channel->access_mode == PIT_BOTH)
                {
                    channel->expecting_low = false;
                }
            }
            else if (channel->access_mode == PIT_BOTH || channel->access_mode == PIT_HIGH)
            {
                channel->divisor = (channel->divisor & 0x00FF) | ((base[io->data_offset] & 0xFF) << 8);
                if (channel->access_mode == PIT_BOTH)
                {
                    channel->expecting_low = true;
                }
            }
            else
            {
                unhandled(io);
            }
        }
    }
    else if (io->port == PIT_COMMAND)
    {
        if (io->direction == EXIT_IO_OUT)
        {
            uint8_t data = base[io->data_offset];
            pit_channel_t* channel = &pit_channels[GET_BITS(data, 6, 2)];
            channel->access_mode = GET_BITS(data, 4, 2);
            channel->operating_mode = GET_BITS(data, 1, 3);
            if(channel->access_mode == PIT_LATCH)
            {
                unhandled(io);
            }
            if(channel->access_mode == PIT_HIGH)
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
            unhandled(io);
        }
    }
}