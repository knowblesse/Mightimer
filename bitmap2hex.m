img = not(im2bw(imread('C:\Users\knowb\Desktop\Timer.bmp')));

outputHex = uint8(zeros(128*8,1));
for row = 1 : 8
    for col = 1 : 128
        outputHex(128*(row-1) + col) = array2hex(img(8*(row-1) + 1 : 8*row, col), false);
        fprintf('0x%02X,', array2hex(img(8*(row-1) + 1 : 8*row, col), false));
    end
    fprintf('\n');
end
fprintf('\b\b');
fprintf('\n');
fprintf('\n');


