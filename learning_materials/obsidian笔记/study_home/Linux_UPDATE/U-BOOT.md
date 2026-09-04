前置补充：
对于编包要求：
链接地址 = 加载地址 = 入口地址
### 0.1上电启动流程
![[QQ_1788313133079.png]]
### 1.U-BOOT流程：
由Miniloader加载：
1. ATF          BL31
2. OP-TEE    BL32
3. U-BOOT   BL33
4. U-BOOT   DTB
BL31 BL32属于trust.img   BL33属于uboot.img
其中：
1. BL31:
![[QQ_1788315802321.png]]
2.BL32:
![[QQ_1788315960171.png]]
3.BL33:
![[QQ_1788316003203.png]]
4,U-BOOT DTB:
![[QQ_1788316131012.png]]
### 2.源码解读
第0步:loader将uboot程序从flash搬运到DDR内存中目标地址
第1步:配置U-Boot proper地址：将CPU的PC指向0x00a0000
```c
#define CONFIG_SYS_TEXT_BASE        0x00a00000
```
![[QQ_1788332116580.png]]
第2步：执行 b resrt：
![[QQ_1788333275156.png]]
把CPU最基本的运行环境准备好，然后跳转到_main  
第3步：执行_main
![[QQ_1788339737489.png]]
其中：
#### board_init_r()
在U-Boot完成重定位后，把驱动、设备、环境变量和控制台全部初始化好，最终进入main_loop()。
第四步：执行main_loop
mian_loop存在以下两条链路:
1. 执行自动启动命令
2. U-Boot命令行
