# weather-station

weather station with arduino micro


<b>Irrigation and wearter indoor station</b>






<b>Outdoor weather station</b>


Arduino micro pin configuration:


VI                                              
GND     GND             GND                         
3V                                                  
5V      VCC             5V     

RX (0)  RS485 converter         RO                                                          
TX (1)  RS485 converter         DI                                                          
2       3.3V level converter    bidirectional bus                   
3       3.3V level converter    bidirectional bus           
4       RS485 converter         DE, RE                                          
5       DHT22                   DATA
A0                              voltage measurement 
A1                              current measurement //not used
A2                              photoseristor


<b>Communication cable</b>
<br>communication bus:  RS485
<br>cable type:         alarm cable 4x0.22 shielded
<br>hearts: 
<br>    red:    VCC
<br>    white:  GND
<br>    yellow: RS485 A
<br>    green:  RS485 B
