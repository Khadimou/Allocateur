/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_malloc.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jterrazz <jterrazz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/05/30 18:47:16 by jterrazz          #+#    #+#             */
/*   Updated: 2019/07/22 12:17:27 by jterrazz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "test.h"
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "malloc.h"
#include "/home/khadimou/Images/Allocateur_Memoire/protos.h"

t_heap			*get_last_heap(t_heap *heap)
{
	if (!heap)
		return (NULL);
	while (heap->next)
		heap = heap->next;
	return (heap);
}

void	ft_itoa_base(size_t nb, char base, char length, t_bool prefix)
{
	char	*str;

	str = "0123456789ABCDEFGHIJKLMNOPQRSTUIVWXYZ";
	if (nb / base)
		ft_itoa_base(nb / base, base, length - 1, prefix);
	else
	{
		if (prefix)
			ft_putstr("0x");
		while (--length > 0)
		{
			ft_putstr("0");
		}
	}
	write(1, &str[nb % base], 1);
}

static size_t	print_block_list(t_block *block)
{
	char	*start_address;
	char	*end_address;
	size_t	total;

	total = 0;
	start_address = NULL;
	end_address = NULL;
	while (block)
	{
		start_address = (char *)BLOCK_SHIFT(block);
		end_address = start_address + block->data_size;
		if (!block->freed)
		{
			ft_itoa_base((size_t)start_address, 16, 9, TRUE);
			ft_putstr(" - ");
			ft_itoa_base((size_t)end_address, 16, 9, TRUE);
			ft_putstr(" : ");
			ft_itoa_base(block->data_size, 10, 0, FALSE);
			ft_putstr(" octets\n");
			total += block->data_size;
		}
		block = block->next;
	}
	return (total);
}


static void		print_heap_header(char *name, t_heap *heap)
{
	ft_putstr(name);
	ft_putstr(" : ");
	ft_itoa_base((size_t)heap, 16, 9, TRUE);
	ft_putstr("\n");
}


void			start_show_alloc_mem(void)
{
	t_heap	*first_heap;
	t_heap	*last_heap;
	size_t	total;

	total = 0;
	first_heap = g_heap_anchor;
	last_heap = get_last_heap(first_heap);
	while (last_heap)
	{
		if (last_heap->group == TINY)
			print_heap_header("TINY", last_heap);
		else if (last_heap->group == SMALL)
			print_heap_header("SMALL", last_heap);
		else
			print_heap_header("LARGE", last_heap);
		if (last_heap->block_count)
			total += print_block_list((t_block *)HEAP_SHIFT(last_heap));
		last_heap = last_heap->prev;
	}
	ft_putstr("Total : ");
	ft_itoa_base(total, 10, 0, FALSE);
	ft_putstr(" octets\n");
}

void			show_alloc_mem(void)
{
	pthread_mutex_lock(&g_ft_malloc_mutex);
	start_show_alloc_mem();
	pthread_mutex_unlock(&g_ft_malloc_mutex);
}


void	ft_putstr(char const *s)
{
	int i;

	i = 0;
	while (s[i])
		i++;
	write(1, s, i);
}

void	ft_itoa_fd(size_t nb, char base, int fd, t_bool prefix)
{
	char *str;

	str = "0123456789ABCDEFGHIJKLMNOPQRSTUIVWXYZ";
	if (nb / base)
		ft_itoa_fd(nb / base, base, fd, prefix);
	else if (prefix)
		write(fd, "0x", 2);
	write(fd, &str[nb % base], 1);
}

void	print_heap_group(t_heap *heap)
{
	if (heap->group == TINY)
		ft_putstr("TINY");
	else if (heap->group == SMALL)
		ft_putstr("SMALL");
	else
		ft_putstr("LARGE");
}

static void	print_heap_description(t_heap *heap)
{
	print_heap_group(heap);
	ft_putstr(" - ");
	ft_itoa_base((size_t)heap, 16, 0, TRUE);
	ft_putstr("\n");
}

static void	print_heap_hex_line(char *start)
{
	uint8_t i;

	i = 0;
	ft_itoa_base((size_t)start, 16, 0, TRUE);
	while (i < 16)
	{
		ft_putstr(" ");
		ft_itoa_base((unsigned char)start[i], 16, 2, FALSE);
		i++;
	}
	ft_putstr("\n");
}


static void	start_show_alloc_mem_hex(void)
{
	t_heap	*heap ;
	size_t	i;
	char	*ptr;

	heap = g_heap_anchor;
	while (heap)
	{
		print_heap_description(heap);
		i = 0;
		ptr = (char *)heap;
		while (i < heap->total_size)
		{
			print_heap_hex_line(ptr + i);
			i += 16;
		}
		heap = heap->next;
	}
}

void		show_alloc_mem_hex(void)
{
	pthread_mutex_lock(&g_ft_malloc_mutex);
	start_show_alloc_mem_hex();
	pthread_mutex_unlock(&g_ft_malloc_mutex);
}

static void test_malloc_null()
{
    void *t = malloc(0);
    if (t)
        printf("malloc(0) should return NULL\n");
    free(t);
}

static void test_malloc_one()
{
    char *t = (char *)malloc(1);
    if (!t) {
        printf("malloc(1) should return ptr\n");
        return;
    }
    t[0] = 0xFF;
    t[1] = 0xFF;
    t[2] = 0xFF;
    t[3] = 0xFF;
    show_alloc_mem_hex();

    t[0] = 0;
    free(t);
}

static void test_malloc_getpagesize()
{
    void *t = malloc(4096);
    free(t);
}

// !!!!!!!!!!!!!!!!
// TODO SHOULD FILL 3 sizes
// CHECK To free all, see if the TINY and SMALL stay
static void test_malloc_limits()
{
    void	*t	= malloc(1);
    void	*t0	= malloc(TINY_BLOCK_SIZE);
    void	*t00	= malloc(TINY_BLOCK_SIZE);
    void	*t000	= malloc(TINY_BLOCK_SIZE);
    void	*t1	= malloc(SMALL_BLOCK_SIZE);
    void	*t2	= malloc(SMALL_BLOCK_SIZE + 1);

    // Should print mallocs in all categories (TINY, SMALL, LARGE)
     //show_alloc_mem();
     show_alloc_mem_hex();
     //show_heap_list();
    free(t0);

    t0 = malloc(TINY_BLOCK_SIZE - sizeof(t_block));
    // show_alloc_mem();
    free(t0);
    free(t00);
    free(t000);
    free(t1);
    free(t2);
	free(t);
}

static void test_malloc_free_size()
{
    void *t = malloc(SMALL_BLOCK_SIZE + 1);

    // heap should have 0 free_space
    //show_heap_list();

    free(t);
}

void run_test_malloc(void)
{
    test_malloc_null();
    test_malloc_one();
    test_malloc_getpagesize();
    test_malloc_limits();
    test_malloc_free_size();

    //show_alloc_mem();
    //show_heap_list();
}


    // ft_putstr("data_size: ");
    // ft_itoa_base(block->data_size, 10, 0, FALSE);
    // ft_putstr(" sup to : ");
    // ft_itoa_base(size + sizeof(t_block), 10, 0, FALSE);
    // ft_putstr("\n");
