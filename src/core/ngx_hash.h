
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_HASH_H_INCLUDED_
#define _NGX_HASH_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>

//  nginx中的哈希表一个特别之处在于, 这个hash表是静态只读的，即不能在运行时动态添加新元素的，一切的结构和数据都在配置初始化的时候就已经规划完毕

/**
 * 存储hash的元素
 */ 
typedef struct {
    void             *value; // 指向value的指针
    u_short           len; // key的长度
    u_char            name[1]; // 柔性数组，一个字节的u_char数组，指向key的第一个地址，key长度为变长 搜索 0或1长度数组
} ngx_hash_elt_t;

/**
 * 基本哈希表结构
 */ 
typedef struct {
    ngx_hash_elt_t  **buckets; // 第一个槽的地址
    ngx_uint_t        size; // 槽的总个数
} ngx_hash_t;

/**
 * 支持通配符的哈希表机构
 * 用于表示前置或后置通配符的哈希表 如*.test.com www.test.*
 */ 
typedef struct {
    ngx_hash_t        hash;
    void             *value;
} ngx_hash_wildcard_t;

/**
 * 
 */ 
typedef struct {
    ngx_str_t         key;
    ngx_uint_t        key_hash;
    void             *value;
} ngx_hash_key_t;


typedef ngx_uint_t (*ngx_hash_key_pt) (u_char *data, size_t len);


typedef struct {
    ngx_hash_t            hash; // 普通hash
    ngx_hash_wildcard_t  *wc_head; // 向前通配符hash
    ngx_hash_wildcard_t  *wc_tail; // 向后通配符hash
} ngx_hash_combined_t;

/**
 * 用于初始化哈希表，初始化哈希表的槽的总数并不是由max_size成员决定的，
 * 而是由在做初始化时预先加入到哈希表的所有元素决定的，包括这些元素的总数、每个元素的关键字长度等，还包括操作系统的页面大小
 */ 
typedef struct {
    ngx_hash_t       *hash; // 指向普通的完全匹配哈希表
    ngx_hash_key_pt   key; // 哈希方法

    ngx_uint_t        max_size; // 哈希表中槽的最大个数
    ngx_uint_t        bucket_size; // 哈希表中一个槽的空间大小，不是sizeof(ngx_hash_elt_t)

    char             *name; // 哈希表的名称
    ngx_pool_t       *pool; // 内存池，负责分配基本哈希列表、前置通配哈希列表、后置哈希列表中所有槽
    ngx_pool_t       *temp_pool; // 临时内存池，它仅存在初始化哈希表之前。用于分配一些临时的动态数组，带通配符的元素初始化时需要用到临时动态数组
} ngx_hash_init_t;


#define NGX_HASH_SMALL            1
#define NGX_HASH_LARGE            2

#define NGX_HASH_LARGE_ASIZE      16384
#define NGX_HASH_LARGE_HSIZE      10007

#define NGX_HASH_WILDCARD_KEY     1
#define NGX_HASH_READONLY_KEY     2

/**
 * 这里设计了3个简易的哈希列表( keys_hash、dns_wc_head_hash、dns_wc_tail_hash)，即采用分离链表法来解决冲突，
 * 这样做的好处是如果没有这三个分离链表法来解决冲突的建议哈希列表，
 * 那么每添加一个关键字元素都要遍历数组（数组采用开放地址法解决冲突，冲突就必须遍历）
 */ 
typedef struct {
    ngx_uint_t        hsize; // 散列中槽总数

    ngx_pool_t       *pool; // 内存池，用于分配永久性的内存
    ngx_pool_t       *temp_pool; // 临时内存池，下面的临时动态数组都是由临时内存池分配

    ngx_array_t       keys; // 存放所有非通配符key的数组 
    ngx_array_t      *keys_hash; // 这是个二维数组，第一个维度代表的是bucket的编号，那么keys_hash[i]中存放的是所有的key算出来的hash值对hsize取模以后的值为i的key。假设有3个key,分别是key1,key2和key3假设hash值算出来以后对hsize取模的值都是i，那么这三个key的值就顺序存放在keys_hash[i][0],keys_hash[i][1], keys_hash[i][2]。该值在调用的过程中用来保存和检测是否有冲突的key值，也就是是否有重复。

    ngx_array_t       dns_wc_head; // 存放前向通配符key被处理完成以后的值。比如：“*.abc.com”被处理完成以后，变成“com.abc.”被存放在此数组中。
    ngx_array_t      *dns_wc_head_hash;// 该值在调用的过程中用来保存和检测是否有冲突的前向通配符的key值，也就是是否有重复。

    ngx_array_t       dns_wc_tail; // 存放后向通配符key被处理完成以后的值。比如：“mail.xxx.*”被处理完成以后，变成“mail.xxx.”被存放在此数组中。
    ngx_array_t      *dns_wc_tail_hash; // 该值在调用的过程中用来保存和检测是否有冲突的后向通配符的key值，也就是是否有重复
} ngx_hash_keys_arrays_t;


typedef struct {
    ngx_uint_t        hash;
    ngx_str_t         key;
    ngx_str_t         value;
    u_char           *lowcase_key;
} ngx_table_elt_t;


void *ngx_hash_find(ngx_hash_t *hash, ngx_uint_t key, u_char *name, size_t len);
void *ngx_hash_find_wc_head(ngx_hash_wildcard_t *hwc, u_char *name, size_t len);
void *ngx_hash_find_wc_tail(ngx_hash_wildcard_t *hwc, u_char *name, size_t len);
void *ngx_hash_find_combined(ngx_hash_combined_t *hash, ngx_uint_t key,
    u_char *name, size_t len);

ngx_int_t ngx_hash_init(ngx_hash_init_t *hinit, ngx_hash_key_t *names,
    ngx_uint_t nelts);
ngx_int_t ngx_hash_wildcard_init(ngx_hash_init_t *hinit, ngx_hash_key_t *names,
    ngx_uint_t nelts);

#define ngx_hash(key, c)   ((ngx_uint_t) key * 31 + c)
ngx_uint_t ngx_hash_key(u_char *data, size_t len);
ngx_uint_t ngx_hash_key_lc(u_char *data, size_t len);
ngx_uint_t ngx_hash_strlow(u_char *dst, u_char *src, size_t n);


ngx_int_t ngx_hash_keys_array_init(ngx_hash_keys_arrays_t *ha, ngx_uint_t type);
ngx_int_t ngx_hash_add_key(ngx_hash_keys_arrays_t *ha, ngx_str_t *key,
    void *value, ngx_uint_t flags);


#endif /* _NGX_HASH_H_INCLUDED_ */
