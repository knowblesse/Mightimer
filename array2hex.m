function output = array2hex(array, isFlip)
%% Change binary array to hexadecimal
if size(array, 2) == 1 && size(array, 1) ~= 1
    array = array';
elseif size(array, 1) ~= 1 && size(array, 2) ~= 1
    error('array must be a vector');
end
    
        
if isFlip
    output = uint8(sum([128, 64, 32, 16, 8, 4, 2, 1] .* array));
else
    output = uint8(sum([1, 2, 4, 8, 16, 32, 64, 128] .* array));
end