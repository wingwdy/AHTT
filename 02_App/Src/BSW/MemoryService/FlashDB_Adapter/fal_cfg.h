/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-05-17     armink       the first version
 */

#ifndef _FAL_CFG_H_
#define _FAL_CFG_H_



#define NOR_FLASH_DEV_NAME                  "norflash0"

#define FAL_KVDB_NAME_PARA                  "para_kvdb"
#define FAL_TSDB_NAME_CHARGE_RECORD         "charge_record_tsdb"
#define FAL_TSDB_NAME_ERROR_RECORD          "error_record_tsdb"

#define FAL_KVDB_PARA_START_ADDR            (0)
#define FAL_KVDB_PARA_SIZE                  (1 * 256 * 1024)
#define FAL_TSDB_CHARGE_RECORD_START_ADDR   (FAL_KVDB_PARA_SIZE + FAL_KVDB_PARA_START_ADDR)
#define FAL_TSDB_CHARGE_RECORD_SIZE         (1 * 256 * 1024)
#define FAL_TSDB_ERROR_RECORD_START_ADDR    (FAL_TSDB_CHARGE_RECORD_SIZE + FAL_TSDB_CHARGE_RECORD_START_ADDR)
#define FAL_TSDB_ERROR_RECORD_SIZE          (1 * 256 * 1024)

#define FAL_PART_HAS_TABLE_CFG
/* ===================== Flash device Configuration ========================= */
extern const struct fal_flash_dev nor_flash0;

/* flash device table */
#define FAL_FLASH_DEV_TABLE                                          \
{                                                                    \
    &nor_flash0,                                                     \
}
/* ====================== Partition Configuration ========================== */
#ifdef FAL_PART_HAS_TABLE_CFG
/* partition table */
#define FAL_PART_TABLE                                                                                                                            \
{                                                                                                                                                 \
    {FAL_PART_MAGIC_WORD, FAL_KVDB_NAME_PARA,          NOR_FLASH_DEV_NAME,  FAL_KVDB_PARA_START_ADDR,            FAL_KVDB_PARA_SIZE,          0}, \
    {FAL_PART_MAGIC_WORD, FAL_TSDB_NAME_CHARGE_RECORD, NOR_FLASH_DEV_NAME,  FAL_TSDB_CHARGE_RECORD_START_ADDR,   FAL_TSDB_CHARGE_RECORD_SIZE, 0}, \
    {FAL_PART_MAGIC_WORD, FAL_TSDB_NAME_ERROR_RECORD,  NOR_FLASH_DEV_NAME,  FAL_TSDB_ERROR_RECORD_START_ADDR,    FAL_TSDB_ERROR_RECORD_SIZE,  0}, \
}
#endif /* FAL_PART_HAS_TABLE_CFG */

#endif /* _FAL_CFG_H_ */
