#include "components/ps2.h"
#include "common.h"
#include "log.h"
#include "components/pic.h"
#include <stdbool.h>

LOG_DEFINE("ps2")

#define PS2_DATA_PORT 0x60
#define PS2_STATUS_COMMAND_PORT 0x64

#define PS2_STATUS_OUT_BUF (1 << 0)
#define PS2_STATUS_IN_BUF (1 << 1)
#define PS2_STATUS_SYS_FLAG (1 << 2)
#define PS2_STATUS_COMMAND_DATA (1 << 3)
#define PS2_STATUS_TIMEOUT (1 << 6)
#define PS2_STATUS_PARITY (1 << 7)

#define PS2_CONFIG_FIRST_INTERRUPT (1 << 0)
#define PS2_CONFIG_SENT_TO_DEVICE (1 << 5) // should be 6 but from seabios it makes sense to be 5

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

#define PS2_DEVICE_SET_SCANCODE 0xF0
#define PS2_DEVICE_ENABLE_SCANNING 0xF4
#define PS2_DEVICE_DISABLE_SCANNING 0xF5
#define PS2_DEVICE_RESET 0xFF

#define PS2_RESPONSE_ACK 0xFA

enum ps2_data_expect
{
    PS2_DATA_NOTHING,
    PS2_DATA_CONFIG_BYTE,
    PS2_DATA_OUTPUT_PORT
};

// static uint8_t ps2_status = 0b100;
static uint8_t ps2_status = PS2_STATUS_SYS_FLAG;
static uint8_t ps2_config_byte = 1; // First PS/2 port interrupt (i think should be enabled)
static uint8_t ps2_output_port = 1; // System reset - should always be set to 1
static enum ps2_data_expect ps2_data_expect = PS2_DATA_NOTHING;
static uint8_t ps2_response = 0;
static bool ps2_response_pending;

#define PS2_RESPONSE_QUEUE_SIZE 10
static uint8_t ps2_response_queue[PS2_RESPONSE_QUEUE_SIZE];
static int ps2_response_queue_head = 0;
static int ps2_response_queue_tail = 0;

void ps2_init()
{
}

static void ps2_update_status()
{
    if (ps2_response_pending)
    {
        ps2_status |= PS2_STATUS_OUT_BUF;
    }
    else
    {
        ps2_status &= ~PS2_STATUS_OUT_BUF;
    }

    // if (ps2_data_expect != PS2_DATA_NOTHING)
    // {
    //     ps2_status |= PS2_STATUS_IN_BUF;
    // }
    // else
    // {
    //     ps2_status &= ~PS2_STATUS_IN_BUF;
    // }
}

static void ps2_queue_response(uint8_t response)
{
    ps2_response_queue[ps2_response_queue_tail] = response;
    ps2_response_queue_tail = (ps2_response_queue_tail + 1) % PS2_RESPONSE_QUEUE_SIZE;
    ps2_response_pending = true;
    ps2_update_status();
    LOG_MSG("queued response: 0x%x, head: %d, tail: %d", response, ps2_response_queue_head, ps2_response_queue_tail);
}

static uint8_t ps2_dequeue_response()
{
    if (ps2_response_queue_head == ps2_response_queue_tail)
    {
        return 0xFF;
    }

    uint8_t response = ps2_response_queue[ps2_response_queue_head];
    ps2_response_queue_head = (ps2_response_queue_head + 1) % PS2_RESPONSE_QUEUE_SIZE;
    ps2_response_pending = (ps2_response_queue_head != ps2_response_queue_tail);
    ps2_update_status();
    LOG_MSG("dequeued response: 0x%x, head: %d, tail: %d", response, ps2_response_queue_head, ps2_response_queue_tail);
    return response;
}

static void ps2_generate_interrupt()
{
    if (ps2_config_byte & PS2_CONFIG_FIRST_INTERRUPT)
    {
        pic_raise_interrupt(PIC_IRQ1);
    }
}

