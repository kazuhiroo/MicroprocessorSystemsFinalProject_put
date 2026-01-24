% * @file    : serial_port_test.m %
% * @author  : a.katowski, https://github.com/kazuhiroo
% * @date    : Jan 24, 2026
% * @brief   : Tracking the response and data received from UART via MATLAB
%              script
% 


s = serialport("COM3", 115200);  % set UART and baudrate
configureTerminator(s,"CR/LF");  
flush(s);

N = 10000;           % samples quantity
y = zeros(N,1);

for k = 1:N
    line = readline(s);
    y(k) = str2double(line);
end

plot(y);

fclose(s);     % closing port
delete(s);     % delete from memory
clear s;       