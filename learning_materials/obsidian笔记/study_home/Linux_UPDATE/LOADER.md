| 阶段      | 瑞芯微闭源方案                | 开源U方案                | 主要职责                |
| ------- | ---------------------- | -------------------- | ------------------- |
| Stage 0 | BootROM                | BootROM              | 芯片固化代码，寻找并加载Loader  |
| Loader1 | DDR Bin / FlashData    | TPL                  | 初始化时钟、DDR           |
| Loader2 | MiniLoader / FlashBoot | SPL                  | 初始化eMMC，读取分区，加载后续镜像 |
| 安全固件    | `trust.img`            | BL31/OP-TEE          | ARM安全世界和异常级切换       |
| 最终阶段    | `uboot.img`            | `u-boot.itb`中的U-Boot | U-Boot proper       |
我们的方案：
1. BOOTROM
2. LOADER(DDR.bin+SPL.bin)
3. UBOOT_A+UBOOT_B