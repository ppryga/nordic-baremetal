#include <stdint.h>
#include <stdio.h>

#include <drivers/include/nrfx_systick.h>
#include <drivers/include/nrfx_uarte.h>
#include <drivers/nrfx_errors.h>

#include <core_cm4.h>

#define UARTE0_PIN_RX 8
#define UARTE0_PIN_TX 6
#define UARTE0_PIN_RTS 5
#define UARTE0_PIN_CTS 7

nrfx_systick_state_t counter;

nrfx_uarte_config_t uarte0_cfg =
    NRFX_UARTE_DEFAULT_CONFIG(UARTE0_PIN_TX, UARTE0_PIN_RX);
nrfx_uarte_t uarte0 = NRFX_UARTE_INSTANCE(0);

extern void SysTick_Handler(void);

uint64_t tick_cnt = 0;
static uint8_t tx_buffer[] = "Nordic Semiconductor";

void SysTick_Handler(void) {
  tick_cnt++;

  //   nrfx_err_t err;

  // err = nrfx_uarte_tx(&uarte0, tx_buffer, sizeof(tx_buffer), 0);
  printf("Nordic semi\n");
  //   if (err != NRFX_SUCCESS) {
  //     return;
  //   }

  SysTick->LOAD = (0xFFFFFF);

  return;
}

// int *(*foo)[sizeof(tx_buffer)] = 1;

void uarte0_event_handle(nrfx_uarte_event_t const *p_event, void *p_context) {
  // do sometingh here
}

int _write(int handle, char *data, int size) {
  int count = size;

  nrfx_err_t err = nrfx_uarte_tx(&uarte0, (uint8_t *)data, size, 0);

  if (err != NRFX_SUCCESS) {
    return -1;
  }

  return size;
}

int main(void) {

  nrfx_systick_init();

  nrfx_err_t err;

  err = nrfx_uarte_init(&uarte0, &uarte0_cfg, uarte0_event_handle);
  if (err != NRFX_SUCCESS) {
    return err;
  }

  SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
  SysTick->LOAD = (0x1 << 24);

  while (1) {
    nrfx_systick_get(&counter);
    // tick_cnt++;
  }
  return 0;
}
