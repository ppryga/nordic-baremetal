/*
 * Copyright (c) 2023 Piotr Pryga
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <drivers/nrfx_common.h>
#include <drivers/nrfx_errors.h>

/* @brief Initialize UARTE driver
 *
 * @return NRFX_SUCCESS if driver is initilized sucessfully, other value in case
 * of errors.
 */
nrfx_err_t uarte_init();

/* @brief Initialize UARTE driver
 *
 * @param data Pointer to data to be send.
 * @param size Pointer to memory where size of data to be send is stored. The
 * same poiner is used to return information how many bytes were send. In case
 * of error the value stored at pointed memory is invalid.
 *
 * @return NRFX_SUCCESS if driver is initilized sucessfully, other value in case
 * of errors.
 */
size_t uarte_tx(uint8_t *data, size_t *size);

/* @brief Initialize UARTE driver
 *
 * @return NRFX_SUCCESS if driver is initilized sucessfully, other value in case
 * of errors.
 */
nrfx_err_t uarte_rx();
