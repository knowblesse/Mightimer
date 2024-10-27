img = not(im2bw(imread('C:\VCF\Mightimer\Timer.bmp')));

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


%% Partial Hex Data

for i = 0 : 9
    img = not(im2bw(imread(strcat('C:\VCF\Mightimer\assets\image\small_digit\', num2str(i), '.bmp'))));
    imagesize = [size(img,1)/8, size(img,2)]; % 8 rows = 1 hex data

    fprintf('\t// %d\n', i);
    fprintf('\t{\n');
    for row = 1 : imagesize(1)
        fprintf('\t');
        for col = 1 : imagesize(2)
            fprintf('0x%02X,', array2hex(img(8*(row-1) + 1 : 8*row, col), false));
        end
        fprintf('\n');
    end
    fprintf('\b\b\b');
    fprintf('\n');
    fprintf('\t}');
    fprintf('\n');
end