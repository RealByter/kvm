#include <stdbool.h>
#include "components/ata.h"
#include "common.h"

#define ATA_MASTER_BASE 0x1f0
#define ATA_MASTER_CONTROL 0x3f6
#define ATA_SLAVE_BASE 0x170
#define ATA_SLAVE_CONTROL 0x376

#define ATA_IO_OFFSET_DATA 0
#define ATA_IO_OFFSET_ERROR 1
#define ATA_IO_OFFSET_FEATURES 1
#define ATA_IO_OFFSET_SECTOR_COUNT 2
#define ATA_IO_OFFSET_SECTOR_NUMBER 3
#define ATA_IO_OFFSET_LBA_LOW 3
#define ATA_IO_OFFSET_CYLINDER_LOW 4
#define ATA_IO_OFFSET_LBA_MID 4
#define ATA_IO_OFFSET_CYLINDER_HIGH 5
#define ATA_IO_OFFSET_LBA_HIGH 5
#define ATA_IO_OFFSET_DRIVE_HEAD 6
#define ATA_IO_OFFSET_STATUS 7
#define ATA_IO_OFFSET_COMMAND 7

#define ATA_CONTROL_OFFSET_ALT_STATUS 0
#define ATA_CONTROL_OFFSET_DEVICE_CONTROL 0
#define ATA_CONTROL_OFFSET_DRIVE_ADDRESS 1

#define ATA_ERROR_ADDRESS_NOT_FOUND (1 << 0)
#define ATA_ERROR_TRACK_0_NOT_FOUND (1 << 1)
#define ATA_ERROR_ABORTED_COMMAND (1 << 2)
#define ATA_ERROR_MEDIA_CHANGE_REQUEST (1 << 3)
#define ATA_ERROR_ID_NOT_FOUND (1 << 4)
#define ATA_ERROR_MEDIA_CHANGED (1 << 5)
#define ATA_ERROR_UNCORRECTABLE_DATA (1 << 6)
#define ATA_ERROR_BAD_BLOCK_DETECTED (1 << 7)

#define ATA_STATUS_ERROR (1 << 0)
#define ATA_STATUS_INDEX (1 << 1)
#define ATA_STATUS_CORRECTED_DATA (1 << 2)
#define ATA_STATUS_DATA_REQUEST (1 << 3)
#define ATA_STATUS_OVERLAPPED_SERVICE (1 << 4)
#define ATA_STATUS_DRIVE_FAULT (1 << 5)
#define ATA_STATUS_READY (1 << 6)
#define ATA_STATUS_BUSY (1 << 7)

#define ATA_DRIVE_HEAD_CHS_LBA_START (1 << 0)
#define ATA_DRIVE_HEAD_CHS_LBA_END (1 << 3)
#define ATA_DRIVE_HEAD_DRIVE (1 << 4)
#define ATA_DRIVE_HEAD_SET_1 (1 << 5)
#define ATA_DRIVE_HEAD_ADDRESSING (1 << 6) // CHS if clear LBA if set
#define ATA_DRIVE_HEAD_SET_2 (1 << 7)

#define ATA_DEVICE_CONTROL_STOP (1 << 1) // stops interrupts
#define ATA_DEVICE_CONTROL_SOFTWARE_RESET (1 << 2)
#define ATA_DEVICE_CONTROL_READBACK (1 << 7)

#define ATA_DRIVE_ADDRESS_DS0 (1 << 0) // clears when drive 0 selected
#define ATA_DRIVE_ADDRESS_DS1 (1 << 1) // clears when drive 1 selected
#define ATA_DRIVE_ADDRESS_HS_START (1 << 2)
#define ATA_DRIVE_ADDRESS_HS_END (1 << 5)
#define ATA_DRIVE_ADDRESS_WRITE (1 << 6) // low while writing
#define ATA_DRIVE_ADDRESS_RESERVED (1 << 7)

#define ATA_COMMAND_NOP 0x00

typedef struct {
    uint16_t data;
    uint8_t error;
    uint8_t features;
    uint8_t sector_count;
    uint8_t sector_number;
    uint8_t lba_low;
    uint8_t cylinder_low;
    uint8_t lba_mid;
    uint8_t cylinder_high;
    uint8_t lba_high;
    uint8_t drive_head;
    uint8_t status;
    uint8_t command;
} ata_channel_t;

static ata_channel_t* ata_channels[2] = {0};

static void ata_handle_command(exit_io_info_t *io, uint8_t *base)
{
    uint8_t data = base[io->data_offset];
    switch (data)
    {
    case ATA_COMMAND_NOP:
        break;
    default:
        unhandled(io, base);
    }
}

void ata_handle_io(exit_io_info_t *io, uint8_t *base, bool slave)
{
    uint8_t port = slave ? io->port - ATA_SLAVE_BASE : io->port - ATA_MASTER_BASE;
    ata_channel_t* channel = &ata_channels[slave ? 1 : 0];

    if(io->direction == EXIT_IO_OUT)
    {
        switch(port)
        {
            case ATA_IO_OFFSET_DRIVE_HEAD:
                channel->drive_head = base[io->data_offset];
                if(!(channel->drive_head & ATA_DRIVE_HEAD_SET_1) || !(channel->drive_head & ATA_DRIVE_HEAD_SET_2))
                {
                    printf("Invalid drive head: %b\n", channel->drive_head);
                    exit(1);
                }
                break;
            case ATA_IO_OFFSET_COMMAND:
                ata_handle_command(io, base);
                break;
            default:
                unhandled(io, base);
        }
    }
}

void ata_handle_control(exit_io_info_t *io, uint8_t *base, bool slave)
{
}

void ata_handle_io_master(exit_io_info_t *io, uint8_t *base)
{
    ata_handle_io(io, base, false);
}

void ata_handle_io_slave(exit_io_info_t *io, uint8_t *base)
{
    ata_handle_io(io, base, true);
}

void ata_handle_control_master(exit_io_info_t *io, uint8_t *base)
{
    ata_handle_control(io, base, false);
}

void ata_handle_control_slave(exit_io_info_t *io, uint8_t *base)
{
    ata_handle_control(io, base, true);
}