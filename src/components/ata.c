#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include "components/ata.h"
#include "common.h"
#include "log.h"

/*
At first I made a mistake thinking each channel was its own drive. Now I know that each channel can have 2 drives.
I want to have a cdrom and a hard drive so I only need one channel. This is why I commented out the secondary channel.
This also means that some of this code is code that was supposed to be for the second drive when I thought that it was the secondary channel.
I still keep those ports mapped to
*/

LOG_DEFINE("ata");

#define ATA_HARDDRIVE_SECTOR_SIZE 512
#define ATA_CDROM_SECTOR_SIZE 2048

#define ATA_MASTER_BASE 0x1f0
#define ATA_MASTER_CONTROL 0x3f6
#define ATA_SLAVE_BASE 0x170
#define ATA_SLAVE_CONTROL 0x376

#define ATA_IO_OFFSET_DATA 0
#define ATA_IO_OFFSET_ERROR 1
#define ATA_IO_OFFSET_FEATURES 1
#define ATA_IO_OFFSET_SECTOR_COUNT 2
#define ATA_IO_OFFSET_SECTOR_NUMBER_LBA_LOW 3
#define ATA_IO_OFFSET_CYLINDER_LOW_LBA_MID 4
#define ATA_IO_OFFSET_CYLINDER_HIGH_LBA_HIGH 5
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

#define ATA_DEVICE_CONTROL_STOP_INTERRUPTS (1 << 1)
#define ATA_DEVICE_CONTROL_SOFTWARE_RESET (1 << 2)
#define ATA_DEVICE_CONTROL_READBACK (1 << 7)

#define ATA_DRIVE_ADDRESS_DS0 (1 << 0) // clears when drive 0 selected
#define ATA_DRIVE_ADDRESS_DS1 (1 << 1) // clears when drive 1 selected
#define ATA_DRIVE_ADDRESS_HS_START (1 << 2)
#define ATA_DRIVE_ADDRESS_HS_END (1 << 5)
#define ATA_DRIVE_ADDRESS_WRITE (1 << 6) // low while writing
#define ATA_DRIVE_ADDRESS_RESERVED (1 << 7)

#define ATA_COMMAND_NOP 0x00
#define ATA_COMMAND_PACKET 0xA0
#define ATA_COMMAND_IDENTIFY_PACKET_DEVICE 0xA1
#define ATA_COMMAND_IDENTIFY_DEVICE 0xEC

typedef struct
{
    uint16_t data;
    uint8_t error;
    uint8_t features;
    uint8_t sector_count;
    union
    {
        // when inputting it doesn't matter
        struct
        {
            uint8_t first;
            uint8_t second;
            uint8_t third;
        } any;

        struct
        {
            uint8_t lba_low;
            uint8_t lba_mid;
            uint8_t lba_high;
        } lba;

        struct
        {
            uint8_t sector_number;
            uint8_t cylinder_low;
            uint8_t cylinder_high;
        } chs;
    } mode;
    uint8_t drive_head;
    uint8_t status;
    pthread_mutex_t status_mutex;
    uint8_t command;
    uint8_t control;
    bool disable_interrupts;
#define ATA_DATA_BUFFER_SIZE 0x8000
    uint8_t data_buffer[ATA_DATA_BUFFER_SIZE];
    uint32_t data_buffer_size;
    uint32_t data_buffer_read;
    pthread_mutex_t data_buffer_mutex;
#define ATA_SCSI_CDB_BUFFER_SIZE 6
    uint16_t scsi_cdb_buffer[ATA_SCSI_CDB_BUFFER_SIZE];
    uint32_t scsi_cdb_buffer_size;
    pthread_mutex_t scsi_cdb_buffer_mutex;
    enum
    {
        ATA_NOTHING,
        ATA_SCSI_CDB,
        ATA_IDENTIFY,
    } expecting;
} ata_channel_t;

static ata_channel_t ata_channels[2] = {0};
static uint32_t cdrom_file_size = 0;
static FILE *cdrom_file = NULL;
static uint32_t harddisk_file_size = 0;
static FILE *harddisk_file = NULL;

