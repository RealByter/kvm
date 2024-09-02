#include "components/ps2.h"
#include "common.h"

#define PS2_DATA_PORT 0x60
#define PS2_STATUS_COMMAND_PORT 0x64

#define PS2_STATUS_OUT_BUF (1 << 0)
#define PS2_STATUS_IN_BUF (1 << 1)
#define PS2_STATUS_SYS_FLAG (1 << 2)
#define PS2_STATUS_COMMAND_DATA (1 << 3)
#define PS2_STATUS_TIMEOUT (1 << 6)
#define PS2_STATUS_PARITY (1 << 7)

// I'm skipping multiple commands I don't see relevant
#define PS2_COMMAND_READ_CONFIG_BYTE 0x20
#define PS2_COMMAND_WRITE_CONFIG_BYTE 0x60
#define PS2_COMMAND_DISABLE_SECOND_PORT 0xA7
#define PS2_COMMAND_ENABLE_SECOND_PORT 0xA8
#define PS2_COMMAND_SELF_TEST 0xAA
#define PS2_COMMAND_TEST_FIRST_PORT 0xAB
#define PS2_COMMAND_DISABLE_FIRST_PORT 0xAD
#define PS2_COMMAND_ENABLE_FIRST_PORT 0xAE
#define PS2_COMMAND_READ_OUTPUT_PORT 0xD0
#define PS2_COMMAND_WRITE_OUTPUT_PORT 0xD1

enum ps2_data_expect {
    PS2_DATA_NOTHING,
    PS2_DATA_CONFIG_BYTE,
    PS2_DATA_OUTPUT_PORT
};

static uint8_t ps2_status = 0b100;
static uint8_t ps2_config_byte = 1; // First PS/2 port interrupt (i think should be enabled)
static uint8_t ps2_output_port = 1; // System reset - should always be set to 1
static enum ps2_data_expect ps2_data_expect = PS2_DATA_NOTHING;

void ps2_init()
{

}

static void ps2_handle_command(exit_io_info_t* io, uint8_t* base)
{
    switch(base[io->data_offset])
    {
        case PS2_COMMAND_READ_CONFIG_BYTE:
            base[io->data_offset] = ps2_config_byte;
            break;
        case PS2_COMMAND_WRITE_CONFIG_BYTE:
            ps2_data_expect = PS2_DATA_CONFIG_BYTE;
            break;
        case PS2_COMMAND_DISABLE_SECOND_PORT:
            ps2_config_byte &= ~(1 << 1);
            break;
        case PS2_COMMAND_ENABLE_SECOND_PORT:
            ps2_config_byte |= (1 << 1);
            break;
        case PS2_COMMAND_SELF_TEST:
            base[io->data_offset] = 0x55;
            break;
        case PS2_COMMAND_TEST_FIRST_PORT:
            base[io->data_offset] = 0x00;
            break;
        case PS2_COMMAND_DISABLE_FIRST_PORT:
            ps2_config_byte &= ~(1 << 0);
            break;
        case PS2_COMMAND_ENABLE_FIRST_PORT:
            ps2_config_byte |= (1 << 0);
            break;
        case PS2_COMMAND_READ_OUTPUT_PORT:
            base[io->data_offset] = ps2_output_port;
            break;
        case PS2_COMMAND_WRITE_OUTPUT_PORT:
            ps2_data_expect = PS2_DATA_OUTPUT_PORT;
            break;
        default:
            unhandled(io, base);
    }
}

static void ps2_handle_data(exit_io_info_t* io, uint8_t* base)
{
    switch(ps2_data_expect)
    {
        case PS2_DATA_CONFIG_BYTE:
            ps2_config_byte = base[io->data_offset];
            break;
        case PS2_DATA_OUTPUT_PORT:
            ps2_output_port = base[io->data_offset];
            break;
        default:
            unhandled(io, base);
    }
}

void ps2_handle(exit_io_info_t* io, uint8_t* base)
{
    if(io->port == PS2_STATUS_COMMAND_PORT)
    {
        if(io->direction == EXIT_IO_IN)
        {
            base[io->data_offset] = ps2_status;
            return;
        }
        else
        {
            ps2_handle_command(io, base);
            return;
        }
    }
    else if(io->port == PS2_DATA_PORT)
    {
        if(io->direction == EXIT_IO_OUT)
        {
            ps2_handle_data(io, base);
            return;
        }
    }
    
    unhandled(io, base);
}