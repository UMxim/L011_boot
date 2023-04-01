extern void *_estack;

void Reset_Handler();
void Default_Handler();
void main(void);

void NMI_Handler()   __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler()   __attribute__((weak, alias("Default_Handler")));

void *vectors[] __attribute__((section(".isr_vector"), used)) = {
    &_estack,
    &Reset_Handler,
    &NMI_Handler,
    &HardFault_Handler
};

void __attribute__((naked, noreturn)) Reset_Handler()
{
   main();
}

void __attribute__((naked, noreturn)) Default_Handler(){
    while(1);
}