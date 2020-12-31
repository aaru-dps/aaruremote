/*
 * This file is part of the Aaru Remote Server.
 * Copyright (c) 2019-2021 Natalia Portillo.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "../aaruremote.h"
#include "linux.h"
#include "mmc/ioctl.h"

int32_t SendSdhciCommand(void*     device_ctx,
                         uint8_t   command,
                         uint8_t   write,
                         uint8_t   application,
                         uint32_t  flags,
                         uint32_t  argument,
                         uint32_t  block_size,
                         uint32_t  blocks,
                         char*     buffer,
                         uint32_t  buf_len,
                         uint32_t  timeout,
                         uint32_t* response,
                         uint32_t* duration,
                         uint32_t* sense)
{
    DeviceContext*     ctx = device_ctx;
    struct mmc_ioc_cmd mmc_ioc_cmd;
    int32_t            error;
    *duration = 0;
    *sense    = 0;

    if(!ctx) return -1;

    memset(response, 0, sizeof(uint32_t) * 4);
    memset(&mmc_ioc_cmd, 0, sizeof(struct mmc_ioc_cmd));

    mmc_ioc_cmd.write_flag = write;
    mmc_ioc_cmd.is_acmd    = application;
    mmc_ioc_cmd.opcode     = command;
    mmc_ioc_cmd.arg        = argument;
    mmc_ioc_cmd.flags      = flags;
    mmc_ioc_cmd.blksz      = block_size;
    mmc_ioc_cmd.blocks     = blocks;
    if(timeout > 0)
    {
        mmc_ioc_cmd.data_timeout_ns = timeout * 1000000000;
        mmc_ioc_cmd.cmd_timeout_ms  = timeout * 1000;
    }
    mmc_ioc_cmd.data_ptr = (uint64_t)buffer;

    // TODO: Timing
    error = ioctl(ctx->fd, MMC_IOC_CMD, &mmc_ioc_cmd);

    if(error < 0) error = errno;

    *sense = error < 0;

    memcpy((char*)response, (char*)&mmc_ioc_cmd.response, sizeof(uint32_t) * 4);

    return error;
}

int32_t GetSdhciRegisters(void*     device_ctx,
                          char**    csd,
                          char**    cid,
                          char**    ocr,
                          char**    scr,
                          uint32_t* csd_len,
                          uint32_t* cid_len,
                          uint32_t* ocr_len,
                          uint32_t* scr_len)
{
    DeviceContext* ctx = device_ctx;
    char*          tmp_string;
    char*          sysfs_path_csd;
    char*          sysfs_path_cid;
    char*          sysfs_path_scr;
    char*          sysfs_path_ocr;
    size_t         len;
    FILE*          file;
    *csd     = NULL;
    *cid     = NULL;
    *ocr     = NULL;
    *scr     = NULL;
    *csd_len = 0;
    *cid_len = 0;
    *ocr_len = 0;
    *scr_len = 0;
    size_t n = 1026;

    if(!ctx) return -1;

    if(strncmp(ctx->device_path, "/dev/mmcblk", 11) != 0) return 0;

    len            = strlen(ctx->device_path) + 19;
    sysfs_path_csd = malloc(len);
    sysfs_path_cid = malloc(len);
    sysfs_path_scr = malloc(len);
    sysfs_path_ocr = malloc(len);
    tmp_string     = malloc(1024);

    if(!sysfs_path_csd || !sysfs_path_cid || !sysfs_path_scr || !sysfs_path_ocr || !tmp_string)
    {
        free(sysfs_path_csd);
        free(sysfs_path_cid);
        free(sysfs_path_scr);
        free(sysfs_path_ocr);
        free(tmp_string);
        return 0;
    }

    memset(sysfs_path_csd, 0, len);
    memset(sysfs_path_cid, 0, len);
    memset(sysfs_path_scr, 0, len);
    memset(sysfs_path_ocr, 0, len);
    memset(tmp_string, 0, strlen(ctx->device_path) - 5);

    memcpy(tmp_string, ctx->device_path + 5, strlen(ctx->device_path) - 5);
    snprintf(sysfs_path_csd, len, "/sys/block/%s/device/csd", tmp_string);
    snprintf(sysfs_path_cid, len, "/sys/block/%s/device/cid", tmp_string);
    snprintf(sysfs_path_scr, len, "/sys/block/%s/device/scr", tmp_string);
    snprintf(sysfs_path_ocr, len, "/sys/block/%s/device/ocr", tmp_string);

    if(access(sysfs_path_csd, R_OK) == 0)
    {
        file = fopen(sysfs_path_csd, "r");

        if(file != NULL)
        {
            len = getline(&tmp_string, &n, file);
            if(len > 0)
            {
                *csd_len = Hexs2Bin(tmp_string, (unsigned char**)csd);

                if(*csd_len <= 0)
                {
                    *csd_len = 0;
                    *csd     = NULL;
                }
            }

            fclose(file);
        }
    }

    if(access(sysfs_path_cid, R_OK) == 0)
    {
        file = fopen(sysfs_path_cid, "r");

        if(file != NULL)
        {
            len = getline(&tmp_string, &n, file);
            if(len > 0)
            {
                *cid_len = Hexs2Bin(tmp_string, (unsigned char**)cid);

                if(*cid_len <= 0)
                {
                    *cid_len = 0;
                    *cid     = NULL;
                }
            }

            fclose(file);
        }
    }

    if(access(sysfs_path_scr, R_OK) == 0)
    {
        file = fopen(sysfs_path_scr, "r");

        if(file != NULL)
        {
            len = getline(&tmp_string, &n, file);
            if(len > 0)
            {
                *scr_len = Hexs2Bin(tmp_string, (unsigned char**)scr);

                if(*scr_len <= 0)
                {
                    *scr_len = 0;
                    *scr     = NULL;
                }
            }

            fclose(file);
        }
    }

    if(access(sysfs_path_ocr, R_OK) == 0)
    {
        file = fopen(sysfs_path_ocr, "r");

        if(file != NULL)
        {
            len = getline(&tmp_string, &n, file);
            if(len > 0)
            {
                *ocr_len = Hexs2Bin(tmp_string, (unsigned char**)ocr);

                if(*ocr_len <= 0)
                {
                    *ocr_len = 0;
                    *ocr     = NULL;
                }
            }

            fclose(file);
        }
    }

    free(sysfs_path_csd);
    free(sysfs_path_cid);
    free(sysfs_path_scr);
    free(sysfs_path_ocr);
    free(tmp_string);

    return csd_len != 0 || cid_len != 0 || scr_len != 0 || ocr_len != 0;
}

int32_t SendMultiSdhciCommand(void*            device_ctx,
                              uint64_t         count,
                              MmcSingleCommand commands[],
                              uint32_t*        duration,
                              uint32_t*        sense)
{
    DeviceContext* ctx = device_ctx;
    *duration          = 0;
    *sense             = 0;
    struct mmc_ioc_multi_cmd* mmc_ioc_multi_cmd;
    uint64_t                  i;
    int32_t                   error;
    if(!ctx) return -1;

    mmc_ioc_multi_cmd = malloc(sizeof(struct mmc_ioc_multi_cmd) + sizeof(struct mmc_ioc_cmd) * count);

    if(!mmc_ioc_multi_cmd)
    {
        *sense = 1;
        return errno;
    }

    memset(mmc_ioc_multi_cmd, 0, sizeof(struct mmc_ioc_multi_cmd) + sizeof(struct mmc_ioc_cmd) * count);

    mmc_ioc_multi_cmd->num_of_cmds = count;

    for(i = 0; i < count; i++)
    {
        mmc_ioc_multi_cmd->cmds[i].write_flag = commands[i].write;
        mmc_ioc_multi_cmd->cmds[i].is_acmd    = commands[i].application;
        mmc_ioc_multi_cmd->cmds[i].opcode     = commands[i].command;
        mmc_ioc_multi_cmd->cmds[i].arg        = commands[i].argument;
        mmc_ioc_multi_cmd->cmds[i].flags      = commands[i].flags;
        mmc_ioc_multi_cmd->cmds[i].blksz      = commands[i].block_size;
        mmc_ioc_multi_cmd->cmds[i].blocks     = commands[i].blocks;
        /* TODO: Where?
        if(commands[i].timeout > 0)
        {
            mmc_ioc_multi_cmd->cmds[i].data_timeout_ns = commands[i].timeout * 1000000000;
            mmc_ioc_multi_cmd->cmds[i].cmd_timeout_ms  = commands[i].timeout * 1000;
        }
         */
        mmc_ioc_multi_cmd->cmds[i].data_ptr = (uint64_t)commands[i].buffer;
    }

    error = ioctl(ctx->fd, MMC_IOC_MULTI_CMD, mmc_ioc_multi_cmd);

    if(error < 0) error = errno;

    *sense = error < 0;

    for(i = 0; i < count; i++)
        memcpy((char*)commands[i].response, (char*)mmc_ioc_multi_cmd->cmds[i].response, sizeof(uint32_t) * 4);

    return error;
}