void ata_init_disks(char *cdrom_path, char *harddisk_path)
{
    cdrom_file = fopen(cdrom_path, "rb");
    if (cdrom_file == NULL)
    {
        errx(1, "Failed to open kernel file");
    }
    cdrom_file_size = get_file_size(cdrom_file);

    harddisk_file = fopen(harddisk_path, "rb+");
    if (harddisk_file == NULL)
    {
        errx(1, "Failed to open hard disk file");
    }
    harddisk_file_size = get_file_size(harddisk_file);
}

void ata_deinit_disks()
{
    if (cdrom_file != NULL)
    {
        fclose(cdrom_file);
        cdrom_file = NULL;
    }

    if (harddisk_file != NULL)
    {
        fclose(harddisk_file);
        harddisk_file = NULL;
    }
}

void ata_identify_packet_device(void *args) // for cdrom
{
    ata_channel_t *channel = (ata_channel_t *)args;
    uint16_t identify_data[256] = {0};

    identify_data[0] = 0x00008580;

    char *serial = "123456789          ";
    for (int i = 0; i < 10; i++)
    {
        identify_data[10 + i] = (serial[i * 2 + 1]) | (serial[i * 2] << 8);
    }

    char *firmware = "1.0     ";
    for (int i = 0; i < 4; i++)
    {
        identify_data[23 + i] = (firmware[i * 2 + 1]) | (firmware[i * 2] << 8);
    }

    char *model = "ATAPI CD-ROM                           ";
    for (int i = 0; i < 20; i++)
    {
        identify_data[27 + i] = (model[i * 2 + 1]) | (model[i * 2] << 8);
    }

    identify_data[47] = 0x8000; // Fixed + Reserved

    identify_data[60] = (cdrom_file_size / ATA_CDROM_SECTOR_SIZE) & 0xFFFF;
    identify_data[61] = ((cdrom_file_size / ATA_CDROM_SECTOR_SIZE) >> 16) & 0xFFFF;

    identify_data[82] = (1 << 14) | (1 << 13) | (1 << 12); // NOP command, read and write buffer are supported
    identify_data[85] = (1 << 14) | (1 << 13) | (1 << 12); // NOP command, read and write buffer are supported

    pthread_mutex_lock(&channel->data_buffer_mutex);
    memcpy(channel->data_buffer, identify_data, sizeof(identify_data));
    channel->data_buffer_size = sizeof(identify_data);
    pthread_mutex_unlock(&channel->data_buffer_mutex);

    pthread_mutex_lock(&channel->status_mutex);
    channel->status |= ATA_STATUS_DATA_REQUEST;
    channel->status &= ~ATA_STATUS_BUSY;
    pthread_mutex_unlock(&channel->status_mutex);
}

void ata_identify_device(void *args) // for hard disk
{
    ata_channel_t *channel = (ata_channel_t *)args;
    uint16_t identify_data[256] = {0};

    // copied from AI
    identify_data[0] = 0x0040; // General configuration: hard disk
    // obselete
    // identify_data[1] = 16383;      // Number of cylinders
    // identify_data[3] = 16;         // Number of heads
    // identify_data[6] = 63;         // Number of sectors per track

    char *serial = "987654321          ";
    for (int i = 0; i < 10; i++)
    {
        identify_data[10 + i] = (serial[i * 2 + 1]) | (serial[i * 2] << 8);
    }

    char *firmware = "1.0     ";
    for (int i = 0; i < 4; i++)
    {
        identify_data[23 + i] = (firmware[i * 2 + 1]) | (firmware[i * 2] << 8);
    }

    char *model = "ATA Hard Drive                          ";
    for (int i = 0; i < 20; i++)
    {
        identify_data[27 + i] = (model[i * 2 + 1]) | (model[i * 2] << 8);
    }

    identify_data[47] = 0x8000; // Fixed + Reserved

    identify_data[60] = (harddisk_file_size / ATA_HARDDRIVE_SECTOR_SIZE) & 0xFFFF;
    identify_data[61] = ((harddisk_file_size / ATA_HARDDRIVE_SECTOR_SIZE) >> 16) & 0xFFFF;

    identify_data[82] = (1 << 14) | (1 << 13) | (1 << 12);
    identify_data[85] = (1 << 14) | (1 << 13) | (1 << 12);

    pthread_mutex_lock(&channel->data_buffer_mutex);
    memcpy(channel->data_buffer, identify_data, sizeof(identify_data));
    channel->data_buffer_size = sizeof(identify_data);
    pthread_mutex_unlock(&channel->data_buffer_mutex);

    pthread_mutex_lock(&channel->status_mutex);
    channel->status |= ATA_STATUS_DATA_REQUEST;
    channel->status &= ~ATA_STATUS_BUSY;
    pthread_mutex_unlock(&channel->status_mutex);
}

