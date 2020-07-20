#include "bootloader.h"

#include "stm32_def.h"
#include "backup.h"


#if !defined(SYSBL_MAGIC_NUMBER_BKP_INDEX) && defined(ENABLE_BACKUP_SUPPORT)
#define SYSBL_MAGIC_NUMBER_BKP_INDEX LL_RTC_BKP_DR2
#else
#define SYSBL_MAGIC_NUMBER_BKP_INDEX 0
#endif /* SYSBL_MAGIC_NUMBER_BKP_INDEX */

#ifndef SYSBL_MAGIC_NUMBER_BKP_VALUE
#define SYSBL_MAGIC_NUMBER_BKP_VALUE 0x515B
#endif /* SYSBL_MAGIC_NUMBER_BKP_VALUE */

WEAK void jumpToBootloaderRequest(void)
{
  enableBackupDomain();
  setBackupRegister(SYSBL_MAGIC_NUMBER_BKP_INDEX, SYSBL_MAGIC_NUMBER_BKP_VALUE);
  NVIC_SystemReset();
}

WEAK void jumpToBootloader(void)
{
  enableBackupDomain();
  if (getBackupRegister(SYSBL_MAGIC_NUMBER_BKP_INDEX) == SYSBL_MAGIC_NUMBER_BKP_VALUE) {
    setBackupRegister(SYSBL_MAGIC_NUMBER_BKP_INDEX, 0);

    void (*sysMemBootJump)(void);
    uint32_t sysMem_addr = 0x1FFFD800;

#ifdef __HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH
    /* Remap system Flash memory at address 0x00000000 */
    __HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();
#endif

    sysMemBootJump = (void (*)(void))(*((uint32_t *)(sysMem_addr + 4)));
    __set_MSP(*(uint32_t *)sysMem_addr);

    sysMemBootJump();
    while (1);
  }
}


#ifdef BL_LEGACY_LEAF
void dtr_togglingHook(uint8_t *buf, uint32_t *len)
{
  /**
   * Four byte is the magic pack "1EAF" that puts the MCU into bootloader.
   * Check if the incoming contains the string "1EAF".
   * If yes, put the MCU into the bootloader mode.
   */
  if ((*len >= 4) && (buf[0] == '1') && (buf[1] == 'E') && (buf[2] == 'A') && (buf[3] == 'F')) {
    NVIC_SystemReset();
  }
}
#endif /* BL_LEGACY_LEAF */

#ifdef BL_HID
void dtr_togglingHook(uint8_t *buf, uint32_t *len)
{
  /**
   * Four byte is the magic pack "1EAF" that puts the MCU into bootloader.
   * Check if the incoming contains the string "1EAF".
   * If yes, put the MCU into the bootloader mode.
   */
  if ((*len >= 4) && (buf[0] == '1') && (buf[1] == 'E') && (buf[2] == 'A') && (buf[3] == 'F')) {
    enableBackupDomain();
    /* New HID Bootloader (ver 2.2+) */
    setBackupRegister(HID_MAGIC_NUMBER_BKP_INDEX, HID_MAGIC_NUMBER_BKP_VALUE);
#ifdef HID_OLD_MAGIC_NUMBER_BKP_INDEX
    /* Compatibility to the old HID Bootloader (ver <= 2.1) */
    setBackupRegister(HID_OLD_MAGIC_NUMBER_BKP_INDEX, HID_MAGIC_NUMBER_BKP_VALUE);
#endif
    NVIC_SystemReset();
  }
}
#endif /* BL_HID */
