#include <stdbool.h>
#include <err.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "components/pic.h"
#include "common.h"
#include "kvm.h"
#include "log.h"

LOG_DEFINE("pic");

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

    struct
    {
        // uint8_t interrupt_mask;
    } ocw1;

    struct
    {
        uint8_t level; // #irq#
        bool eoi;
        bool rotate;   // 1 = rotate, 0 = not rotate
        bool specific; // 1 = specific eoi, 0 = not specific eoi
    } ocw2;

    struct
    {
        bool ris;  // 1 = read IS register, 0 = read IR register
        bool rr;   // read register command
        bool poll; // read pool command
        bool smm;  // special mask mode
    } ocw3;

    uint8_t imr;
    uint8_t irr;
    uint8_t isr;
    uint8_t lowest_priority;
    bool auto_rotate;
} pic_t;

static pic_t pic_master;
static pic_t pic_slave;

#define PIC_CLOCK_FREQUENCY_HZ 1000
static pthread_mutex_t pic_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_t pic_thread = NULL;

// #define PIC_CONTROLLER ((slave) ? (pic_slave) : (pic_master))

uint8_t pic_get_next_interrupt(pic_t *pic)
{
    uint8_t pending = pic->irr & ~pic->imr; // only the interrupts that are pending and not masked will remain

    if (pending == 0)
    {
        return 0xff; // not interrupt pending
    }

    for (int i = 0; i < 8; i++)
    {
        uint8_t check_level = (pic->lowest_priority + i) % 8; // check from the lowest priority in a clock-like cycle
        if (pending & (1 << check_level))
        {
            pic->irr &= ~(1 << check_level); // clear interrupt
            return check_level;
        }
    }

    return 0xff; // shouldn't happen if pending is non-zero
}

void pic_process_interrupts()
{
    pic_t *pics[] = {&pic_master, &pic_slave};
    for (int i = 0; i < 2; i++) // 2 for the 2 pic's: master and slave
    {
        pic_t *pic = pics[i];
        uint8_t next_interrupt = pic_get_next_interrupt(pic);
        if (next_interrupt != 0xff) // no interrupt
        {
            uint8_t vector = pic->icw2.offset + next_interrupt;
            LOG_MSG("Interrupt %d, vector %d", next_interrupt, vector);
            kvm_interrupt(vector);

            if (!pic->icw4.aeoi)
            {
                pic->isr |= (1 << next_interrupt);
            }
            else if (pic->auto_rotate)
            {
                pic->lowest_priority = next_interrupt;
            }
        }
    }
}

void pic_clock_thread(void *arg)
{
    struct timespec sleep_time;
    sleep_time.tv_sec = 0;
    sleep_time.tv_nsec = 1000000000 / PIC_CLOCK_FREQUENCY_HZ; // nanoseconds per clock tick

    while (1)
    {
        nanosleep(&sleep_time, NULL);

        if (kvm_is_interrupts_enabled())
        {
            pthread_mutex_lock(&pic_mutex);
            pic_process_interrupts();
            pthread_mutex_unlock(&pic_mutex);
        }
    }

    return NULL;
}

void pic_init(bool slave)
{
    if (pic_thread == NULL)
    {
        if (pthread_create(&pic_thread, NULL, pic_clock_thread, NULL) != 0)
        {
            errx(1, "Failed to create pic clock thread");
        }
    }

    pic_t *pic = slave ? &pic_slave : &pic_master;
    pic->imr = 0xff;
    pic->lowest_priority = 7;
}

void pic_init_master()
{
    pic_init(false);
}

void pic_init_slave()
{
    pic_init(true);
}

void pic_handle(exit_io_info_t *io, uint8_t *base, bool slave)
{
    LOG_MSG("Handling pic port: 0x%x, direction: 0x%x, size: 0x%x, count: 0x%x, data: 0x%lx", io->port, io->direction, io->size, io->count, base[io->data_offset]);

    pic_t *pic = slave ? &pic_slave : &pic_master;
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
            if (GET_BITS(base[io->data_offset], 6, 2) != 0b01) // ocw2
            {
                uint8_t data = base[io->data_offset];
                pic->ocw2.level = GET_BITS(data, 0, 2);
                pic->ocw2.eoi = GET_BITS(data, 5, 1);
                pic->ocw2.rotate = GET_BITS(data, 6, 1);
                pic->ocw2.specific = GET_BITS(data, 7, 1);
                if (pic->ocw2.eoi)
                {
                    if (pic->ocw2.specific)
                    {
                        pic->isr &= ~(1 << pic->ocw2.level);
                    }
                    else
                    {
                        for (int i = 0; i < 8; i++)
                        {
                            int irq = (pic->lowest_priority + i + 1) % 8;
                            if (pic->isr & (1 << irq))
                            {
                                pic->isr &= ~(1 << irq);
                                if (pic->ocw2.rotate)
                                {
                                    pic->lowest_priority = irq;
                                }
                                break;
                            }
                        }
                    }
                }

                if (pic->ocw2.rotate)
                {
                    if (pic->ocw2.specific)
                    {
                        pic->lowest_priority = pic->ocw2.level;
                    }
                    else
                    {
                        pic->auto_rotate = true;
                    }
                }
                else if (!pic->ocw2.specific)
                {
                    pic->auto_rotate = false;
                }
                return;
            }
            else // ocw3
            {
                uint8_t data = base[io->data_offset];
                pic->ocw3.ris = GET_BITS(data, 0, 1);
                pic->ocw3.rr = GET_BITS(data, 1, 1);
                pic->ocw3.poll = GET_BITS(data, 2, 1);
                pic->ocw3.smm = GET_BITS(data, 5, 1);
                return;
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
            if (!pic->ocw3.rr)
            {
                base[io->data_offset] = pic->imr;
                return;
            }
            else
            {
                if (pic->ocw3.ris)
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

    printf("PIC Port 0x%02x is unhandled for direction: %d\n", io->port, io->direction);
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

void pic_raise_interrupt(uint8_t irq)
{
    if (irq >= 16)
    {
        return;
    }

    pic_t *pic = irq < 8 ? &pic_master : &pic_master;
    irq = irq % 8;

    pthread_mutex_lock(&pic_mutex);
    if (!(pic->irr & (1 << irq)) && !(pic->isr & (1 << irq)) && !(pic->imr & (1 << irq))) // not pending, not in service, and not masked
    {
        pic->irr |= 1 << irq; // raise interrupt
    }
    pthread_mutex_unlock(&pic_mutex);
}