void ata_handle_scsi_cdb(void *args)
{
    ata_channel_t *channel = (ata_channel_t *)args;
    uint8_t *cdb = channel->scsi_cdb_buffer;
    uint32_t sector_size = (channel->drive_head & ATA_DRIVE_HEAD_DRIVE) ? ATA_HARDDRIVE_SECTOR_SIZE : ATA_CDROM_SECTOR_SIZE;

    if (cdb[0] == 0x28)
    {
        uint32_t lba = (cdb[2] << 24) | (cdb[3] << 16) | (cdb[4] << 8) | cdb[5];
        uint32_t count = (cdb[7] << 8) | cdb[8];

        if (lba + count > cdrom_file_size / sector_size)
        {
            LOG_MSG("Error");
            printf("error\n");
            pthread_mutex_lock(&channel->status_mutex);
            channel->error = ATA_ERROR_ABORTED_COMMAND;
            channel->status |= ATA_STATUS_ERROR;
            pthread_mutex_unlock(&channel->status_mutex);
        }
        else
        {
            printf("lba: %d %d\n", lba, lba * sector_size);
            printf("size: %d\n", cdrom_file_size);
            printf("cdrom_file: %d\n", cdrom_file);
            fseek(cdrom_file, lba * sector_size, SEEK_SET);

            pthread_mutex_lock(&channel->data_buffer_mutex);
            channel->data_buffer_size = sector_size * count;
            printf("count: %d %d\n", count, channel->data_buffer_size);
            fread(channel->data_buffer, 1, channel->data_buffer_size, cdrom_file);
            pthread_mutex_unlock(&channel->data_buffer_mutex);

            pthread_mutex_lock(&channel->scsi_cdb_buffer_mutex);
            channel->scsi_cdb_buffer_size = 0;
            memset(channel->scsi_cdb_buffer, 0, sizeof(channel->scsi_cdb_buffer));
            pthread_mutex_unlock(&channel->scsi_cdb_buffer_mutex);

            pthread_mutex_lock(&channel->status_mutex);
            channel->status |= ATA_STATUS_DATA_REQUEST;
            channel->status &= ~ATA_STATUS_BUSY;
            pthread_mutex_unlock(&channel->status_mutex);
            LOG_MSG("Read. Status: %b", channel->status);
        }
    }
    else if (channel->scsi_cdb_buffer[0] == 0x00)
    {
        // pthread_mutex_lock(&channel->data_buffer_mutex);
        // channel->data_buffer[0] = 0;
        // channel->data_buffer_size = 1;
        // pthread_mutex_unlock(&channel->data_buffer_mutex);
        printf("test\n");

        // pthread_mutex_lock(&channel->scsi_cdb_buffer_mutex);
        // channel->scsi_cdb_buffer_size = 0;
        // memset(channel->scsi_cdb_buffer, 0, sizeof(channel->scsi_cdb_buffer));
        // pthread_mutex_unlock(&channel->scsi_cdb_buffer_mutex);

        pthread_mutex_lock(&channel->status_mutex);
        // channel->status |= ATA_STATUS_DATA_REQUEST;
        channel->status &= ~ATA_STATUS_BUSY;
        pthread_mutex_unlock(&channel->status_mutex);
        LOG_MSG("Set: %b", channel->status);
    }
}

