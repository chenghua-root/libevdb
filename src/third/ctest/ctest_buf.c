#include <stdarg.h>

#include "ctest_buf.h"
#include "ctest_string.h"

/**
 * 创建一个新的ctest_buf_t
 */
ctest_buf_t *ctest_buf_create(ctest_pool_t *pool, uint32_t size)
{
    ctest_buf_t              *b;

    if ((b = (ctest_buf_t *)ctest_pool_calloc(pool, sizeof(ctest_buf_t))) == NULL)
        return NULL;

    // 一个page大小
    if (size == 0)
        size = pool->end - pool->last;

    if ((b->pos = (char *)ctest_pool_alloc(pool, size)) == NULL)
        return NULL;

    b->last = b->pos;
    b->end = b->last + size;
    b->cleanup = NULL;
    b->args = pool;
    ctest_list_init(&b->node);

    return b;
}

/**
 * 把data包成ctest_buf_t
 */
ctest_buf_t *ctest_buf_pack(ctest_pool_t *pool, const void *data, uint32_t size)
{
    ctest_buf_t              *b;

    if ((b = (ctest_buf_t *)ctest_pool_calloc(pool, sizeof(ctest_buf_t))) == NULL)
        return NULL;

    ctest_buf_set_data(pool, b, data, size);

    return b;
}

/**
 * 设置数据到b里
 */
void ctest_buf_set_data(ctest_pool_t *pool, ctest_buf_t *b, const void *data, uint32_t size)
{
    b->pos = (char *)data;
    b->last = b->pos + size;
    b->end = b->last;
    b->cleanup = NULL;
    b->args = pool;
    b->flags = 0;
    ctest_list_init(&b->node);
}

/**
 * 创建一个ctest_file_buf_t, 用于sendfile等
 */
ctest_file_buf_t *ctest_file_buf_create(ctest_pool_t *pool)
{
    ctest_file_buf_t         *b;

    b = (ctest_file_buf_t *)ctest_pool_calloc(pool, sizeof(ctest_file_buf_t));
    b->flags = CTEST_BUF_FILE;
    b->cleanup = NULL;
    b->args = pool;
    ctest_list_init(&b->node);

    return b;
}

void ctest_file_buf_set_close(ctest_file_buf_t *b)
{
    if ((b->flags & CTEST_BUF_FILE))
        b->flags = CTEST_BUF_CLOSE_FILE;
}

void ctest_buf_set_cleanup(ctest_buf_t *b, ctest_buf_cleanup_pt *cleanup, void *args)
{
    b->cleanup = cleanup;
    b->args = args;
}

void ctest_buf_destroy(ctest_buf_t *b)
{
    ctest_buf_cleanup_pt         *cleanup;
    ctest_list_del(&b->node);

    if ((b->flags & CTEST_BUF_CLOSE_FILE) == CTEST_BUF_CLOSE_FILE)
        close(((ctest_file_buf_t *)b)->fd);

    // cleanup
    if ((cleanup = b->cleanup)) {
        b->cleanup = NULL;
        (*cleanup)(b, b->args);
    }
}

/**
 * 空间不够,分配出一块来,保留之前的空间
 */
int ctest_buf_check_read_space(ctest_pool_t *pool, ctest_buf_t *b, uint32_t size)
{
    int                     dsize;
    char                    *ptr;

    if ((b->end - b->last) >= (int)size)
        return CTEST_OK;

    // 需要大小
    dsize = (b->last - b->pos);
    size = ctest_max(dsize * 3 / 2, size + dsize);
    size = ctest_align(size, CTEST_POOL_PAGE_SIZE);

    // alloc
    if ((ptr = (char *)ctest_pool_alloc(pool, size)) == NULL)
        return CTEST_ERROR;

    // copy old buf to new buf
    if (dsize > 0)
        memcpy(ptr, b->pos, dsize);

    b->pos = ptr;
    b->last = b->pos + dsize;
    b->end = b->pos + size;

    return CTEST_OK;
}

/**
 * 空间不够,分配出一块来,保留之前的空间
 */
ctest_buf_t *ctest_buf_check_write_space(ctest_pool_t *pool, ctest_list_t *bc, uint32_t size)
{
    ctest_buf_t              *b = ctest_list_get_last(bc, ctest_buf_t, node);

    if (b != NULL && (b->end - b->last) >= (int)size)
        return b;

    // 重新生成一个buf,放入buf_chain_t中
    size = ctest_align(size, CTEST_POOL_PAGE_SIZE);

    if ((b = ctest_buf_create(pool, size)) == NULL)
        return NULL;

    ctest_list_add_tail(&b->node, bc);

    return b;
}

/**
 * 清除掉
 */
void ctest_buf_chain_clear(ctest_list_t *l)
{
    ctest_buf_t              *b, *b1;

    ctest_list_for_each_entry_safe(b, b1, l, node) {
        ctest_buf_destroy(b);
    }
    ctest_list_init(l);
}

/**
 * 加到后面
 */
void ctest_buf_chain_offer(ctest_list_t *l, ctest_buf_t *b)
{
    if (!l->next) ctest_list_init(l);

    ctest_list_add_tail(&b->node, l);
}

/**
 * 把s复制到d上
 */
int ctest_buf_string_copy(ctest_pool_t *pool, ctest_buf_string_t *d, const ctest_buf_string_t *s)
{
    if (s->len > 0) {
        d->data = (char *)ctest_pool_alloc(pool, s->len + 1);
        memcpy(d->data, s->data, s->len);
        d->data[s->len] = '\0';
        d->len = s->len;
    }

    return s->len;
}

int ctest_buf_string_printf(ctest_pool_t *pool, ctest_buf_string_t *d, const char *fmt, ...)
{
    int                     len;
    char                    buffer[2048];

    va_list                 args;
    va_start(args, fmt);
    len = ctest_vsnprintf(buffer, 2048, fmt, args);
    va_end(args);
    d->data = (char *)ctest_pool_alloc(pool, len + 1);
    memcpy(d->data, buffer, len);
    d->data[len] = '\0';
    d->len = len;
    return len;
}

int ctest_buf_list_len(ctest_list_t *l)
{
    ctest_buf_t              *b;
    int                     len = 0;

    ctest_list_for_each_entry(b, l, node) {
        len += ctest_buf_len(b);
    }

    return len;
}

