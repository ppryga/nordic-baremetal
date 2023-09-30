/*
 * Copyright (c) 2023 Piotr Pryga
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <drivers/include/nrfx_uarte.h>
#include <drivers/nrfx_errors.h>

/* As of now, use fixed GPIO PINs. This couples the driver with nRF52833DK. */
#define UARTE0_PIN_RX  8
#define UARTE0_PIN_TX  6
#define UARTE0_PIN_RTS 5
#define UARTE0_PIN_CTS 7

/* Flags are not used by the nRFX driver for TX */
#define UARTE_FLAGS_DEFAULT 0

static nrfx_uarte_config_t uarte0_cfg = NRFX_UARTE_DEFAULT_CONFIG(UARTE0_PIN_TX, UARTE0_PIN_RX);
static nrfx_uarte_t uarte0 = NRFX_UARTE_INSTANCE(0);

/* Use of the handler in nRFx UARTE driver makes it to work in not-blocking
 * mode.
 */
void uarte0_event_handle(nrfx_uarte_event_t const *p_event, void *p_context)
{
	/* TODO: If blcoking mode is required and multiple execution contexts are
	 * supported synchronization object is required. */
}

nrfx_err_t uarte_init()
{
	nrfx_err_t err;

	err = nrfx_uarte_init(&uarte0, &uarte0_cfg, NULL); // uarte0_event_handle);
	if (err != NRFX_SUCCESS) {
		return err;
	}

	return NRFX_SUCCESS;
}

int uarte_tx(uint8_t *data, size_t *size)
{

	if (!data) {
		return NRFX_ERROR_NULL;
	}

	if (!(*size)) {
		return NRFX_ERROR_INVALID_LENGTH;
	}

	nrfx_err_t err = nrfx_uarte_tx(&uarte0, data, *size, UARTE_FLAGS_DEFAULT);

	return err;
}