void ata_init_primary()
{
    ata_channel_t *master = &ata_channels[0];
    master->status = ATA_STATUS_READY;
    master->drive_head = ATA_DRIVE_HEAD_SET_1 | ATA_DRIVE_HEAD_SET_2;
    master->status_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    master->data_buffer_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    master->scsi_cdb_buffer_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

    // uint32_t *identify_data = master->identify_data;
    // identify_data[0] = 0x00008580;

    // char *serial = "1234567890123456";
    // for (int i = 0; i < 5; i++)
    // {
    //     identify_data[10 + i] = (serial[i * 4 + 3] << 24) | (serial[i * 4 + 2] << 16) | (serial[i * 4 + 1] << 8) | serial[i * 4];
    // }

    // char *firmware = "1.0";
    // for (int i = 0; i < 2; i++)
    // {
    //     identify_data[23 + i] = (firmware[i * 4 + 3] << 24) | (firmware[i * 4 + 2] << 16) | (firmware[i * 4 + 1] << 8) | firmware[i * 4];
    // }

    // char *model = "ATAPI CD-ROM";
    // for (int i = 0; i < 10; i++)
    // {
    //     identify_data[27 + i] = (model[i * 4 + 3] << 24) | (model[i * 4 + 2] << 16) | (model[i * 4 + 1] << 8) | model[i * 4];
    // }
}

void ata_init_secondary()
{
    // ata_channel_t *slave = &ata_channels[0];
    // slave->status = ATA_STATUS_READY;
    // slave->drive_head = ATA_DRIVE_HEAD_SET_1 | ATA_DRIVE_HEAD_SET_2 | ATA_DRIVE_HEAD_DRIVE;
    // slave->status_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    // slave->buffer_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
}

static void ata_handle_command(exit_io_info_t *io, uint8_t *base, ata_channel_t *channel)
{
    uint8_t data = base[io->data_offset];
    switch (data)
    {
    case ATA_COMMAND_NOP:
        break;
    case ATA_COMMAND_IDENTIFY_PACKET_DEVICE:
        // for the sake of completeness
        if (channel->drive_head & ATA_DRIVE_HEAD_DRIVE) // hard disk
        {
            pthread_mutex_lock(&channel->status_mutex);
            channel->error = ATA_ERROR_ABORTED_COMMAND;
            channel->status |= ATA_STATUS_ERROR;
            pthread_mutex_unlock(&channel->status_mutex);
        }
        else // cdrom
        {
            pthread_mutex_lock(&channel->status_mutex);
            channel->status |= ATA_STATUS_BUSY;
            pthread_mutex_unlock(&channel->status_mutex);
            channel->expecting = ATA_IDENTIFY;
            pthread_t thread;
            pthread_create(&thread, NULL, ata_identify_packet_device, channel);
            pthread_detach(thread);
        }
        break;
    case ATA_COMMAND_PACKET:
        if (channel->drive_head & ATA_DRIVE_HEAD_DRIVE) // hard disk
        {
            pthread_mutex_lock(&channel->status_mutex);
            channel->error = ATA_ERROR_ABORTED_COMMAND;
            channel->status |= ATA_STATUS_ERROR;
            pthread_mutex_unlock(&channel->status_mutex);
        }
        else // cdrom
        {
            pthread_mutex_lock(&channel->status_mutex);
            channel->status |= ATA_STATUS_DATA_REQUEST;
            pthread_mutex_unlock(&channel->status_mutex);
            channel->expecting = ATA_SCSI_CDB;
        }
        break;
    case ATA_COMMAND_IDENTIFY_DEVICE:
        if (channel->drive_head & ATA_DRIVE_HEAD_DRIVE) // hard disk
        {
            pthread_mutex_lock(&channel->status_mutex);
            channel->status |= ATA_STATUS_BUSY;
            pthread_mutex_unlock(&channel->status_mutex);
            channel->expecting = ATA_IDENTIFY;
            pthread_t thread;
            pthread_create(&thread, NULL, ata_identify_device, channel);
            pthread_detach(thread);
        }
        else // cdrom
        {
            pthread_mutex_lock(&channel->status_mutex);
            channel->error = ATA_ERROR_ABORTED_COMMAND;
            channel->status |= ATA_STATUS_ERROR;
            pthread_mutex_unlock(&channel->status_mutex);
        }
        break;
    default:
        unhandled(io, base);
    }
}

