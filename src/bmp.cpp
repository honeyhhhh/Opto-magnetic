#include "bmp.hpp"
#include <fstream>
#include <chrono>
#include <iostream>


void SaveVectorAsBmp(std::unique_ptr<uint8_t[]> p, int width, int heigth, int channels,  const std::string& filename)
{

    auto t1 = std::chrono::steady_clock::now();
    auto imgbuffer = p.get();

    std::ofstream out(filename, std::ios::binary);

    bmp_file_header file_head;
    bmp_info_header info_head;

    file_head.file_type[0] = 'B';
    file_head.file_type[1] = 'M';
    file_head.reserved1 = 0;
    file_head.reserved2 = 0;
    file_head.file_size = 14 + 40 + 1024 + width * heigth * channels;
    file_head.offset_data = 54;

    std::cout << "write bmp head 1 :" << sizeof(file_head) << std::endl;
    out.write((char *)(&file_head), sizeof(file_head));


    info_head.size = 40;
    info_head.width = width;  // cols列1280
    info_head.height = heigth; // rows行1024
    info_head.bit_count = 8 * channels;
    info_head.planes = 1;
    info_head.compression = 0;
    info_head.size_image = width * heigth * channels;
    info_head.x_pixels_per_meter = 2835;
    info_head.y_pixels_per_meter = 2835;
    info_head.colors_used = 0;
    info_head.colors_important = 0;

    std::cout << "write bmp head 2 :" << sizeof(info_head) << std::endl;
    out.write((char *)(&info_head), sizeof(info_head));

    if (channels == 1)
    {
        for (int i = heigth - 1; i >= 0; i--)
        {
            for (int j = 0; j < width; j++)
            {
                out.write(reinterpret_cast<char *>(&imgbuffer[i * width + j]), sizeof(uint8_t));
            }
        }
    }

    out.close();
    auto t2 = std::chrono::steady_clock::now();
    double dr_ms = std::chrono::duration<double,std::milli>(t2-t1).count();
    std::cout << "use time: " << dr_ms << std::endl;


}