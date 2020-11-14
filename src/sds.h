/* SDSLib, A C dynamic strings library
 *
 * Copyright (c) 2006-2010, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * 关于SDS：
 * 1、通过使用 SDS 而不是 C 字符串， Redis 将获取字符串长度所需的复杂度从 O(N) 降低到了 O(1) ， 这确保了获取字符串长度的工作不会成为 Redis 的性能瓶颈。
 * 2、C 字符串不记录自身长度带来的另一个问题是容易造成缓冲区溢出， SDS 的空间分配策略完全杜绝了发生缓冲区溢出的可能性： 当 SDS API 需要对 SDS 进行修改时， API 会先检查 SDS 的空间是否满足修改所需的要求。
 * 3、通过未使用空间， SDS 实现了空间预分配和惰性空间释放两种优化策略。
 *      在 SDS 中， buf 数组的长度不一定就是字符数量加一， 数组里面可以包含未使用的字节。
 * /

#ifndef __SDS_H
#define __SDS_H

/*
 * 最大预分配长度
 * 1、如果对 SDS 进行修改之后， SDS 的长度（也即是 len 属性的值）将小于 1 MB ， 那么程序分配和 len 属性同样大小的未使用空间， 这时 SDS len 属性的值将和 free 属性的值相同。
 * 2、如果对 SDS 进行修改之后， SDS 的长度将大于等于 1 MB ， 那么程序会分配 1 MB 的未使用空间。
 */
#define SDS_MAX_PREALLOC (1024*1024)

#include <sys/types.h>
#include <stdarg.h>

/*
 * 类型别名，用于指向 sdshdr 的 buf 属性
 * HEL:
 *  sds 本质上是一个char * ， sdshdr结构的buf属性本质上也是一个 char * 类型，sds 可以作为 sdshdr 的 buf 属性 类型的别名
 *      在使用时，sds 特指 sdshdr 对象的buf属性，而且基于 sds 指针值的 反向指针操作 可以获取 以 sds作为buf属性值的sdshdr对象的地址
 *      随处可见的反向操作：sdshdr *sh = (void*)(s-(sizeof(struct sdshdr)))
*/
typedef char *sds;

/*
 * 保存字符串对象的结构
 *  HEL tips:
 *      sdshdr 的 hdr 应该是 holder 的缩写
 */
struct sdshdr {
    
    // buf 中已占用空间的长度
    int len;

    // buf 中剩余可用空间的长度
    int free;

    // 数据空间
    char buf[];
};

/*
 * 返回 sds 实际保存的字符串的长度
 *
 * T = O(1)
 *
 * HEL:
 *  请求参数：sds是一个 char * 指针类型 的别名，请求参数 s 是sds类型，即与 sdshdr.buf 字段类型相同
 *  s-(sizeof(struct sdshdr)) 是一个指针地址操作，基于s的指针地址值，往回移动 sizeof(sdshdr)个位置
 *      即返回一个新的指针值，指向包含以 s 作为 buf字段值 的 sdshdr变量的地址
 *      即返回一个指向 sdshdr变量的指针
 */
static inline size_t sdslen(const sds s) {
    struct sdshdr *sh = (void*)(s-(sizeof(struct sdshdr)));
    return sh->len;
}

/*
 * 返回 sds 可用空间的长度
 *
 * T = O(1)
 */
static inline size_t sdsavail(const sds s) {
    struct sdshdr *sh = (void*)(s-(sizeof(struct sdshdr)));
    return sh->free;
}

/**
 * 基于 init指向的字符串 和 initlen指定的初始化长度，新建一个 sds对象
*/
sds sdsnewlen(const void *init, size_t initlen);

/**
 * 基于 init指向的字符串新建一个 sds对象
*/
 */
sds sdsnew(const char *init);
sds sdsempty(void);
size_t sdslen(const sds s);
sds sdsdup(const sds s);
void sdsfree(sds s);
size_t sdsavail(const sds s);
sds sdsgrowzero(sds s, size_t len);
sds sdscatlen(sds s, const void *t, size_t len);

/**
 * HEL:
 * 将 t 指向的字符串拼接到 s存储到字符串后面
 * 注意：第一个请求参数并不是 sdshdr 类型，而是 sds 类型
 */
sds sdscat(sds s, const char *t);

/**
 * 将 s 和 t 两个sds指向的字符串拼接起来，生存一个新的sdshdr对象，并返回对应的 sds 地址值
*/
sds sdscatsds(sds s, const sds t);

/**
 *
*/
sds sdscpylen(sds s, const char *t, size_t len);
sds sdscpy(sds s, const char *t);

sds sdscatvprintf(sds s, const char *fmt, va_list ap);
#ifdef __GNUC__
sds sdscatprintf(sds s, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
#else
sds sdscatprintf(sds s, const char *fmt, ...);
#endif

sds sdscatfmt(sds s, char const *fmt, ...);
sds sdstrim(sds s, const char *cset);
void sdsrange(sds s, int start, int end);
void sdsupdatelen(sds s);
void sdsclear(sds s);
int sdscmp(const sds s1, const sds s2);
sds *sdssplitlen(const char *s, int len, const char *sep, int seplen, int *count);
void sdsfreesplitres(sds *tokens, int count);
void sdstolower(sds s);
void sdstoupper(sds s);
sds sdsfromlonglong(long long value);
sds sdscatrepr(sds s, const char *p, size_t len);
sds *sdssplitargs(const char *line, int *argc);
sds sdsmapchars(sds s, const char *from, const char *to, size_t setlen);
sds sdsjoin(char **argv, int argc, char *sep);

/* Low level functions exposed to the user API */
sds sdsMakeRoomFor(sds s, size_t addlen);
void sdsIncrLen(sds s, int incr);
sds sdsRemoveFreeSpace(sds s);
size_t sdsAllocSize(sds s);

#endif