void ata_handle_io(exit_io_info_t *io, uint8_t *base, bool is_secondary)
{
    LOG_MSG("Handling ata io port: 0x%x, direction: 0x%x, size: 0x%x, count: 0x%x, data: 0x%x", io->port, io->direction, io->size, io->count, base[io->data_offset]);
    uint8_t offset = is_secondary ? io->port - ATA_SLAVE_BASE : io->port - ATA_MASTER_BASE;
    ata_channel_t *channel = &ata_channels[is_secondary ? 1 : 0];

    if (io->direction == EXIT_IO_OUT)
    {
        switch (offset)
        {
        case ATA_IO_OFFSET_DATA:
            if (channel->status & ATA_STATUS_DATA_REQUEST)
            {
                if (channel->expecting != ATA_SCSI_CDB)
                {
                    printf("Data sent without data request (expected SCSI CDB)\n");
                    exit(1);
                }
                // pthread_mutex_lock(&channel->buffer_mutex);
                memcpy(channel->scsi_cdb_buffer + channel->scsi_cdb_buffer_size / 2, base + io->data_offset, io->count * io->size);
                LOG_MSG("Read %d bytes from data_buffer at offset %d", io->count * io->size, channel->scsi_cdb_buffer_size);
                pthread_mutex_lock(&channel->scsi_cdb_buffer_mutex);
                channel->scsi_cdb_buffer_size += io->count * io->size;
                pthread_mutex_unlock(&channel->scsi_cdb_buffer_mutex);
                LOG_MSG("scsi_cdb_buffer_size: %d\n", channel->scsi_cdb_buffer_size);
                uint8_t *cdb = (uint8_t *)channel->scsi_cdb_buffer;

                if (channel->scsi_cdb_buffer_size / 2 == ATA_SCSI_CDB_BUFFER_SIZE)
                {
                    pthread_mutex_lock(&channel->status_mutex);
                    channel->status &= ~ATA_STATUS_DATA_REQUEST;
                    channel->status |= ATA_STATUS_BUSY;
                    pthread_mutex_unlock(&channel->status_mutex);

                    pthread_mutex_lock(&channel->scsi_cdb_buffer_mutex);
                    channel->scsi_cdb_buffer_size = 0;
                    pthread_mutex_unlock(&channel->scsi_cdb_buffer_mutex);

                    pthread_t thread;
                    pthread_create(&thread, NULL, ata_handle_scsi_cdb, channel);
                    int ret;
                    pthread_join(thread, &ret);
                    LOG_MSG("created thread\n");
                }
            }
            else
            {
                printf("Data sent without data request\n");
                exit(1);
            }
            break;
        case ATA_IO_OFFSET_FEATURES:
            channel->features = base[io->data_offset];
            if (channel->features != 0)
            {
                printf("Invalid features: %b\n", channel->features);
                exit(1);
            }
            break;
        case ATA_IO_OFFSET_SECTOR_COUNT:
            channel->sector_count = base[io->data_offset];
            break;
        case ATA_IO_OFFSET_SECTOR_NUMBER_LBA_LOW:
            channel->mode.any.first = base[io->data_offset];
            break;
        case ATA_IO_OFFSET_CYLINDER_LOW_LBA_MID:
            channel->mode.any.second = base[io->data_offset];
            break;
        case ATA_IO_OFFSET_CYLINDER_HIGH_LBA_HIGH:
            channel->mode.any.third = base[io->data_offset];
            break;
        case ATA_IO_OFFSET_DRIVE_HEAD:
            channel->drive_head = base[io->data_offset];
            if (!(channel->drive_head & ATA_DRIVE_HEAD_SET_1) || !(channel->drive_head & ATA_DRIVE_HEAD_SET_2))
            {
                printf("Invalid drive head: %b\n", channel->drive_head);
                exit(1);
            }
            break;
        case ATA_IO_OFFSET_COMMAND:
            ata_handle_command(io, base, channel);
            break;
        default:
            unhandled(io, base);
        }
    }
    else // EXIT_IO_IN
    {
        switch (offset)
        {
        case ATA_IO_OFFSET_DATA:
            if (channel->status & ATA_STATUS_DATA_REQUEST && channel->data_buffer_read < channel->data_buffer_size)
            {
                uint32_t to_read = io->count * io->size;
                if (channel->data_buffer_read + to_read > channel->data_buffer_size)
                {
                    to_read = channel->data_buffer_size - channel->data_buffer_read; // or throw an error
                }

                pthread_mutex_lock(&channel->data_buffer_mutex);

                memcpy(base + io->data_offset, channel->data_buffer + channel->data_buffer_read, to_read);
                LOG_MSG("Wrote %d bytes to data_buffer at offset %d", to_read, channel->data_buffer_read);
                channel->data_buffer_read += to_read;

                for (int i = 0; i < to_read; i++)
                {
                    LOG_MSG("%d: %02x ", i, base[io->data_offset + i]);
                }

                if (channel->data_buffer_read == channel->data_buffer_size)
                {
                    channel->data_buffer_read = 0;
                    channel->data_buffer_size = 0;

                    pthread_mutex_lock(&channel->scsi_cdb_buffer_mutex);
                    channel->scsi_cdb_buffer_size = 0;
                    pthread_mutex_unlock(&channel->scsi_cdb_buffer_mutex);

                    pthread_mutex_lock(&channel->status_mutex);
                    channel->status &= ~ATA_STATUS_DATA_REQUEST;
                    pthread_mutex_unlock(&channel->status_mutex);
                }

                pthread_mutex_unlock(&channel->data_buffer_mutex);
            }
            else
            {
                LOG_MSG("sizeof(data_buffer): %d, read: %d\n", channel->data_buffer_size, channel->data_buffer_read);
                LOG_MSG("Data requested without data available\n");
                exit(1);
            }
        case ATA_IO_OFFSET_ERROR:
            base[io->data_offset] = channel->error;
            channel->error = 0;
            pthread_mutex_lock(&channel->status_mutex);
            channel->status &= ~ATA_STATUS_ERROR;
            pthread_mutex_unlock(&channel->status_mutex);
            break;
        case ATA_IO_OFFSET_SECTOR_COUNT:
            base[io->data_offset] = channel->sector_count;
            break;
        case ATA_IO_OFFSET_SECTOR_NUMBER_LBA_LOW:
            base[io->data_offset] = channel->mode.any.first;
            break;
        case ATA_IO_OFFSET_DRIVE_HEAD:
            base[io->data_offset] = channel->drive_head;
            break;
        case ATA_IO_OFFSET_STATUS:
            pthread_mutex_lock(&channel->status_mutex);
            // should affect interrupts
            base[io->data_offset] = channel->status;
            LOG_MSG("Status: %b", channel->status);
            pthread_mutex_unlock(&channel->status_mutex);
            break;
        default:
            unhandled(io, base);
        }
    }
}

