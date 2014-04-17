  SPI_Uart_WriteByte(LCR,0x80); // 0x80 to program baudrate
  
  SPI_Uart_WriteByte(DLL,SPI_Uart_config.DivL); //0x50 = 9600 with Xtal = 12.288MHz
  SPI_Uart_WriteByte(DLM,SPI_Uart_config.DivM); 

  SPI_Uart_WriteByte(LCR, 0xBF); // access EFR register
  SPI_Uart_WriteByte(EFR, SPI_Uart_config.Flow); // enable enhanced registers
  SPI_Uart_WriteByte(LCR, SPI_Uart_config.DataFormat); // 8 data bit, 1 stop bit, no parity
  SPI_Uart_WriteByte(FCR, 0x06); // reset TXFIFO, reset RXFIFO, non FIFO mode
  SPI_Uart_WriteByte(FCR, 0x01); // enable FIFO mode

  // Perform read/write test to check if UART is working
  SPI_Uart_WriteByte(SPR,'H');
  data = SPI_Uart_ReadByte(SPR);

  if(data == 'H'){ 
    return 1; 
  }
  else{ 
    return 0; 
  }
