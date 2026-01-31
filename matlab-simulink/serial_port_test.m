% * @file    : serial_port_test.m %
% * @author  : a.katowski, https://github.com/kazuhiroo
% * @date    : Jan 24, 2026
% * @brief   : Tracking the response and data received from UART via MATLAB
%              script

s = serialport("COM3", 115200);
configureTerminator(s,"CR/LF");  
flush(s);

N = 1000;           
data = zeros(N,3);

for k = 1:N
    line = readline(s);                  
    vals = str2double(split(strtrim(line)));  
    if numel(vals) == 3
        data(k,:) = vals;             
    else
        warning('err: %s', line);
    end
end

subplot(3,1,1); plot(data(:,1), 'b'); title('y\_ref');
subplot(3,1,2); plot(data(:,2), 'r'); title('y');
subplot(3,1,3); plot(data(:,3), 'y'); title('u');

clear s;
     