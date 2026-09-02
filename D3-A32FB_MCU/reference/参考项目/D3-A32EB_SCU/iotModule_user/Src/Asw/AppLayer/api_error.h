#ifndef __API_ERROR_H
#define __API_ERROR_H


/* 错误码类型 */
typedef int error_t;


#define API_EOK             0       /* 无错 */
#define API_EPERM           1       /* 操作不允许 */
#define API_ENOENT          2       /* 文件或目录不存在    */
#define API_ESRCH           3       /* 进程不存在          */
#define API_EINTR           4       /* 调用被中断          */
#define API_EIO             5       /* I/O 错误            */
#define API_ENXIO           6       /* 设备或地址不存在    */
#define API_E2BIG           7       /* 参数列表太长        */
#define API_ENOEXEC         8       /* 可执行文件格式错误  */
#define API_EBADF           9       /* 文件描述符损坏      */
#define API_ECHILD          10      /* 没有子进程          */
#define API_EAGAIN          11      /* 资源不可用，需重试  */
#define API_ENOMEM          12      /* 空间（内存）不足    */
#define API_EACCES          13      /* 权限不够            */
#define API_EFAULT          14      /* 地址错误            */
#define API_ENOTEMPTY       15      /* 目录非空            */
#define API_EBUSY           16      /* 设备或资源忙        */
#define API_EEXIST          17      /* 文件已经存在        */
#define API_EXDEV           18      /* 跨设备连接          */
#define API_ENODEV          19      /* 设备不存在          */
#define API_ENOTDIR         20      /* 不是目录            */
#define API_EISDIR          21      /* 是目录              */
#define API_EINVAL          22      /* 无效参数            */
#define API_ENFILE          23      /* 系统打开文件太多，描述符不够用 */
#define API_EMFILE          24      /* 打开的文件太多      */
#define API_ENOTTY          25      /* 不合适的I/O控制操作 */
#define API_ENAMETOOLONG    26      /* 文件名太长          */
#define API_EFBIG           27      /* 文件太大            */
#define API_ENOSPC          28      /* 设备剩余空间不足    */
#define API_ESPIPE          29      /* 无效的搜索（Invalid seek） */
#define API_EROFS           30      /* 文件系统只读        */
#define API_EMLINK          31      /* 链接太多            */
#define API_EPIPE           32      /* 损坏的管道          */
#define API_EDEADLK         33      /* 资源可能死锁        */
#define API_ENOLCK          34      /* 无可用（空闲）的锁  */
#define API_ENOTSUP         35      /* 不支持              */
#define API_EMSGSIZE        36      /* 消息太大            */


#define API_TAKE_TIMEOUT    100     /* 获取同步信号超时 */
#define API_TAKE_FAIL       101     /* 获取同步信号失败 */
#define API_RELEASE_FAIL    102     /* 释放同步信号失败 */
#define API_ENODATA         103     /* 无数据 */
#define API_ENULL           104     /* 地址为空 */
#define API_ECRC            105     /* CRC校验异常 */
#define API_EFLEN           106     /* 帧长度异常 */

#endif
