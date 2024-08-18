def convert_rgb_to_c_array(raw_file, c_array_name, width, height):
    with open(raw_file, 'rb') as f:
        raw_data = f.read()

    c_array = f'const unsigned char {c_array_name}[{len(raw_data)}] = {{\n'

    for i in range(0, len(raw_data), 3):
        r = raw_data[i]
        g = raw_data[i + 1]
        b = raw_data[i + 2]
        # Swap R and B to convert RGB to BGR
        c_array += f'0X{b:02X}, 0X{g:02X}, 0X{r:02X}, '

        # Insert a newline every 12 bytes for readability
        if (i + 3) % (12 * 3) == 0:
            c_array += '\n'

    c_array = c_array.rstrip(', \n')  # Remove the last comma and newline
    c_array += '\n};'

    return c_array

# Convert and save to a C header file
c_array = convert_rgb_to_c_array('output.raw', 'gImage_logo', 160, 80)
with open('image_data.h', 'w') as f:
    f.write(c_array)

print("C array has been written to 'image_data.h'")
