#include <sam.h>

#define NUM_LEDS 6
#define LED_START_PIN 16
#define LED_MASK (0x3F << LED_START_PIN)  // Pins PA16-PA21

#define BUTTON_PIN 15  

// Global variables
volatile uint8_t led_position = 0;
volatile int8_t direction = 1;  // 1 for right, -1 for left
volatile uint8_t is_running = 0;
volatile uint32_t last_button_time = 0;

void SystemInit(void);
void LED_Init(void);
void TCC0_Init(void);
void EIC_Init(void);
void update_leds(void);

void TCC0_Handler(void);
void EIC_Handler(void);

int main(void) {
    SystemInit();
    
    while (1) {
      
        __WFI();  
    }
}

void SystemInit(void) {
  
    
    LED_Init();
    TCC0_Init();
    EIC_Init();
}

void LED_Init(void) {
    // Enable PORT A
    PM->APBAMASK.reg |= PM_APBAMASK_PORT;
    
    // Set LED pins as outputs
    PORT->Group[0].DIRSET.reg = LED_MASK;
    
    // Turn off all LEDs initially
    PORT->Group[0].OUTCLR.reg = LED_MASK;
}

void TCC0_Init(void) {
    // Enable TCC0 peripheral clock
    PM->APBCMASK.reg |= PM_APBCMASK_TCC0;
    
    // Configure GCLK for TCC0
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | 
                        GCLK_CLKCTRL_GEN_GCLK0 | 
                        GCLK_CLKCTRL_ID_TCC0_TCC1;
    while (GCLK->STATUS.bit.SYNCBUSY);
    
    // Disable TCC0 before configuration
    TCC0->CTRLA.bit.ENABLE = 0;
    while (TCC0->SYNCBUSY.bit.ENABLE);
    
    // Reset TCC0
    TCC0->CTRLA.bit.SWRST = 1;
    while (TCC0->SYNCBUSY.bit.SWRST || TCC0->CTRLA.bit.SWRST);
    
    // Set prescaler to 64 (48MHz/64 = 750kHz)
    TCC0->CTRLA.reg |= TCC_CTRLA_PRESCALER_DIV64;
    
    // Set period for 1ms interrupt (750kHz/750 = 1kHz)
    TCC0->PER.reg = 750 - 1;
    while (TCC0->SYNCBUSY.bit.PER);
    
    // Enable overflow interrupt
    TCC0->INTENSET.reg = TCC_INTENSET_OVF;
    
    // Clear any pending interrupts
    TCC0->INTFLAG.reg = TCC_INTFLAG_OVF;
    
    // Enable TCC0
    TCC0->CTRLA.bit.ENABLE = 1;
    while (TCC0->SYNCBUSY.bit.ENABLE);
    
    // Enable TCC0 interrupt in NVIC
    NVIC_EnableIRQ(TCC0_IRQn);
    NVIC_SetPriority(TCC0_IRQn, 1);
}

void EIC_Init(void) {
    // Enable EIC peripheral clock
    PM->APBAMASK.reg |= PM_APBAMASK_EIC;
    
    // Configure GCLK for EIC
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | 
                        GCLK_CLKCTRL_GEN_GCLK0 | 
                        GCLK_CLKCTRL_ID_EIC;
    while (GCLK->STATUS.bit.SYNCBUSY);
    
    // Configure button pin (PA15) for EIC
    PORT->Group[0].PINCFG[BUTTON_PIN].reg = PORT_PINCFG_PMUXEN | PORT_PINCFG_INEN;
    PORT->Group[0].PMUX[BUTTON_PIN / 2].reg &= ~(0xF << (4 * (BUTTON_PIN % 2)));
    PORT->Group[0].PMUX[BUTTON_PIN / 2].reg |= (0x0 << (4 * (BUTTON_PIN % 2)));  
    
    EIC->CTRL.bit.ENABLE = 0;
    while (EIC->STATUS.bit.SYNCBUSY);
    
    // Reset EIC
    EIC->CTRL.bit.SWRST = 1;
    while (EIC->STATUS.bit.SYNCBUSY || EIC->CTRL.bit.SWRST);
    
    EIC->CONFIG[0].reg |= EIC_CONFIG_SENSE0_FALL | 
                          EIC_CONFIG_FILTEN0 | 
                          (BUTTON_PIN << EIC_CONFIG_EXTINTEO_Pos);
    
    EIC->INTENSET.reg = (1 << BUTTON_PIN);
    
    EIC->INTFLAG.reg = (1 << BUTTON_PIN);
    =
    EIC->CTRL.bit.ENABLE = 1;
    while (EIC->STATUS.bit.SYNCBUSY);
    
    NVIC_EnableIRQ(EIC_IRQn);
    NVIC_SetPriority(EIC_IRQn, 0);
}

void update_leds(void) {
    // Turn off all LEDs
    PORT->Group[0].OUTCLR.reg = LED_MASK;
    
    // Turn on the current LED
    PORT->Group[0].OUTSET.reg = (1 << (LED_START_PIN + led_position));
}

void TCC0_Handler(void) {
    if (TCC0->INTFLAG.bit.OVF) {
        // Clear overflow flag
        TCC0->INTFLAG.reg = TCC_INTFLAG_OVF;
        
        if (is_running) {
            // Update LED position
            led_position += direction;
            
            // Change direction if at ends
            if (led_position == 0 || led_position == (NUM_LEDS - 1)) {
                direction = -direction;
            }
            
            // Update LEDs
            update_leds();
        }
    }
}
void EIC_Handler(void) {
    if (EIC->INTFLAG.reg & (1 << BUTTON_PIN)) {
        // Clear interrupt flag
        EIC->INTFLAG.reg = (1 << BUTTON_PIN);
        
        // Simple software debounce (check if at least 200ms have passed)
        uint32_t current_time = TCC0->COUNT.reg;
        if ((current_time - last_button_time) > 200) {
            last_button_time = current_time;
            
            if (!is_running) {
                // Start the sequence
                is_running = 1;
            } else {
                // Stop the sequence and reset
                is_running = 0;
                led_position = 0;
                direction = 1;
                update_leds();
            }
        }
    }
}
