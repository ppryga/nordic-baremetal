/*
 * Copyright (c) 2023 Piotr Pryga
 *
 * SPDX-License-Identifier: Apache-2.0
 * 
 * The file contains functions that are expected by C library to provide access to hardware e.g. _write is required to
 * use printf function. 
 * TODO: add here better description.
 */

#include <errno.h>
#include <stdint.h>

#include <drivers/uart.h>

int _write(int handle, char *data, size_t size)
{

	nrfx_err_t ret = uarte_tx((uint8_t *)data, &size);

	if (ret != NRFX_SUCCESS) {
		errno = EIO;
		return -1;
	}

	return size;
}
