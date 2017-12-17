# SPD Firmware guidelines
This example and library was build upon STM32F0 microcontroller.

## Instructions how to setup & use
User can/must edit specific #defines in sdp.h and sdp_user_xxxx() functions in sdp_user.c
SDP relies on ring_buffer.h and.c files for storing and handling all received bytes.

1. Configure UART driver: 8 data bits, 1 start, 1 stop bit, no parity checking, no hardware flow control. Baud rate must
    be low enough so that byte interrupt does not overload CPU. Tested with 115200 bps.
    Enable RX not empty interrup and set up IRQ handler.

2. Code:
  - Setup #defines in sdp.h file (User includes and User defines)      
  - Create your uart and data node (or more nodes) structures as global variable in main.c
    ```
    SDP_uart_t cu_uart;
    SDP_data_t cu_node;
    ```      
    Note: each node should have its own ID. In this example cu_node has id CU_NODE_ID, which is 0.
      
  - Edit structure fields (example):
    ```
    cu_uart.handle = USART1; // PC <-> CU use UART1 handle
    cu_uart.rx_timeout = 3; //[ms] (byte) - highest priority (besides systick) - no nested interrupts
    cu_uart.tx_timeout = 20; //[ms] (byte) - possible delay because of other interrupts
    ```
    Optionally, user can modify node`s `rx_msg_timeout` and `tx_msg_timeout` field.
   
  - Add your CRC 16-bit imlementation: either use hardware based CRC calculation or implement custom function. 
  In this example, STM32 LL CRC library was used. Add CRC calculation code in 
  ```
  sdp_user_calculate_crc() function.
  ```
  Note: it is possible to disable CRC checking by setting #define SDP_CRC_SIZE to 0.
      
  - Init SDP by calling (do that for each node):
    ```
    sdp_init_node(&cu_node, &cu_uart, CU_NODE_ID)
    ```
      
  - Implement reading & transmitting functions. Edit sdp_user.c and your interrupt handlers
    - Edit `sdp_user_receive_byte()` to receive one byte.
    - Edit `sdp_user_transmit_byte()` to transmit one byte. 
      Note: For both, receive and transmit functions, user should check for timeouts, handle tx empty and tx complete flags.

  - Add `sdp_receive_data()` to uart interrupt routine and call it when RX not empty flag is set. See example file stm32f0xx_it.c
  - Add `sdp_parse_rx_data()` to main.c in while(1) loop. Call this function as frequently as possible to handle data in time.
    
  - Edit `sdp_user_handle_message()` accordingly to your needs.
    !!! Important note: this function is called only on correctly received message. This means, 
    user MUST send response with `sdp_send_response()` in this handler (or elsewhere in code) in transmiter node tx_msg_timeout time.
    User can also send dummy response (with no payload) with `sdp_send_dummy_response()`;
    
  - Optionally: add log/error handler to `sdp_debug()`. See error codes in function comment section.
    
3. Usage:
  - To send data, easiest way is to create array of uint8_t bytes and fill it up with your data, than call:
  ```
  sdp_send_data()
  ```
  Notes:
    - This function retransmit (if errors) accordingly to `SDP_RETRANSMIT` and than returns true/false on success/error.
    - On success, receiver response is stored in node rx_data array, while response size is returned with `sdp_send_data()`
  
  - To receive data, check section 3.2 last segment. User can handle messages in `sdp_user_handle_message()` and MUST 
  implement response with `sdp_send_response()`. 

   - Note: User should handle uart/communication port hardware errors (like overrun, noise or other enabled interrupts) which
    can`t be detected using protocol framing CRC. Most of the time this means flushing and re-enabling interrupts or
    re-initializing driver. It is not really possible to include instructions for such errors. 
