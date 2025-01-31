
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_ARRAY_H_INCLUDED_
#define _NGX_ARRAY_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>

/**
 * 数组数据结构
 * 只存储比较小的数据
 * 数组的元素长度在创建数组的时候就固定死了。但是数组个数，会自动扩容
 * 数组的数据结构和元素内存都会分配在Nginx的pool内存池上
 * 数组回收会去检查pool内存池，看是否可以将数组内存交还给内存池
 */ 
typedef struct {
    void        *elts; // 指向数组第一个元素指针
    ngx_uint_t   nelts; // 未使用元素的索引 下一个要使用的索引
    size_t       size; // 每个元素的大小，元素大小固定
    ngx_uint_t   nalloc; // 分配多少个元素
    ngx_pool_t  *pool; // 内存池
} ngx_array_t;


ngx_array_t *ngx_array_create(ngx_pool_t *p, ngx_uint_t n, size_t size);
void ngx_array_destroy(ngx_array_t *a);
void *ngx_array_push(ngx_array_t *a);
void *ngx_array_push_n(ngx_array_t *a, ngx_uint_t n);


static ngx_inline ngx_int_t
ngx_array_init(ngx_array_t *array, ngx_pool_t *pool, ngx_uint_t n, size_t size)
{
    /*
     * set "array->nelts" before "array->elts", otherwise MSVC thinks
     * that "array->nelts" may be used without having been initialized
     */

    array->nelts = 0;
    array->size = size;
    array->nalloc = n;
    array->pool = pool;

    array->elts = ngx_palloc(pool, n * size);
    if (array->elts == NULL) {
        return NGX_ERROR;
    }

    return NGX_OK;
}


#endif /* _NGX_ARRAY_H_INCLUDED_ */
