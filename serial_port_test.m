% * @file    : serial_port_test.m %
% * @author  : a.katowski, https://github.com/kazuhiroo
% * @date    : Jan 24, 2026
% * @brief   : Tracking the response and data received from UART via MATLAB
%              script

s = serialport("COM3", 115200);  % set UART and baudrate
configureTerminator(s,"CR/LF");  
flush(s);

N = 10000;           % samples quantity
data = zeros(N,3);

for k = 1:N
    line = readline(s);
    rx = str2double(line);
    data(k,:) = rx;
end

subplot(3,1,1); plot(data(:,1), 'b'); title('y_ref');
subplot(3,1,2); plot(data(:,2), 'r'); title('y');
subplot(3,1,3); plot(data(:,3), 'y'); title('u');

fclose(s);     % closing port
delete(s);     % delete from memory
clear s;       