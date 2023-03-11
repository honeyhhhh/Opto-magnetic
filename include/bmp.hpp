#ifndef _BMP_H 
#define _BMP_H 
#include <iostream>
#include <fstream>
#include <vector>
#include <Windows.h>
#include <memory>



// 


// 定义一个结构体，表示bmp文件的文件头
#pragma pack(2)
typedef struct
{
    uint16_t file_type;         // 文件类型，必须为BM  1-2字节
    uint32_t file_size;              // 文件大小，以字节为单位 3-6字节
    uint16_t reserved1;              // 保留，必须为0    7-8字节
    uint16_t reserved2;              // 保留，必须为0    9-10字节
    uint32_t offset_data;            // 文件头到数据的偏移量，以字节为单位   11-14字节
} bmp_file_header;

// 定义一个结构体，表示bmp文件的信息头
typedef struct
{
    uint32_t size;                   // 信息头的大小，以字节为单位   15-18
    int32_t width;                   // 位图宽度，以像素为单位     19-22
    int32_t height;                  // 位图高度，以像素为单位     23-26
    uint16_t planes;                 // 颜色平面数，必须为1          27-28
    uint16_t bit_count;              // 每个像素的位数（1、4、8、16、24或32）    29-30
    uint32_t compression;            // 压缩类型（0=无压缩）                     31-34
    uint32_t size_image;             // 位图大小，以字节为单位（可设为0）        35-38
    int32_t x_pixels_per_meter;   // 水平分辨率（每米像素数）2835             39-42   
    int32_t y_pixels_per_meter;   // 垂直分辨率（每米像素数）2835             43-46
    uint32_t colors_used;            // 位图中实际使用的颜色表的颜色数（可设为0）                47-50
    uint32_t colors_important;       // 位图显示过程中重要颜色数（可设为0）              51-54
    
} bmp_info_header;

// 定义一个结构体，表示一个RGB颜色值（24位）
struct rgb_color {
    uint8_t blue; // 蓝色分量（最低有效字节）
    uint8_t green; // 绿色分量
    uint8_t red; // 红色分量（最高有效字节）
};



// 定义一个函数模板，将任意类型的二维向量转换成rgb_color类型的二维向量
template<typename T> 
std::vector<std::vector<rgb_color>> convert_to_rgb(const std::vector<std::vector<T>>& matrix) 
{
    std::vector<std::vector<rgb_color>> result;
    for (const auto& row : matrix) 
    {
        std::vector<rgb_color> rgb_row;
        for (const auto& element : row) 
        {
            rgb_color color;
            color.red = static_cast<uint8_t>(element); // 将元素值转换成红色分量值
            color.green = static_cast<uint8_t>(element); // 将元素值转换成绿色分量值
            color.blue = static_cast<uint8_t>(element); // 将元素值转换成蓝色分量值
            rgb_row.push_back(color);
        }
        result.push_back(rgb_row);
    }
    return result;
}



void SaveVectorAsBmp(std::unique_ptr<uint8_t[]> p, int width, int heigth, int channels,  const std::string& filename)


#endif	// _BMP_H