static void ps2_handle_command(exit_io_info_t *io, uint8_t *base)
{
    switch (base[io->data_offset])
    {
    case PS2_COMMAND_READ_CONFIG_BYTE:
        ps2_queue_response(ps2_config_byte);
        ps2_generate_interrupt();
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
        ps2_status &= ~(1 << 2); // clear system flag (bit 2)
        ps2_queue_response(0x55);
        ps2_generate_interrupt();
        break;
    case PS2_COMMAND_TEST_FIRST_PORT:
        ps2_queue_response(0x00);
        ps2_generate_interrupt();
        break;
    case PS2_COMMAND_DISABLE_FIRST_PORT:
        ps2_config_byte &= ~(1 << 0);
        break;
    case PS2_COMMAND_ENABLE_FIRST_PORT:
        ps2_config_byte |= (1 << 0);
        break;
    case PS2_COMMAND_READ_OUTPUT_PORT:
        ps2_queue_response(ps2_output_port);
        ps2_generate_interrupt();
        break;
    case PS2_COMMAND_WRITE_OUTPUT_PORT:
        ps2_data_expect = PS2_DATA_OUTPUT_PORT;
        break;
    default:
        unhandled(io, base);
    }
    ps2_update_status();
}

static void ps2_handle_device(exit_io_info_t *io, uint8_t *base)
{
    static bool ps2_expect_scancode = false;
    ps2_queue_response(PS2_RESPONSE_ACK);
    if (ps2_expect_scancode)
    {
        if (base[io->data_offset] != 0x02)
        {
            printf("Unexpected scancode: 0x%x\n", base[io->data_offset]);
            exit(1);
        }
        ps2_expect_scancode = false;
    }
    else
    {
        // assumes EXIT_IO_OUT
        switch (base[io->data_offset])
        {
        case PS2_DEVICE_SET_SCANCODE:
            ps2_expect_scancode = true;
            break;
        case PS2_DEVICE_ENABLE_SCANNING:
            // enable scanning
            break;
        case PS2_DEVICE_DISABLE_SCANNING:
            // disable scanning
            break;
        case PS2_DEVICE_RESET:
            ps2_status = PS2_STATUS_SYS_FLAG; // can skip this but just for the sake of completeness
            ps2_status &= ~(1 << 2);          // clear system flag (bit 2)
            ps2_queue_response(0xAA);         // self test passed
            break;
        default:
            unhandled(io, base);
        }
    }
    ps2_generate_interrupt();
}

static void ps2_handle_data(exit_io_info_t *io, uint8_t *base)
{
    switch (ps2_data_expect)
    {
    case PS2_DATA_NOTHING:
        if (ps2_config_byte & PS2_CONFIG_SENT_TO_DEVICE)
        {
            ps2_handle_device(io, base);
        }
        else
        {
            unhandled(io, base);
        }
        break;
    case PS2_DATA_CONFIG_BYTE:
        ps2_config_byte = base[io->data_offset];
        ps2_data_expect = PS2_DATA_NOTHING;
        break;
    case PS2_DATA_OUTPUT_PORT:
        ps2_output_port = base[io->data_offset];
        ps2_data_expect = PS2_DATA_NOTHING;
        break;
    default:
        unhandled(io, base);
    }
    ps2_update_status();
}

void ps2_handle(exit_io_info_t *io, uint8_t *base)
{
    LOG_MSG("Handling ps2 port: 0x%x, direction: 0x%x, size: 0x%x, count: 0x%x, data: 0x%lx", io->port, io->direction, io->size, io->count, base[io->data_offset]);
    if (io->port == PS2_STATUS_COMMAND_PORT)
    {
        if (io->direction == EXIT_IO_IN)
        {
            base[io->data_offset] = ps2_status;
            LOG_MSG("PS2 status: 0x%x", base[io->data_offset]);
        }
        else
        {
            ps2_handle_command(io, base);
        }
    }
    else if (io->port == PS2_DATA_PORT)
    {
        if (io->direction == EXIT_IO_OUT)
        {
            ps2_handle_data(io, base);
        }
        else // EXIT_IO_IN
        {
            if (ps2_response_pending)
            {
                base[io->data_offset] = ps2_dequeue_response();
                LOG_MSG("PS2 response: 0x%x", base[io->data_offset]);
            }
            else
            {
                LOG_MSG("PS2 data read but no response pending");
                base[io->data_offset] = 0xFF; // error
            }
        }
    }
}