void ata_handle_control(exit_io_info_t *io, uint8_t *base, bool slave)
{
    LOG_MSG("Handling ata control port: 0x%x, direction: 0x%x, size: 0x%x, count: 0x%x, data: 0x%x", io->port, io->direction, io->size, io->count, base[io->data_offset]);
    uint8_t offset = slave ? io->port - ATA_SLAVE_CONTROL : io->port - ATA_MASTER_CONTROL;
    ata_channel_t *channel = &ata_channels[slave ? 1 : 0];
    switch (offset)
    {
    case ATA_CONTROL_OFFSET_DEVICE_CONTROL:
        if (io->direction == EXIT_IO_OUT)
        {
            uint8_t data = base[io->data_offset];
            if (channel->control & ATA_DEVICE_CONTROL_SOFTWARE_RESET && (!(data & ATA_DEVICE_CONTROL_SOFTWARE_RESET)))
            {
                pthread_mutex_lock(&channel->status_mutex);
                channel->status = ATA_STATUS_READY;
                pthread_mutex_unlock(&channel->status_mutex);

                memset(&channel->mode.any, 0, sizeof(channel->mode.any));

                pthread_mutex_lock(&channel->data_buffer_mutex);
                memset(channel->data_buffer, 0, sizeof(channel->data_buffer));
                pthread_mutex_unlock(&channel->data_buffer_mutex);
            }
            channel->control = data;
        }
        else // EXIT_IO_IN
        {
            base[io->data_offset] = channel->status;
        }
        break;
    default:
        unhandled(io, base);
    }
}

void ata_handle_io_primary(exit_io_info_t *io, uint8_t *base)
{
    ata_handle_io(io, base, false);
}

void ata_handle_io_secondary(exit_io_info_t *io, uint8_t *base)
{
    // ata_handle_io(io, base, true);
}

void ata_handle_control_primary(exit_io_info_t *io, uint8_t *base)
{
    ata_handle_control(io, base, false);
}

void ata_handle_control_secondary(exit_io_info_t *io, uint8_t *base)
{
    // ata_handle_control(io, base, true);
}