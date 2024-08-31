#include <stdbool.h>
#include <err.h>
#include "devices/pic.h"
#include "common.h"

#define PIC_MASTER_COMMAND 0x20
#define PIC_MASTER_DATA 0x21
#define PIC_SLAVE_COMMAND 0xA0
#define PIC_SLAVE_DATA 0xA1

typedef enum
{
    PIC_ICW1,
    PIC_ICW2,
    PIC_ICW3,
    PIC_ICW4
} pic_init_state_t;

typedef struct
{
    pic_init_state_t init_state;
    struct
    {
        bool icw4_needed;
        bool mode;         // 1 = single mode, 0 = cascade
        bool adi;          // 1 = 4 byte interrupt vector, 0 = 8 byte interupt vector
        bool trigger_mode; // 1 = level triggered, 0 = edge triggered
    } icw1;

    struct
    {
        uint8_t offset;
    } icw2;

    struct
    {
        bool has_slave;
        uint8_t slave_id; // irq#
    } icw3;

    struct
    {
        bool type; // 1 = 8086/8088, 0 = MCS-80/85
        bool aeoi;
        uint8_t buffer_mode; // first bit 0: non buffered mode. second bit 1: master, second bit 0: slave
        bool buffered;
        bool sfnm; // 1 = special fully nested mode, 0 = not special fully nested mode
    } icw4;

    struct {
        // uint8_t interrupt_mask;
    } ocw1;

    struct {
        uint8_t level; // #irq#
        bool eoi;
        bool rotate; // 1 = rotate, 0 = not rotate
        bool specific; // 1 = specific eoi, 0 = not specific eoi
    } ocw2;

    struct {
        bool ris; // 1 = read IS register, 0 = read IR register
        bool rr; // read register command
        bool poll; // read pool command
        bool smm; // special mask mode
    } ocw3;

    uint8_t imr;
    uint8_t irr;
    uint8_t isr;
} pic_t;

static pic_t pic_master;
static pic_t pic_slave;

// #define PIC_CONTROLLER ((slave) ? (pic_slave) : (pic_master))

void pic_init_master()
{
    pic_master.imr = 0xff;
}

void pic_init_slave()
{
    pic_slave.imr = 0xff;
}

void pic_handle(exit_io_info_t *io, uint8_t *base, bool slave)
{
    pic_t* pic = slave ? &pic_slave : &pic_master;
    if (io->port == PIC_MASTER_COMMAND || io->port == PIC_SLAVE_COMMAND)
    {
        if (io->direction == EXIT_IO_OUT)
        {
            if (GET_BITS(base[io->data_offset], 4, 1)) // icw1
            {
                if (pic->init_state != PIC_ICW1)
                {
                    errx(1, "Command ICW1 in invalid state");
                }
                uint8_t data = base[io->data_offset];
                pic->icw1.icw4_needed = GET_BITS(data, 0, 1);
                pic->icw1.mode = GET_BITS(data, 1, 1);
                pic->icw1.adi = GET_BITS(data, 2, 1);
                pic->icw1.trigger_mode = GET_BITS(data, 3, 1);
                pic->init_state = PIC_ICW2;
                return;
            }
            if(GET_BITS(base[io->data_offset], 6, 2) != 0b01) // ocw2
            {
                uint8_t data = base[io->data_offset];
                pic->ocw2.level = GET_BITS(data, 0, 2);
            }
            else // ocw3
            {
                uint8_t data = base[io->data_offset];
                pic->ocw3.ris = GET_BITS(data, 0, 1);
                pic->ocw3.rr = GET_BITS(data, 1, 1);
                pic->ocw3.poll = GET_BITS(data, 2, 1);
                pic->ocw3.smm = GET_BITS(data, 5, 1);
            }
        }
    }
    else if (io->port == PIC_MASTER_DATA || io->port == PIC_SLAVE_DATA)
    {
        if (io->direction == EXIT_IO_OUT)
        {
            if (pic->init_state != PIC_ICW1)
            {
                switch (pic->init_state)
                {
                case PIC_ICW2:
                    pic->icw2.offset = base[io->data_offset] & ~0b11;
                    if (pic->icw1.icw4_needed)
                    {
                        pic->init_state = PIC_ICW3;
                    }
                    else
                    {
                        pic->init_state = PIC_ICW1;
                    }
                    return;

                case PIC_ICW3:
                    if (!slave)
                    {
                        pic->icw3.has_slave = base[io->data_offset];
                    }
                    else
                    {
                        pic->icw3.slave_id = base[io->data_offset] & 0b111;
                    }
                    pic->init_state = PIC_ICW4;
                    return;

                case PIC_ICW4:
                    uint8_t data = base[io->data_offset];
                    pic->icw4.type = GET_BITS(data, 0, 1);
                    pic->icw4.aeoi = GET_BITS(data, 1, 1);
                    pic->icw4.buffer_mode = GET_BITS(data, 2, 2);
                    pic->icw4.buffered = GET_BITS(data, 4, 1);
                    pic->icw4.sfnm = GET_BITS(data, 5, 1);
                    pic->init_state = PIC_ICW1;
                    return;
                }
            }

            // ocw1
            pic->imr = base[io->data_offset];
            return;
        }
        else
        {
            if(!pic->ocw3.rr)
            {
                base[io->data_offset] = pic->imr;
                return;
            }
            else
            {
                if(pic->ocw3.ris)
                {
                    base[io->data_offset] = pic->isr;
                    return;
                }
                else
                {
                    base[io->data_offset] = pic->irr;
                    return;
                }
            }
        }
    }

    printf("DMA Port 0x%02x is unhandled for direction: %d\n", io->port, io->direction);
    exit(1);
}

void pic_handle_master(exit_io_info_t *io, uint8_t *base)
{
    pic_handle(io, base, false);
}

void pic_handle_slave(exit_io_info_t *io, uint8_t *base)
{
    pic_handle(io, base, true);
}